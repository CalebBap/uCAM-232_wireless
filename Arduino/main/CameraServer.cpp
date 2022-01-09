#include "CameraServer.h"
#include "CameraCommands.h"
#include "FileOperations.h"

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);

ESP8266WebServer server;
WebSocketsServer webSocket = WebSocketsServer(81);

const char * syncCmd = "#sync";

void CameraServer::initialise(){
  IPAddress staticIP(10, 100, 0, 200);
  IPAddress gateway(10, 100, 0, 254);
  IPAddress subnet(255, 255, 255, 0);
  
  WiFi.config(staticIP, gateway, subnet);
  WiFi.begin(WAN_NAME, PASSWORD);

  while(WiFi.status() != WL_CONNECTED){
    yield();
  }

  server.onNotFound(sendFile);

  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  SPIFFS.begin();
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length){
  static CameraCommands cameraCommands;
  
  if( (type == WStype_TEXT) && (memcmp((char *)payload, syncCmd, sizeof(syncCmd)) == 0) ){
    cameraCommands.attemptSync();
  }
}

void CameraServer::handleWifi(){
    webSocket.loop();
    server.handleClient();
}

void CameraServer::sendClientMessage(const char* message){
  webSocket.broadcastTXT(message, strlen(message));
}

void CameraServer::sendFile(){
  String path = FileOperations::getFilePath(server.uri());
  String mimeType = FileOperations::getMimeType(path);

  if(path == ""){
      server.send(404, "text/plain", "404: Not Found");
      return;
  }
  
  File file = SPIFFS.open(path, "r");
  size_t sent = server.streamFile(file, mimeType);
  file.close();
}
