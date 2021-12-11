#include <SoftwareSerial.h>

SoftwareSerial serialDebug(3, 2);

void attemptSync();

void setup(){
  Serial.begin(57600);
  serialDebug.begin(9600);
  attemptSync();
}

void attemptSync(){
  byte sync_cmd[] = {0xAA, 0x0D, 0x00, 0x00, 0x00, 0x00};
  byte ACK[] = {0xAA, 0x0E, 0x0D, 0x00, 0x00, 0x00};
  byte reply[6];
  int byte_num = 0;
  
  serialDebug.println("Sending SYNC command: ");
  for(int i=0; i<6; i++){
    serialDebug.print(sync_cmd[i], HEX);
  }
  serialDebug.println("");
  
  for(int i=0; i<60; i++){
    Serial.write(sync_cmd, sizeof(sync_cmd));
    
    delay(5 + i*1);

    while(Serial.available() > 0){
      reply[byte_num] = Serial.read();

      if(byte_num == 5){
          serialDebug.println("Received reply");
          for(int i=0; i<6; i++){
            serialDebug.print(reply[i], HEX);
          }
          serialDebug.println("");

          if(memcmp(reply, ACK, 6)){
            serialDebug.println("That's an ACK!");
            while(true){}
          }

          byte_num = 0;
      }else{
        byte_num++;
      }
    }  
  }
}

void loop(){}
