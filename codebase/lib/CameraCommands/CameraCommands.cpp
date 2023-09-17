#include "CameraCommands.h"
#include <sstream>
#include <iomanip>

namespace {
    constexpr int PKG_SIZE_BYTES { 512 };
    constexpr int NUM_BYTES_IN_CMD { 6 };
    constexpr std::string_view CMD_DELIMITER = ":";
    constexpr std::string_view VALUE_DELIMITER = ",";
    constexpr std::string_view JPEG_TYPE = "J";
    constexpr std::string_view HEX_DIGITS = "0123456789abcdef";

    enum DataTypeValues {
        SNAPSHOT = 0x01, RAW_PREVIEW = 0x02, JPEG_PREVIEW = 0x05
    };

    const std::map<std::string_view, byte> ColourTypeValues {
        {"2GS", 0x01}, {"4GS", 0x02}, {"8GS", 0x03},
        {"8C", 0x04}, {"12C", 0x05}, {"16C", 0x06},
        {JPEG_TYPE, 0x07}
    };

    const std::map<std::string_view, byte> RawResolutionValues {
        {"80x60", 0x01}, {"128x96", 0x0B}, {"128x128", 0x09},
        {"160x120", 0x03}, {"320x240", 0x05}, {"640x480", 0x07},
    };

    const std::map<std::string_view, byte> JpegResolutionValues {
        {"80x64", 0x01}, {"160x128", 0x03}, {"320x240", 0x05},
        {"640x480", 0x07}
    };

    const std::map<uint8_t, std::string> NakReason {
        { 0x01, "Picture Type Error" }, { 0x02, "Picture Up Scale" },
        { 0x03, "Picture Scale Error" }, { 0x04, "Unexpected Reply" },
        { 0x05, "Send Picture Timeout" }, { 0x06, "Unexpected Command" },
        { 0x07, "SRAM JPEG Type Error" }, { 0x08, "SRAM JPEG Size Error" },
        { 0x09, "Picture Format Error" }, { 0x0A, "Picture Size Error" },
        { 0x0B, "Parameter Error" }, { 0x0C, "Send Register Timeout" },
        { 0x0D, "Command ID Error" }, { 0x0F, "Picture Not Ready" },
        { 0x10, "Transfer Package Number Error" }, { 0x11, "Set Transfer Package Size Wrong" },
        { 0xF0, "Command Header Error" }, { 0xF1, "Command Length Error" },
        { 0xF5, "Send Picture Error" }, { 0xFF, "Send Command Error" }
    };
} // namespace

CameraCommands::CameraCommands(WebSocketsServer& wbs) :
    mWebSocket(wbs) {}

void CameraCommands::sendClientMessage(const std::string& message) {
    mWebSocket.broadcastTXT(message.c_str(), message.size());
}

void CameraCommands::sendClientCommand(const std::vector<byte>& cmd) {
    std::ostringstream oss;
    oss << std::hex;

    auto addValue = [&oss](byte value) {
        oss << "0x" << std::setfill('0') << std::setw(2) << static_cast<int>(value) << ", ";
    };

    for (byte value : cmd) {
        addValue(value);
    }

    std::string cmd_str(oss.str());
    cmd_str.replace(cmd_str.end() - 2, cmd_str.end(), "\n\n");
    sendClientMessage(cmd_str);
}

void CameraCommands::unrecognisedCommand(const std::string& command) {
    sendClientMessage("Unrecognised command: " + command);
}

void CameraCommands::receiveCameraResponse(std::vector<byte>& reply, int size) {
    while (Serial.available() < size) yield();

    reply.resize(size);
    for (int i{0}; i < size; i++) {
        reply[i] = Serial.read() & 0xFF;
    }
}

bool CameraCommands::getCameraCommand(const byte id, uint8_t& nak_reason) {
    constexpr byte nak_byte { 0x04 };
    const std::array<byte, 3> ack_reply {0xAA, 0x0E, id};
    constexpr std::array<byte, 3> nak_reply { 0xAA, 0x0F, 0x00 };

    std::vector<byte> reply(NUM_BYTES_IN_CMD);
    receiveCameraResponse(reply, NUM_BYTES_IN_CMD);

    if (std::equal(ack_reply.begin(), ack_reply.end(), reply.begin())) {
        sendClientMessage("Received ACK: ");
        sendClientCommand(reply);
        return true;
    }
    else if (std::equal(nak_reply.begin(), nak_reply.end(), reply.begin())) {
        sendClientMessage("Received NAK: ");

        nak_reason = reply.at(nak_byte);
        auto reason = NakReason.find(nak_reason);
        if (reason != NakReason.end())
            sendClientMessage(reason->second + "\n\n");
        else
            sendClientCommand(reply);
    }

    return false;
}

