#include "FileOperations.h"

String FileOperations::getMimeType(String filename){
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

String FileOperations::getFilePath(String uri){
  if(uri.endsWith("/"))
    uri += "index.html";   // send the index file when IP address is visited
  
  String uriWithGz = uri + ".gz";
  
  if(SPIFFS.exists(uriWithGz) || SPIFFS.exists(uri)){
    // use gzip version of file even if normal version exists
    if(SPIFFS.exists(uriWithGz))
      uri += ".gz";
    return uri;
  }
  return "";
}
