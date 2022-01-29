#include "CameraCommands.h"

void CameraCommands::attemptSync(){
    byte sync_cmd[] = {0xAA, 0x0D, 0x00, 0x00, 0x00, 0x00};
    byte ack_cmd[] = {0xAA, 0x0E, 0x0D, 0x00, 0x00, 0x00};
    byte ack_reply[] = {0xAA, 0x0E, 0x0D};
    
    byte reply[NUM_BYTES_IN_CMD];
    int byte_num = 0;
    bool received_ack = false;

    CameraServer::sendClientMessage("Attempting to sync with camera...\n");

    for(int i = 0; i < MAX_SYNC_ATTEMPTS; i++){
        Serial.write(sync_cmd, sizeof(sync_cmd));
        
        delay(MIN_SYNC_DELAY + i);

        while(Serial.available() > 0){
            reply[byte_num] = Serial.read();
            byte_num++;

            if(byte_num == NUM_BYTES_IN_CMD){
                if(!received_ack && memcmp(reply, ack_reply, sizeof(ack_reply))){
                    CameraServer::sendClientMessage("Received ACK\n");
                    received_ack = true;
                }
                else if(received_ack && memcmp(reply, sync_cmd, sizeof(sync_cmd))){
                    CameraServer::sendClientMessage("Received SYNC\n");
                    Serial.write(ack_cmd, sizeof(ack_cmd));
                    CameraServer::sendClientMessage("#synced");
                    return;
                }

                byte_num = 0;
            }
        }

        yield();
    }
    
    CameraServer::sendClientMessage("Failed to sync\n");
    CameraServer::sendClientMessage("#sync_failed");
}

void CameraCommands::attemptInitialisation(const char* command){
  byte init_cmd[] = {0xAA, 0x01, 0x00, 0x01, 0x01, 0x01};
  
  CameraServer::sendClientMessage("Attempting initialisation...\n");

  const char *start_parameter_1 = strchr(command, ':') + 1;
  const char *start_parameter_2 = strchr(command, ',') + 1;

  size_t parameter_1_length = start_parameter_2 - start_parameter_1;
  size_t parameter_2_length = (command + strlen(command)) - start_parameter_2;
  
  char parameter_1[parameter_1_length];
  char parameter_2[parameter_2_length];
  
  strncpy(parameter_1, start_parameter_1, parameter_1_length);
  parameter_1[sizeof(parameter_1) - 1] = '\0';
  strncpy(parameter_2, start_parameter_2, parameter_2_length);
}