bool CameraCommands::sendCameraCommand(const std::vector<byte>& cmd, const byte id) {
    uint8_t nak_reason { 0 };
    constexpr int minCmdDelay { 5 };
    constexpr int maxCmdAttempts { 60 };

    if (cmd.size() != NUM_BYTES_IN_CMD) {
        std::string bytes { std::to_string(cmd.size()) };
        sendClientMessage("Unexpected camera command size: " + bytes + " bytes");
        return false;
    }

    for (int i{0}; i < maxCmdAttempts; i++) {
        Serial.write(cmd.data(), cmd.size());
        Serial.flush();
        delay(minCmdDelay + i);

        if (getCameraCommand(id, nak_reason))
            return true;
        
        yield();
    }

    return false;
}

void CameraCommands::attemptSync() {
    const std::string sync_ack { "#sync_ack" };
    const std::vector<byte> sync_cmd {0xAA, 0x0D, 0x00, 0x00, 0x00, 0x00};
    const std::vector<byte> ack_cmd {0xAA, 0x0E, 0x0D, 0x00, 0x00, 0x00};

    // send SYNC command
    sendClientMessage("Attempting to sync: ");
    sendClientCommand(sync_cmd);

    constexpr byte sync_id { 0x0D };
    const std::string sync_nak = "#sync_nak";
    if (!sendCameraCommand(sync_cmd, sync_id)) {
        sendClientMessage("Failed to sync with camera\n\n");
        sendClientMessage(sync_nak);
        return;
    }

    // receive SYNC command and ACK it
    std::vector<byte> reply(NUM_BYTES_IN_CMD);
    receiveCameraResponse(reply, NUM_BYTES_IN_CMD);

    if (std::equal(sync_cmd.begin(), sync_cmd.end(), reply.begin())) {
        sendClientMessage("Received SYNC: ");
        sendClientCommand(reply);

        sendClientMessage("Acknowledging sync: ");
        sendClientCommand(ack_cmd);
        Serial.write(ack_cmd.data(), ack_cmd.size());
        Serial.flush();
        sendClientMessage(sync_ack);
    }
    else {
        sendClientMessage("Failed to sync: ");
        sendClientCommand(reply);
        sendClientMessage(sync_nak);
    }
}

bool CameraCommands::parseInitParameters(std::vector<byte>& init_cmd, std::string command) {
    // parse colour type and resolution from command
    command.erase(0, command.find(CMD_DELIMITER) + CMD_DELIMITER.size());
    current_colour_type = command.substr(0, command.find(VALUE_DELIMITER));
    command.erase(0, command.find(VALUE_DELIMITER) + VALUE_DELIMITER.size());
    current_resolution = command;

    // set current colour type and resolution members
    bool validColour = ColourTypeValues.find(current_colour_type) != ColourTypeValues.end();
    if (!validColour) {
        sendClientMessage("Invalid colour type in init command: " + current_colour_type + "\n\n");
        current_colour_type.clear();
        return false;
    }

    bool validResolution = 
        (current_colour_type == JPEG_TYPE && JpegResolutionValues.find(current_resolution) != JpegResolutionValues.end()) ||
        (RawResolutionValues.find(current_resolution) != RawResolutionValues.end());
    if (!validResolution) {
        sendClientMessage("Invalid resolution in init command: " + current_colour_type + "\n\n");
        current_resolution.clear();
        return false;
    }

    // set colour type byte in command
    if (ColourTypeValues.find(current_colour_type) != ColourTypeValues.end()) {
        init_cmd[3] = ColourTypeValues.at(current_colour_type);
    }
    else {
        sendClientMessage("Invalid colour type parameter: " + current_colour_type + "\n\n");
        current_colour_type.clear();
        current_resolution.clear();
        return false;
    }

    // set resolution byte in command
    if ( (current_colour_type == JPEG_TYPE) && (JpegResolutionValues.find(current_resolution) != JpegResolutionValues.end()) ) {
        init_cmd[5] = JpegResolutionValues.at(current_resolution);
        return true;
    }
    else if ( (!current_colour_type.empty()) && (RawResolutionValues.find(current_resolution) != RawResolutionValues.end()) ) {
        init_cmd[4] = RawResolutionValues.at(current_resolution);
        return true;
    }
    sendClientMessage("Invalid resolution parameter: " + current_resolution + "\n\n");
    current_colour_type.clear();
    current_resolution.clear();
    return false;
}

