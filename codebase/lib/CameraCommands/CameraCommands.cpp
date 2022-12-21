#include "CameraCommands.h"
#include <sstream>

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

void CameraCommands::receiveCameraResponse(byte* reply) {
    int byte_num { 0 };
    while (Serial.available() > 0) {
        reply[byte_num] = Serial.read();
        byte_num++;

        if (byte_num == NUM_BYTES_IN_CMD)
            break;
    }
}

bool CameraCommands::getCameraCommand(const byte id, uint8_t& nak_reason) {
    const byte ack_reply[] = {0xAA, 0x0E, id};
    byte reply[NUM_BYTES_IN_CMD];
    receiveCameraResponse(reply);

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
    receiveCameraResponse(reply);
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

bool CameraCommands::parseInitialisationParameters(byte* init_cmd, std::string command) {
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
    parseInitialisationParameters(init_cmd, command);

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
    constexpr byte lower_byte = MAX_PKG_SIZE_BYTES & 0xFF;
    constexpr byte upper_byte = (MAX_PKG_SIZE_BYTES >> 8) & 0xFF;
    const byte package_size_cmd[] = {0xAA, 0x06, 0x08, lower_byte, upper_byte, 0x00};

    sendClientMessage("Attempting to set package size: ");
    sendClientCommand(package_size_cmd);

    Serial.write(package_size_cmd, sizeof(package_size_cmd));
    Serial.flush();

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
    
}