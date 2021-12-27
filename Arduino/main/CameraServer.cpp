#include "CameraServer.h"
#include "CameraCommands.h"

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);

ESP8266WebServer server;
WebSocketsServer webSocket = WebSocketsServer(81);

IPAddress staticIP(10, 100, 0, 200);
IPAddress gateway(10, 100, 0, 254);
IPAddress subnet(255, 255, 255, 0);

void CameraServer::initialise(){
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

    server.begin();
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);

    SPIFFS.begin();
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length){
  if( (type == WStype_TEXT) && (payload[0] == '#') ){
    CameraCommands::attemptSync();
  }
}

void CameraServer::sendClientMessage(const char* message){
  webSocket.broadcastTXT(message, strlen(message));
}

void CameraServer::handleWifi(){
    webSocket.loop();
    server.handleClient();
}

String CameraServer::getContentType(String filename){
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

bool CameraServer::handleFileRead(String path){
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