void CameraCommands::attemptInitialisation(const std::string& command) {
    const std::string init_ack = "#init_ack";
    const std::string init_nak = "#init_nak";
    std::vector<byte> init_cmd {0xAA, 0x01, 0x00, 0x01, 0x01, 0x01};

    if (!parseInitParameters(init_cmd, command)) {
        sendClientMessage(init_nak);
        sendClientMessage("Failed to parse initialisation parameters\n\n");
        return;
    }

    sendClientMessage("Attempting initialisation: ");
    sendClientCommand(init_cmd);

    constexpr byte init_id { 0x01 };
    if (!sendCameraCommand(init_cmd, init_id)) {
        sendClientMessage("Failed to initialise camera\n\n");
        sendClientMessage(init_nak);
        return;
    }

    if (current_colour_type == JPEG_TYPE) {
        if (setPackageSize())
            sendClientMessage(init_ack);
        else
            sendClientMessage(init_nak);
    }
    else if (!current_colour_type.empty()) {
        sendClientMessage(init_ack);
    }
    else {
        sendClientMessage("Invalid colour type after initialisation\n\n");
        sendClientMessage(init_nak);
    }
}

bool CameraCommands::setPackageSize() {
    constexpr byte lower_byte = PKG_SIZE_BYTES & 0xFF;
    constexpr byte upper_byte = (PKG_SIZE_BYTES >> 8) & 0xFF;
    const std::vector<byte> package_size_cmd {0xAA, 0x06, 0x08, lower_byte, upper_byte, 0x00};

    sendClientMessage("Attempting to set package size: ");
    sendClientCommand(package_size_cmd);

    constexpr byte pkg_size_id { 0x06 };
    if (sendCameraCommand(package_size_cmd, pkg_size_id)) {
        sendClientMessage("Package size set to 512 bytes\n\n");
        return true;
    }
    else {
        sendClientMessage("Failed to set package size\n\n");
        return false;
    }
}

int CameraCommands::parseSnapshotParameters(std::string command) {
    command.erase(0, command.find(CMD_DELIMITER) + CMD_DELIMITER.size());

    int num_frames { -1 };
    std::istringstream ss(command);
    ss >> num_frames;
    if (!ss || !ss.eof())
        sendClientMessage("Frames value \"" + command + "\" is invalid.\n\n");

   return num_frames;
}

void CameraCommands::attemptSnapshot(const std::string& command) {
    const std::string snap_nak = "#snap_nak";

    int num_frames { parseSnapshotParameters(command) };
    if (num_frames < 0) {
        sendClientMessage("Failed to get snapshot\n\n");
        sendClientMessage(snap_nak);
        return;
    }

    const byte lower_byte = num_frames & 0xFF;
    const byte upper_byte = (num_frames >> 8) & 0xFF;
    std::vector<byte> snapshot_cmd {0xAA, 0x05, 0x00, lower_byte, upper_byte, 0x00};

    constexpr byte jpeg_snapshot { 0x00 };
    constexpr byte raw_snapshot { 0x01 };
    if (current_colour_type == JPEG_TYPE) {
        snapshot_cmd[2] = jpeg_snapshot;
    }
    else if (!current_colour_type.empty()) {
        snapshot_cmd[2] = raw_snapshot;
    }
    else {
        sendClientMessage("Failed to get snapshot: no colour type selected.\n\n");
        return;
    }

    sendClientMessage("Attempting to take snapshot: ");
    sendClientCommand(snapshot_cmd);

    constexpr byte snapshot_id { 0x05 };
    if (sendCameraCommand(snapshot_cmd, snapshot_id))
        getPicture(SNAPSHOT);
    else {
        sendClientMessage("Failed to get snapshot\n\n");
        sendClientMessage(snap_nak);
    }
}

void CameraCommands::getPicture(byte data_type) {
    const std::vector<byte> picture_cmd {0xAA, 0x04, data_type, 0x00, 0x00, 0x00};

    sendClientMessage("Attempting to get picture: ");
    sendClientCommand(picture_cmd);

    constexpr byte picture_id { 0x04 };
    if (sendCameraCommand(picture_cmd, picture_id))
        getData(data_type);
    else {
        sendClientMessage("Failed to get picture\n\n");
        return;
    }
}

