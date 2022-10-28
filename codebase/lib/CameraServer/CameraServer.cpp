#include "CameraServer.h"

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);

ESP8266WebServer server;
WebSocketsServer webSocket = WebSocketsServer(81);

const char * syncCmd = "#sync";
const char * initialiseCmd = "#initial";
const char * getPictureCmd = "#getPicture";
const char * snapshotCmd = "#snapshot";
const char * setPackageSizeCmd = "#setPackageSize";
const char * setBaudRateCmd = "#setBaudRate";
const char * resetCmd = "#reset";
const char * dataCmd = "#data";
const char * ackCmd = "#ack";
const char * nakCmd = "#nak";
const char * lightCmd = "#light";

void CameraServer::initialise() {
  IPAddress staticIP(10, 100, 0, 200);
  IPAddress gateway(10, 100, 0, 254);
  IPAddress subnet(255, 255, 255, 0);
  
  WiFi.config(staticIP, gateway, subnet);
  WiFi.begin(WAN_NAME, PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    yield();
  }

  server.onNotFound(sendFile);

  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  SPIFFS.begin();
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  static CameraCommands cameraCommands;

  if (type == WStype_TEXT) {
    if (memcmp((char *)payload, syncCmd, sizeof(syncCmd)) == 0) {
      cameraCommands.attemptSync();
    }
    else if (memcmp((char *)payload, initialiseCmd, sizeof(initialiseCmd)) == 0) {
      cameraCommands.parseInitialisationParameters((char *)payload);
    }
    else if (memcmp((char *)payload, snapshotCmd, sizeof(snapshotCmd)) == 0) {
      cameraCommands.parseSnapshotParameters((char *)payload);
    }
  }
}

void CameraServer::handleWifi() {
    webSocket.loop();
    server.handleClient();
}

void CameraServer::sendClientMessage(const char* message) {
  webSocket.broadcastTXT(message, strlen(message));
}

void CameraServer::sendClientCommand(const byte cmd[]) {
  char values[CMD_CLIENT_MESSAGE_SIZE];
  sprintf(values, "0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X\n\n", cmd[0], cmd[1], cmd[2], cmd[3], cmd[4], cmd[5]);
  webSocket.broadcastTXT(values, strlen(values));
}

void CameraServer::sendFile() {
  String path = FileOperations::getFilePath(server.uri());
  String mimeType = FileOperations::getMimeType(path);

  if (path == "") {
      server.send(404, "text/plain", "404: Not Found");
      return;
  }
  
  File file = SPIFFS.open(path, "r");
  server.streamFile(file, mimeType);
  file.close();
}
