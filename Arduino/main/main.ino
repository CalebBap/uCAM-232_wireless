#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include "Credentials.h"

void mainPage();
void pageNotFound();
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
void attemptSync();
String getContentType(String filename);
bool handleFileRead(String path);
void sendClientMessage(const char* message);

ESP8266WebServer server;
WebSocketsServer webSocket = WebSocketsServer(81);

IPAddress staticIP(10, 100, 0, 200);
IPAddress gateway(10, 100, 0, 254);
IPAddress subnet(255, 255, 255, 0);

void setup(){
  Serial.begin(57600);
  delay(10);

  WiFi.config(staticIP, gateway, subnet);
  WiFi.begin(WAN_NAME, PASSWORD);

  while(WiFi.status() != WL_CONNECTED){
    delay(1000);
  }

  server.onNotFound([](){
    if(!handleFileRead(server.uri())){
      server.send(404, "text/plain", "404: Not Found");
    }
  });

  SPIFFS.begin();
  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop(){
  webSocket.loop();
  server.handleClient();
}

void mainPage(){
  String path = server.uri() + "index.html";
  String contentType = "text/html";

  if(SPIFFS.exists(path)){
    File file = SPIFFS.open(path, "r");
    size_t size = server.streamFile(file, contentType);
    file.close();
    return;
  }
}

void pageNotFound(){
  server.send(404, "text/plain", "404: Not Found");
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length){
  if( (type == WStype_TEXT) && (payload[0] == '#') ){
    attemptSync();
  }
}

void attemptSync(){
  sendClientMessage("Attempting to sync with camera...\n");

  byte sync_cmd[] = {0xAA, 0x0D, 0x00, 0x00, 0x00, 0x00};
  byte ACK[] = {0xAA, 0x0E, 0x0D, 0x00, 0x00, 0x00};
  byte reply[6];
  int byte_num = 0;
  
  for(int i=0; i<60; i++){
    Serial.write(sync_cmd, sizeof(sync_cmd));
    
    delay(5 + i);

    while(Serial.available() > 0){
      reply[byte_num] = Serial.read();

      if(byte_num == 5){
          if(memcmp(reply, ACK, 6)){
            sendClientMessage("Received ACK\n");
            return;
          }

          byte_num = -1;
      }
      byte_num++;
    }

    yield();
  }
  
  sendClientMessage("Failed to sync\n");
}

String getContentType(String filename){
  if(filename.endsWith(".html")){
    return "text/html";
  }
  else if(filename.endsWith(".css")){
    return "text/css";
  }
  else if(filename.endsWith(".js")){
    return "application/javascript";
  }
  else if(filename.endsWith(".ico")){
    return "image/x-icon";
  }
  else if(filename.endsWith(".gz")){
    return "application/x-gzip";
  }
  else{
    return "text/plain";
  }
}

bool handleFileRead(String path){
  if(path.endsWith("/")){
    path += "index.html";   // send the index file when IP address is visited
  }

  String contentType = getContentType(path);    // get the MIME type
  String pathWithGz = path + ".gz";
  
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){
    
    // use gzip version of file even if normal version exists
    if(SPIFFS.exists(pathWithGz)){
      path += ".gz";
    }
    
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, contentType);
    file.close();
    
    return true;
  }

  return false;   // return false if the file doesn't exist
}

void sendClientMessage(const char* message){
  webSocket.broadcastTXT(message, strlen(message));
}
