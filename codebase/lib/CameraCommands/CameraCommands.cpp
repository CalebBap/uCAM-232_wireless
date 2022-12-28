#include "CameraCommands.h"
#include <sstream>
#include <iomanip>

void CameraCommands::sendClientMessage(std::string message) {
    webSocket.broadcastTXT(message.c_str(), message.size());
}

void CameraCommands::sendClientCommand(const byte* cmd) {
    char values[CMD_CLIENT_MESSAGE_SIZE];
    snprintf(values, sizeof(values), "0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X\n\n",
             cmd[0], cmd[1], cmd[2], cmd[3], cmd[4], cmd[5]);
    webSocket.broadcastTXT(values, strlen(values));
}

void CameraCommands::unrecognisedCommand(std::string command) {
    sendClientMessage("Unrecognised command: " + command);
}

void CameraCommands::receiveCameraResponse(byte* reply, int reply_size) {
    while (Serial.available() < reply_size) yield();

    for (int i{0}; i < reply_size; i++)
        reply[i] = Serial.read();
}

bool CameraCommands::getCameraCommand(const byte id, uint8_t& nak_reason) {
    const byte ack_reply[] = {0xAA, 0x0E, id};
    byte reply[NUM_BYTES_IN_CMD];
    receiveCameraResponse(reply, sizeof(reply));

    if (memcmp(reply, ack_reply, sizeof(ack_reply)) == 0) {
        sendClientMessage("Received ACK: ");
        sendClientCommand(reply);
        return true;
    }
    else if ( (memcmp(reply, nak_reply, sizeof(nak_reply)) == 0) && (nak_reason != reply[nak_byte]) ) {
        sendClientMessage("Received NAK: ");

        nak_reason = reply[nak_byte];
        auto reason = NakReason.find(nak_reason);
        if (reason != NakReason.end())
            sendClientMessage(reason->second + "\n\n");
        else
            sendClientCommand(reply);
    }

    return false;
}

bool CameraCommands::sendCameraCommand(const byte* cmd, const byte id) {
    uint8_t nak_reason { 0 };
    for (int i{0}; i < MAX_CMD_ATTEMPTS; i++) {
        Serial.write(cmd, NUM_BYTES_IN_CMD);
        Serial.flush();
        delay(MIN_CMD_DELAY + i);

        if (getCameraCommand(id, nak_reason))
            return true;
        
        yield();
    }

    return false;
}

void CameraCommands::attemptSync() {
    const byte sync_cmd[] = {0xAA, 0x0D, 0x00, 0x00, 0x00, 0x00};
    const byte ack_cmd[] = {0xAA, 0x0E, 0x0D, 0x00, 0x00, 0x00};

    // send SYNC command
    sendClientMessage("Attempting to sync: ");
    sendClientCommand(sync_cmd);
    if (!sendCameraCommand(sync_cmd, sync_id)) {
        sendClientMessage("Failed to sync with camera\n\n");
        sendClientMessage(sync_nak);
        return;
    }

    // receive SYNC command and ACK it
    byte reply[NUM_BYTES_IN_CMD];
    receiveCameraResponse(reply, sizeof(reply));
    if (memcmp(reply, sync_cmd, sizeof(sync_cmd)) == 0) {
        sendClientMessage("Received SYNC: ");
        sendClientCommand(reply);

        sendClientMessage("Acknowledging sync: ");
        sendClientCommand(ack_cmd);
        Serial.write(ack_cmd, sizeof(ack_cmd));
        Serial.flush();
        sendClientMessage(sync_ack);
    }
    else {
        sendClientMessage("Failed to sync: ");
        sendClientCommand(reply);
        sendClientMessage(sync_nak);
    }
}

bool CameraCommands::parseInitParameters(byte* init_cmd, std::string command) {
    command.erase(0, command.find(cmd_delimiter) + cmd_delimiter.size());
    std::string_view colour_type { command.substr(0, command.find(value_delimiter)) };
    command.erase(0, command.find(value_delimiter) + value_delimiter.size());
    std::string_view resolution { command };

    if (ColourTypes.find(colour_type) != ColourTypes.end()) {
        init_cmd[3] = ColourTypes[colour_type];
    }
    else {
        sendClientMessage("Invalid colour type parameter\n\n");
        sendClientMessage(static_cast<std::string>(colour_type));
        sendClientMessage("\n\n");
        sendClientMessage(init_nak);
        return false;
    }

    if ( (colour_type == "J") && (JpegResolutions.find(resolution) != JpegResolutions.end()) ) {
        init_cmd[4] = JpegResolutions[resolution];
        init_cmd[5] = JpegResolutions[resolution];
        current_colour_type = JPEG;
    }
    else if (RawResolutions.find(resolution) != RawResolutions.end()) {
        init_cmd[4] = RawResolutions[resolution];
        current_colour_type = RAW;
    }
    else {
        sendClientMessage("Invalid resolution parameter\n\n");
        sendClientMessage(static_cast<std::string>(resolution));
        sendClientMessage("\n\n");
        sendClientMessage(init_nak);
        current_colour_type = NONE;
        return false;
    }

    return true;
}

