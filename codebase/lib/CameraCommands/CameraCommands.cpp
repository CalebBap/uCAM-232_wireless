#include "CameraCommands.h"
#include <map>
#include <string>
#include <sstream>
#include <stdexcept>

void CameraCommands::receiveCameraResponse(byte* reply) {
    int byte_num = 0;

    while (Serial.available() > 0) {
        reply[byte_num] = Serial.read();
        byte_num++;

        if (byte_num == NUM_BYTES_IN_CMD)
            break;
    }
}

void CameraCommands::sendClientMessage(std::string message) {
  webSocket.broadcastTXT(message.c_str(), message.size());
}

void CameraCommands::sendClientCommand(const byte cmd[]) {
  char values[CMD_CLIENT_MESSAGE_SIZE];
  snprintf(values, sizeof(values), "0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X\n\n",\
            cmd[0], cmd[1], cmd[2], cmd[3], cmd[4], cmd[5]);
  webSocket.broadcastTXT(values, strlen(values));
}

void CameraCommands::unrecognisedCommand(std::string command) {
    sendClientMessage("Unrecognised command: " + command);
}

void CameraCommands::attemptSync() {
    byte sync_cmd[] = {0xAA, 0x0D, 0x00, 0x00, 0x00, 0x00};
    byte ack_cmd[] = {0xAA, 0x0E, 0x0D, 0x00, 0x00, 0x00};
    byte ack_reply[] = {0xAA, 0x0E, 0x0D};
    byte reply[NUM_BYTES_IN_CMD];
    
    bool received_ack = false;

    sendClientMessage("Attempting to sync: ");
    sendClientCommand(sync_cmd);

    for (int i = 0; i < MAX_SYNC_ATTEMPTS; i++) {
        Serial.write(sync_cmd, sizeof(sync_cmd));
        
        delay(MIN_SYNC_DELAY + i);

        receiveCameraResponse(reply);

        if (!received_ack && memcmp(reply, ack_reply, sizeof(ack_reply))) {
            sendClientMessage("Received ACK: ");
            sendClientCommand(reply);
            received_ack = true;
        }
        else if (received_ack && memcmp(reply, sync_cmd, sizeof(sync_cmd))) {
            sendClientMessage("Received SYNC: ");
            sendClientCommand(reply);
            Serial.write(ack_cmd, sizeof(ack_cmd));
            sendClientMessage("#synced");
            return;
        }

        yield();
    }
    
    sendClientMessage("Failed to sync\n\n");
    sendClientMessage("#sync_failed");
}

void CameraCommands::parseInitialisationParameters(std::string command) {
    byte init_cmd[]= {0xAA, 0x01, 0x00, 0x01, 0x01, 0x01};
    bool set_package_size = false;

    static std::map<std::string_view, byte> ColourTypes = {  
                                                        {"2GS", 0x01}, {"4GS", 0x02}, {"8GS", 0x03},
                                                        {"8C", 0x04}, {"12C", 0x05}, {"16C", 0x06},
                                                        {"J", 0x07}
                                                    };

    static std::map<std::string_view, byte> RawResolutions = { 
                                                            {"80x60", 0x01}, {"160x120", 0x03}, {"320x240", 0x05},
                                                            {"640x480", 0x07}, {"128x128", 0x09}, {"128x96", 0x0B}
                                                        };

    static std::map<std::string_view, byte> JpegResolutions = {  
                                                            {"80x64", 0x01}, {"160x128", 0x03}, {"320x240", 0x05},
                                                            {"640x480", 0x07}
                                                        };

    command.erase(0, command.find(cmd_delimiter) + cmd_delimiter.size());
    std::string_view colour_type = command.substr(0, command.find(value_delimiter));
    command.erase(0, command.find(value_delimiter) + value_delimiter.size());
    std::string_view resolution = command;

    if (ColourTypes.find(colour_type) != ColourTypes.end()) {
        init_cmd[3] = ColourTypes[colour_type];
    }
    else {
        sendClientMessage("Invalid colour type parameter\n\n");
        sendClientMessage(static_cast<std::string>(colour_type));
        sendClientMessage("\n\n");    // add to start of the below string?
        sendClientMessage("#init_failed");
        return;
    }

    if ( (colour_type == "J") && (JpegResolutions.find(resolution) != JpegResolutions.end()) ) {
        init_cmd[5] = JpegResolutions[resolution];
        set_package_size = true;
    }
    else if (RawResolutions.find(resolution) != RawResolutions.end()) {
        init_cmd[4] = RawResolutions[resolution];
    }
    else {
        sendClientMessage("Invalid resolution parameter\n\n");
        sendClientMessage(static_cast<std::string>(resolution));
        sendClientMessage("\n\n");    // add to start of the below string?
        sendClientMessage("#init_failed");
        return;
    }

    attemptInitialisation(init_cmd, set_package_size);
}

void CameraCommands::attemptInitialisation(const byte* init_cmd, bool set_package_size) {
    byte ack_reply[] = {0xAA, 0x0E, 0x01};
    byte reply[NUM_BYTES_IN_CMD];

    sendClientMessage("Attempting initialisation: ");
    sendClientCommand(init_cmd);

    Serial.write(init_cmd, sizeof(init_cmd));

    receiveCameraResponse(reply);

    if (memcmp(reply, ack_reply, sizeof(ack_reply))) {
        sendClientMessage("Received ACK: ");
        sendClientCommand(reply);

        if (set_package_size && !setPackageSize()) {
            sendClientMessage("#init_failed");
            return;
        }

        sendClientMessage("#initialised");
        return;
    }
    
    sendClientMessage("Failed to initialise\n\n");
    sendClientMessage("#init_failed");
}

bool CameraCommands::setPackageSize() {
    byte ack_reply[] = {0xAA, 0x0E, 0x06};
    byte package_size_cmd[] = {0xAA, 0x06, 0x08, 0x00, 0x02, 0x00};
    byte reply[NUM_BYTES_IN_CMD];

    sendClientMessage("Attempting to set package size: ");
    sendClientCommand(package_size_cmd);

    Serial.write(package_size_cmd, sizeof(package_size_cmd));
    
    receiveCameraResponse(reply);

    if (memcmp(reply, ack_reply, sizeof(ack_reply))) {
        sendClientMessage("Package size set to 512 bytes\n\n");
        return true;
    }
    else {
        sendClientMessage("Failed to set package size\n\n");
        return false;
    }
}

void CameraCommands::parseSnapshotParameters(std::string command) {
    command.erase(0, command.find(cmd_delimiter) + cmd_delimiter.size());
    std::string frames_value = command;

    int num_frames {};
    if (!std::istringstream(frames_value) >> num_frames) {
        sendClientMessage("Frames value \"" + frames_value + "\" is invalid.\n\n");
        return;
    }

    getSnapshot(num_frames);
}

void CameraCommands::getSnapshot(int num_frames) {
    
}