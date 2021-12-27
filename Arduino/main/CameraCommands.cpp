#include "CameraCommands.h"
#include "CameraServer.h"

void CameraCommands::attemptSync(){
    byte sync_cmd[] = {0xAA, 0x0D, 0x00, 0x00, 0x00, 0x00};
    byte ACK[] = {0xAA, 0x0E, 0x0D, 0x00, 0x00, 0x00};
    byte reply[6];
    int byte_num = 0;

    CameraServer::sendClientMessage("Attempting to sync with camera...\n");

    for(int i=0; i<60; i++){
        Serial.write(sync_cmd, sizeof(sync_cmd));
        
        delay(5 + i);

        while(Serial.available() > 0){
            reply[byte_num] = Serial.read();

            if(byte_num == 5){
                if(memcmp(reply, ACK, 6)){
                    CameraServer::sendClientMessage("Received ACK\n");
                    return;
                }

                byte_num = -1;
            }
            byte_num++;
        }

        yield();
    }
    
    CameraServer::sendClientMessage("Failed to sync\n");
}