void CameraCommands::attemptInitialisation(std::string command) {
    byte init_cmd[]= {0xAA, 0x01, 0x00, 0x01, 0x01, 0x01};
    parseInitParameters(init_cmd, command);

    sendClientMessage("Attempting initialisation: ");
    sendClientCommand(init_cmd);

    if (!sendCameraCommand(init_cmd, init_id)) {
        sendClientMessage("Failed to initialise camera\n\n");
        sendClientMessage(init_nak);
        return;
    }

    if (current_colour_type == JPEG) {
        if (setPackageSize())
            sendClientMessage(init_ack);
        else
            sendClientMessage(init_nak);
    }
    else if (current_colour_type == RAW) {
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
    const byte package_size_cmd[] = {0xAA, 0x06, 0x08, lower_byte, upper_byte, 0x00};

    sendClientMessage("Attempting to set package size: ");
    sendClientCommand(package_size_cmd);

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
    command.erase(0, command.find(cmd_delimiter) + cmd_delimiter.size());

    int num_frames { -1 };
    std::istringstream ss(command);
    ss >> num_frames;
    if (!ss || !ss.eof())
        sendClientMessage("Frames value \"" + command + "\" is invalid.\n\n");

   return num_frames;
}

void CameraCommands::attemptSnapshot(std::string command) {
    int num_frames { parseSnapshotParameters(command) };
    if (num_frames < 0) {
        sendClientMessage("Failed to get snapshot\n\n");
        sendClientMessage(snap_nak);
        return;
    }

    const byte lower_byte = num_frames & 0xFF;
    const byte upper_byte = (num_frames >> 8) & 0xFF;
    byte snapshot_cmd[]= {0xAA, 0x05, 0x00, lower_byte, upper_byte, 0x00};

    if (SnapshotType.find(current_colour_type) != SnapshotType.end()) {
        snapshot_cmd[2] = SnapshotType[current_colour_type];
    }
    else {
        sendClientMessage("Failed to get snapshot: no colour type selected.\n\n");
        return;
    }

    sendClientMessage("Attempting to take snapshot: ");
    sendClientCommand(snapshot_cmd);

    if (sendCameraCommand(snapshot_cmd, snapshot_id))
        getPicture(SNAPSHOT);
    else {
        sendClientMessage("Failed to get snapshot\n\n");
        sendClientMessage(snap_nak);
    }
}

void CameraCommands::getPicture(DataType data_type) {
    const byte picture_cmd[] = {0xAA, 0x04, data_type, 0x00, 0x00, 0x00};

    sendClientMessage("Attempting to get picture: ");
    sendClientCommand(picture_cmd);

    if (sendCameraCommand(picture_cmd, picture_id))
        getData(data_type);
    else {
        sendClientMessage("Failed to get picture\n\n");
        return;
    }
}

void CameraCommands::getData(DataType data_type) {
    byte reply[NUM_BYTES_IN_CMD];
    receiveCameraResponse(reply, sizeof(reply));
    
    const byte expected_reply[] = {0xAA, data_id, data_type};
    if ( memcmp(reply, expected_reply, sizeof(expected_reply)) == 0 ) {
        sendClientMessage("Received data command: ");
        sendClientCommand(reply);
    }
    else {
        sendClientMessage("Unexpected data command: ");
        sendClientCommand(reply);
        return;
    }

    uint32_t img_size { (uint32_t)reply[3] | (uint32_t)(reply[4] << 8) | (uint32_t)(reply[5] << 16) };

    if ( (data_type == SNAPSHOT) && (current_colour_type == JPEG) )
        getJpegData(img_size);
    else
        getRawData();
}

void CameraCommands::getJpegData(uint32_t img_size) {
    const uint32_t num_pkgs { (img_size + NUM_PKG_DATA_BYTES - 1) / NUM_PKG_DATA_BYTES };    // ceiling division
    byte ack_cmd[] = { 0xAA, 0x0E, 0x00, 0x00, 0x00, 0x00};

    sendClientMessage("Sending ACK: ");
    sendClientCommand(ack_cmd);

    for (uint16_t pkg_num {0}; pkg_num < num_pkgs; pkg_num++) {
        ack_cmd[4] = static_cast<byte>(pkg_num & 0xFF);
        ack_cmd[5] = static_cast<byte>(pkg_num >> 8);
        Serial.write(ack_cmd, sizeof(ack_cmd));
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
            data_str += hex_digits[(data & 0xF0) >> 4];
            data_str += hex_digits[(data & 0xF)];
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

void CameraCommands::getRawData() {
    sendClientMessage("getRawData()");
}