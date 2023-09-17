#include "CameraServer.h"
#include "Credentials.h"
#include "FileOperations.h"
#include <string>

namespace {
  constexpr std::string_view syncCmd { "#sync" };
  constexpr std::string_view initialiseCmd { "#init" };
  constexpr std::string_view getPictureCmd { "#getPicture" };
  constexpr std::string_view snapshotCmd { "#snapshot" };
  constexpr std::string_view setPackageSizeCmd { "#setPackageSize" };
  constexpr std::string_view setBaudRateCmd { "#setBaudRate" };
  constexpr std::string_view resetCmd { "#reset" };
  constexpr std::string_view dataCmd { "#data" };
  constexpr std::string_view ackCmd { "#ack" };
  constexpr std::string_view nakCmd { "#nak" };
  constexpr std::string_view lightCmd { "#light" };
} // namespace

CameraServer::CameraServer(uint16_t port) :
  mWebSocket(port),
  mCameraCommands(mWebSocket) {}

void CameraServer::initialise() {
  const IPAddress staticIP(10, 100, 0, 200);
  const IPAddress gateway(10, 100, 0, 254);
  const IPAddress subnet(255, 255, 255, 0);

  WiFi.config(staticIP, gateway, subnet);
  WiFi.begin(WAN_NAME, PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    yield();
  }

  mServer.onNotFound([this]() {
    sendFile();
  });

  mServer.begin();
  mWebSocket.begin();
  mWebSocket.onEvent([&](uint8_t, WStype_t type, uint8_t* payload, size_t) {
    webSocketEvent(type, payload);
  });

  SPIFFS.begin();
}

void CameraServer::webSocketEvent(WStype_t type, uint8_t* payload) {
  if (type == WStype_TEXT) {
    std::string payloadString { (char*)payload };
    issueCommand(payloadString);
  }
}

void CameraServer::issueCommand(const std::string& payload) {
  if (payload.substr(0, syncCmd.size()) == syncCmd) {
    mCameraCommands.attemptSync();
  }
  else if (payload.substr(0, initialiseCmd.size()) == initialiseCmd) {
    mCameraCommands.attemptInitialisation(payload);
  }
  else if (payload.substr(0, snapshotCmd.size()) == snapshotCmd) {
    mCameraCommands.attemptSnapshot(payload);
  }
  else {
    mCameraCommands.unrecognisedCommand(payload);
  }
}

void CameraServer::handleWifi() {
    mWebSocket.loop();
    mServer.handleClient();
}

void CameraServer::sendFile() {
  String path = FileOperations::getFilePath(mServer.uri());
  String mimeType = FileOperations::getMimeType(path);

  if (path == "") {
      mServer.send(404, "text/plain", "404: Not Found");
      return;
  }
  
  File file = SPIFFS.open(path, "r");
  mServer.streamFile(file, mimeType);
  file.close();
}