void CameraCommands::getData(byte data_type) {
    constexpr byte data_id { 0x0A };
    const std::array<byte, 3> expected_reply {0xAA, data_id, data_type};

    std::vector<byte> reply(NUM_BYTES_IN_CMD);
    receiveCameraResponse(reply, NUM_BYTES_IN_CMD);

    if (std::equal(expected_reply.begin(), expected_reply.end(), reply.begin())) {
        sendClientMessage("Received data command: ");
        sendClientCommand(reply);
    }
    else {
        sendClientMessage("Unexpected data command: ");
        sendClientCommand(reply);
        return;
    }

    sendClientMessage("#img," + current_colour_type + "," + current_resolution);

    uint32_t img_size { 0 };
    img_size |= static_cast<uint32_t>(reply.at(3));
    img_size |= static_cast<uint32_t>(reply.at(4) << 8);
    img_size |= static_cast<uint32_t>(reply.at(5) << 16);

    if ( (data_type == SNAPSHOT) && (current_colour_type == JPEG_TYPE) )
        getJpegData(img_size);
    else
        getRawData(img_size);
}

void CameraCommands::getJpegData(uint32_t img_size) {
    constexpr int numPkgDataBytes { PKG_SIZE_BYTES - 6 };
    const uint32_t num_pkgs { (img_size + numPkgDataBytes - 1) / numPkgDataBytes };    // ceiling division
    std::vector<byte> ack_cmd { 0xAA, 0x0E, 0x00, 0x00, 0x00, 0x00};

    sendClientMessage("Sending ACK: ");
    sendClientCommand(ack_cmd);

    for (uint16_t pkg_num {0}; pkg_num < num_pkgs; pkg_num++) {
        ack_cmd[4] = static_cast<byte>(pkg_num & 0xFF);
        ack_cmd[5] = static_cast<byte>(pkg_num >> 8);
        Serial.write(ack_cmd.data(), ack_cmd.size());
        Serial.flush();

        while (Serial.available() < 4) yield(); // wait for id and data size bytes

        uint32_t pkg_sum { 0 };

        uint8_t id_byte_1 { static_cast<uint8_t>(Serial.read() & 0xFF) };
        uint8_t id_byte_2 { static_cast<uint8_t>(Serial.read() & 0xFF) };
        pkg_sum += (id_byte_1 + id_byte_2);

        uint16_t pkg_id { static_cast<uint16_t>(id_byte_1 | (id_byte_2 << 8)) };
        if (pkg_num != pkg_id) {
            sendClientMessage("Unexpected package ID received\n\n");
            sendClientMessage("#end");
            return;
        }

        uint8_t data_size_byte_1 { static_cast<uint8_t>(Serial.read() & 0xFF) };
        uint8_t data_size_byte_2 { static_cast<uint8_t>(Serial.read() & 0xFF) };
        pkg_sum += (data_size_byte_1 + data_size_byte_2);

        uint16_t data_size { static_cast<uint16_t>(data_size_byte_1 | (data_size_byte_2 << 8)) };
        if (data_size > PKG_SIZE_BYTES) {
            sendClientMessage("Data size of " + std::to_string(data_size) + " bytes is too large\n\n");
            sendClientMessage("#end");
            return;
        }

        std::string data_str { "#img" };
        for (int i{0}; i < data_size; i++) {
            while (Serial.available() == 0) yield();
            byte data { static_cast<byte>(Serial.read() & 0xFF) };
            pkg_sum += data;
            data_str += HEX_DIGITS[(data & 0xF0) >> 4];
            data_str += HEX_DIGITS[(data & 0x0F)];
        }

        if (pkg_num == num_pkgs - 1)
            data_str += "#end";
        sendClientMessage(data_str);

        while (Serial.available() < 2) yield(); // wait for verify code bytes

        uint8_t verify_code_byte_1 { static_cast<uint8_t>(Serial.read() & 0xFF) };
        uint8_t verify_code_byte_2 { static_cast<uint8_t>(Serial.read() & 0xFF) };

        if ( (verify_code_byte_1 != (pkg_sum & 0xFF)) || (verify_code_byte_2 != 0) ) {
            sendClientMessage("Invalid verify code received");
            sendClientMessage("#end");
            return;
        }
    }
}

void CameraCommands::getRawData(uint32_t img_size) {
    std::string data_str { "#img" };
    for (uint32_t i{1}; i <= img_size; i++) {
        while (Serial.available() == 0) yield();

        byte data { static_cast<byte>(Serial.read() & 0xFF) };
        data_str += HEX_DIGITS[(data & 0xF0) >> 4];
        data_str += HEX_DIGITS[(data & 0x0F)];

        if (i == img_size) {
            data_str += "#end";
            sendClientMessage(data_str);
            return;
        }

        if (i % PKG_SIZE_BYTES == 0) {
            sendClientMessage(data_str);
            data_str = "#img";
        }
    }
}