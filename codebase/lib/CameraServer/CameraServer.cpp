#include "CameraServer.h"
#include "..\..\src\Credentials.h"
#include "..\FileOperations\FileOperations.h"
#include <string>

void CameraServer::initialise() {
  IPAddress staticIP(10, 100, 0, 200);
  IPAddress gateway(10, 100, 0, 254);
  IPAddress subnet(255, 255, 255, 0);
  
  WiFi.config(staticIP, gateway, subnet);
  WiFi.begin(WAN_NAME, PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    yield();
  }

  server.onNotFound([&]() {
    sendFile();
  });

  server.begin();
  webSocket.begin();
  webSocket.onEvent([&](uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
    webSocketEvent(type, payload);
  });

  SPIFFS.begin();
}

void CameraServer::webSocketEvent(WStype_t type, uint8_t* payload) {
  if (type == WStype_TEXT) {
    std::string payload_str((char*)payload);

    if (payload_str.substr(0, syncCmd.size()) == syncCmd)
      cameraCommands.attemptSync();

    else if (payload_str.substr(0, initialiseCmd.size()) == initialiseCmd)
      cameraCommands.parseInitialisationParameters(payload_str);

    else if (payload_str.substr(0, snapshotCmd.size()) == snapshotCmd)
      cameraCommands.parseSnapshotParameters(payload_str);

    else
      cameraCommands.unrecognisedCommand(payload_str);
  }
}

void CameraServer::handleWifi() {
    webSocket.loop();
    server.handleClient();
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
