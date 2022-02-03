#include "CameraCommands.h"

void CameraCommands::attemptSync(){
    byte sync_cmd[] = {0xAA, 0x0D, 0x00, 0x00, 0x00, 0x00};
    byte ack_cmd[] = {0xAA, 0x0E, 0x0D, 0x00, 0x00, 0x00};
    byte ack_reply[] = {0xAA, 0x0E, 0x0D};
    
    byte reply[NUM_BYTES_IN_CMD];
    int byte_num = 0;
    bool received_ack = false;

    CameraServer::sendClientMessage("Attempting to sync with command: ");
    CameraServer::sendClientCommand(sync_cmd);

    for(int i = 0; i < MAX_SYNC_ATTEMPTS; i++){
        Serial.write(sync_cmd, sizeof(sync_cmd));
        
        delay(MIN_SYNC_DELAY + i);

        while(Serial.available() > 0){
            reply[byte_num] = Serial.read();
            byte_num++;

            if(byte_num == NUM_BYTES_IN_CMD){
                if(!received_ack && memcmp(reply, ack_reply, sizeof(ack_reply))){
                    CameraServer::sendClientMessage("Received ACK: ");
                    CameraServer::sendClientCommand(reply);
                    received_ack = true;
                }
                else if(received_ack && memcmp(reply, sync_cmd, sizeof(sync_cmd))){
                    CameraServer::sendClientMessage("Received SYNC: ");
                    CameraServer::sendClientCommand(reply);
                    Serial.write(ack_cmd, sizeof(ack_cmd));
                    CameraServer::sendClientMessage("#synced");
                    return;
                }

                byte_num = 0;
            }
        }

        yield();
    }
    
    CameraServer::sendClientMessage("Failed to sync\n\n");
    CameraServer::sendClientMessage("#sync_failed");
}

void CameraCommands::parseInitialisationParameters(const char* command){
    byte init_cmd[]= {0xAA, 0x01, 0x00, 0x01, 0x01, 0x01};

    static std::map<std::string, byte> ColourTypes = {  
                                                {"2GS", 0x01}, {"4GS", 0x02}, {"8GS", 0x03},
                                                {"8C", 0x04}, {"12C", 0x05}, {"16C", 0x06},
                                                {"J", 0x07}
                                            };

    static std::map<std::string, byte> RawResolutions = { 
                                                        {"80x60", 0x01}, {"160x120", 0x03}, {"320x240", 0x05},
                                                        {"640x480", 0x07}, {"128x128", 0x09}, {"128x96", 0x0B} 
                                                };

    static std::map<std::string, byte> JpegResolutions = {  
                                                        {"80x64", 0x01}, {"160x128", 0x03}, {"320x240", 0x05},
                                                        {"640x480", 0x07} 
                                                };

    const char *start_parameter_1 = strchr(command, ':') + 1;
    const char *start_parameter_2 = strchr(command, ',') + 1;

    size_t parameter_1_length = start_parameter_2 - start_parameter_1;
    size_t parameter_2_length = (command + strlen(command)) - start_parameter_2;
    
    char parameter_1[parameter_1_length];
    char parameter_2[parameter_2_length];
    
    strncpy(parameter_1, start_parameter_1, parameter_1_length);
    parameter_1[sizeof(parameter_1) - 1] = '\0';
    strncpy(parameter_2, start_parameter_2, parameter_2_length);

    if(ColourTypes.find(parameter_1) != ColourTypes.end()){
        init_cmd[3] = ColourTypes[parameter_1];
    }
    else{
        CameraServer::sendClientMessage("Invalid colour type parameter");
        return;
    }

    if( (strcmp(parameter_1, "J") == 0) && (JpegResolutions.find(parameter_2) != JpegResolutions.end()) ){
        init_cmd[5] = JpegResolutions[parameter_2];
    }
    else if(RawResolutions.find(parameter_2) != RawResolutions.end()){
        init_cmd[4] = RawResolutions[parameter_2];
    }
    else{
        CameraServer::sendClientMessage("Invalid resolution parameter");
        return;
    }

    attemptInitialisation(init_cmd);
}

void CameraCommands::attemptInitialisation(const byte* init_cmd){
    byte reply[NUM_BYTES_IN_CMD];
    byte ack_reply[] = {0xAA, 0x0E, 0x01};
    int byte_num = 0;

    CameraServer::sendClientMessage("Attempting initialisation with command: ");
    CameraServer::sendClientCommand(init_cmd);

    Serial.write(init_cmd, sizeof(init_cmd));

    while(Serial.available() > 0){
        reply[byte_num] = Serial.read();
        byte_num++;

        if(byte_num == NUM_BYTES_IN_CMD){
            if(memcmp(reply, ack_reply, sizeof(ack_reply))){
                CameraServer::sendClientMessage("Received ACK: ");
                CameraServer::sendClientCommand(reply);
                return;
            }

            byte_num = 0;
        }
    }
    
    CameraServer::sendClientMessage("Failed to initialise\n\n");
    CameraServer::sendClientMessage("#init_failed");
}