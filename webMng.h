#ifndef webMng_h
#define webMng_h
  
  // CHECK HOW TO ADD OTA: https://robotzero.one/arduino-ide-partitions/
  
  #include <Arduino.h>
  #include "SPIFFS.h"               // Install: https://github.com/me-no-dev/arduino-esp32fs-plugin
  #include <FS.h>                   // this needs to be first, or it all crashes and burns...
  #include <ArduinoJson.h>          // https://github.com/bblanchon/ArduinoJson

  #include <WiFi.h>
  #include <WiFiClient.h>
  #include <WebServer.h>
  #include <ESPmDNS.h>

  #define LED_WIFI 27
  
  class WebMng{
    public:
      WebMng();
      void begin(const char *fwName, const char *fwVer);
      void manage();
    
    private:
      String getContentType(String filename);           // convert the file extension to the MIME type
      bool handleFileRead(String path);                 // send the right file to the client (if it exists)
      
      const char *_fwName;
      const char *_fwVer;
      uint32_t _t_ws = 0;
  };

  void formatFileSystem();
    
  extern WebMng webMng;

#endif
