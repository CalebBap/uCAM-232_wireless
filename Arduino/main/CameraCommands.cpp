#include "CameraCommands.h"
#include "CameraServer.h"

#define NUM_BYTES_IN_CMD 6
#define MAX_SYNC_ATTEMPTS 60
#define MIN_SYNC_DELAY  5

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
                }else if(received_ack && memcmp(reply, sync_cmd, sizeof(sync_cmd))){
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

void CameraCommands::attemptInitialisation(){
  CameraServer::sendClientMessage("Attempting initialisation...\n");
}
