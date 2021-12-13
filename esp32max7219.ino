// Copyright Rob Latour, 2021

// use board doit esp32 devkit v1

// For OTA Updates please ensure you are connected to the same wireless network as the device being updated

// ref: https://www.brainy-bits.com/arduino-max7219-scrolling-text-matrix/

// ota password MsgBoard

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>
#include <WebSocketsClient.h>

#include <time.h>
#include <TimeLib.h>

#include <MD_Parola.h>   // https://github.com/MajicDesigns/MD_Parola
#include <MD_MAX72xx.h>  // https://github.com/MajicDesigns/MD_MAX72xx
#include <SPI.h>

#include <EEPROM.h>

#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#define MAX_DEVICES 12  // <********************************* Number of modules connected, you may need to change this

//pins for a ESP32 Dev Kit V1
#define CLK_PIN   18 // SCK (Brown)
#define DATA_PIN  23 // MOSI (Orange)
#define CS_PIN    5  // SS (Red)

#define EEPROM_SIZE 512

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW

MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);  // SPI config

// sets scrolling direction if slider in middle at start
textEffect_t scrollEffect = PA_SCROLL_LEFT;
textPosition_t scrollAlign = PA_LEFT;  // how to aligh the text
int scrollSpeed = 1000;
int scrollPause = 0; // ms of pause after finished displaying message
int slider_scroll_speed;

#define  BUF_SIZE  1000  // Maximum of 1000 characters
char curMessage[BUF_SIZE] = { " " };  // used to hold current message

//Program ID
String ProgramID = "ESP32 message board v1.1";

//Wifi
/* WiFi network name and password */
const char * wifi_name = "xxxxxxx";              // <********************************* change this
const char * wifi_pass = "xxxxxxx";              // <********************************* change this

unsigned long LastTimeWiFiWasConnected;
unsigned long secondsSinceStartup;
unsigned long TwoMinutes = 120000;

// External button used to clear message
//const int External_Button = 33;

// Pushbullet
const String My_PushBullet_Access_Token = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";  // <********************************* change this

const String Pushbullet_Note_Title_To_React_To = "Leave a message";
const String Pushbullet_Note_Title_To_Clear    = "Clear the message";
const String Pushbullet_Note_Title_To_Restart  = "Reset the ESP32 message board alpha 1 beta 2";  // triggered by "Restart the message board"

const String PushBullet_Server = "stream.pushbullet.com";
const String PushBullet_Server_Directory = "/websocket/";
const String PushBullet_KeepAlive_ID = "rob_messages_active"; // <*** do not change this, it is a special code Pushbullet has provided for use with this program only; used to prevent accounts from expiring for 30 days
const int PushBullet_Server_Port = 443;
const int https_port = 443;

String My_PushBullet_Client_ID = "";

// to get a certificate, use a program called openssl
// for more information please see this video: https://youtu.be/pLmHELQfkx0
// to get oppenssl, download Windows executable from: https://kb.firedaemon.com/support/solutions/articles/4000121705#Download-OpenSSL
// here is the command to get the certificate for api.pushbullet.com
//      openssl s_client -showcerts -connect api.pushbullet.com:443
// use the second certificate displayed

const char* host = "api.pushbullet.com";
const char* Pushbullet_API_root_ca = "-----BEGIN CERTIFICATE-----\n" \
                                     "MIIFYjCCBEqgAwIBAgIQd70NbNs2+RrqIQ/E8FjTDTANBgkqhkiG9w0BAQsFADBX\n" \
                                     "MQswCQYDVQQGEwJCRTEZMBcGA1UEChMQR2xvYmFsU2lnbiBudi1zYTEQMA4GA1UE\n" \
                                     "CxMHUm9vdCBDQTEbMBkGA1UEAxMSR2xvYmFsU2lnbiBSb290IENBMB4XDTIwMDYx\n" \
                                     "OTAwMDA0MloXDTI4MDEyODAwMDA0MlowRzELMAkGA1UEBhMCVVMxIjAgBgNVBAoT\n" \
                                     "GUdvb2dsZSBUcnVzdCBTZXJ2aWNlcyBMTEMxFDASBgNVBAMTC0dUUyBSb290IFIx\n" \
                                     "MIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAthECix7joXebO9y/lD63\n" \
                                     "ladAPKH9gvl9MgaCcfb2jH/76Nu8ai6Xl6OMS/kr9rH5zoQdsfnFl97vufKj6bwS\n" \
                                     "iV6nqlKr+CMny6SxnGPb15l+8Ape62im9MZaRw1NEDPjTrETo8gYbEvs/AmQ351k\n" \
                                     "KSUjB6G00j0uYODP0gmHu81I8E3CwnqIiru6z1kZ1q+PsAewnjHxgsHA3y6mbWwZ\n" \
                                     "DrXYfiYaRQM9sHmklCitD38m5agI/pboPGiUU+6DOogrFZYJsuB6jC511pzrp1Zk\n" \
                                     "j5ZPaK49l8KEj8C8QMALXL32h7M1bKwYUH+E4EzNktMg6TO8UpmvMrUpsyUqtEj5\n" \
                                     "cuHKZPfmghCN6J3Cioj6OGaK/GP5Afl4/Xtcd/p2h/rs37EOeZVXtL0m79YB0esW\n" \
                                     "CruOC7XFxYpVq9Os6pFLKcwZpDIlTirxZUTQAs6qzkm06p98g7BAe+dDq6dso499\n" \
                                     "iYH6TKX/1Y7DzkvgtdizjkXPdsDtQCv9Uw+wp9U7DbGKogPeMa3Md+pvez7W35Ei\n" \
                                     "Eua++tgy/BBjFFFy3l3WFpO9KWgz7zpm7AeKJt8T11dleCfeXkkUAKIAf5qoIbap\n" \
                                     "sZWwpbkNFhHax2xIPEDgfg1azVY80ZcFuctL7TlLnMQ/0lUTbiSw1nH69MG6zO0b\n" \
                                     "9f6BQdgAmD06yK56mDcYBZUCAwEAAaOCATgwggE0MA4GA1UdDwEB/wQEAwIBhjAP\n" \
                                     "BgNVHRMBAf8EBTADAQH/MB0GA1UdDgQWBBTkrysmcRorSCeFL1JmLO/wiRNxPjAf\n" \
                                     "BgNVHSMEGDAWgBRge2YaRQ2XyolQL30EzTSo//z9SzBgBggrBgEFBQcBAQRUMFIw\n" \
                                     "JQYIKwYBBQUHMAGGGWh0dHA6Ly9vY3NwLnBraS5nb29nL2dzcjEwKQYIKwYBBQUH\n" \
                                     "MAKGHWh0dHA6Ly9wa2kuZ29vZy9nc3IxL2dzcjEuY3J0MDIGA1UdHwQrMCkwJ6Al\n" \
                                     "oCOGIWh0dHA6Ly9jcmwucGtpLmdvb2cvZ3NyMS9nc3IxLmNybDA7BgNVHSAENDAy\n" \
                                     "MAgGBmeBDAECATAIBgZngQwBAgIwDQYLKwYBBAHWeQIFAwIwDQYLKwYBBAHWeQIF\n" \
                                     "AwMwDQYJKoZIhvcNAQELBQADggEBADSkHrEoo9C0dhemMXoh6dFSPsjbdBZBiLg9\n" \
                                     "NR3t5P+T4Vxfq7vqfM/b5A3Ri1fyJm9bvhdGaJQ3b2t6yMAYN/olUazsaL+yyEn9\n" \
                                     "WprKASOshIArAoyZl+tJaox118fessmXn1hIVw41oeQa1v1vg4Fv74zPl6/AhSrw\n" \
                                     "9U5pCZEt4Wi4wStz6dTZ/CLANx8LZh1J7QJVj2fhMtfTJr9w4z30Z209fOU0iOMy\n" \
                                     "+qduBmpvvYuR7hZL6Dupszfnw0Skfths18dG9ZKb59UhvmaSGZRVbNQpsg3BZlvi\n" \
                                     "d0lIKO2d1xozclOzgjXPYovJJIultzkMu34qQb9Sz/yilrbCgj8=\n" \
                                     "-----END CERTIFICATE-----\n";


// change SendPushbulletKeepAliveRequest to false if you don't want to send PushBullet Keep Alive Requests
// Keep alive requests helps keep your Pushbullet account alive for up to 30 days from its last use

bool  SendPushbulletKeepAliveRequest = true;
const char* KeepAliveHost = "zebra.pushbullet.com";
const char* Pushbullet_KeepAlive_root_ca = "-----BEGIN CERTIFICATE-----\n" \    
                                           "MIIFFjCCAv6gAwIBAgIRAJErCErPDBinU/bWLiWnX1owDQYJKoZIhvcNAQELBQAw\n" \
                                           "TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh\n" \
                                           "cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMjAwOTA0MDAwMDAw\n" \
                                           "WhcNMjUwOTE1MTYwMDAwWjAyMQswCQYDVQQGEwJVUzEWMBQGA1UEChMNTGV0J3Mg\n" \
                                           "RW5jcnlwdDELMAkGA1UEAxMCUjMwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEK\n" \
                                           "AoIBAQC7AhUozPaglNMPEuyNVZLD+ILxmaZ6QoinXSaqtSu5xUyxr45r+XXIo9cP\n" \
                                           "R5QUVTVXjJ6oojkZ9YI8QqlObvU7wy7bjcCwXPNZOOftz2nwWgsbvsCUJCWH+jdx\n" \
                                           "sxPnHKzhm+/b5DtFUkWWqcFTzjTIUu61ru2P3mBw4qVUq7ZtDpelQDRrK9O8Zutm\n" \
                                           "NHz6a4uPVymZ+DAXXbpyb/uBxa3Shlg9F8fnCbvxK/eG3MHacV3URuPMrSXBiLxg\n" \
                                           "Z3Vms/EY96Jc5lP/Ooi2R6X/ExjqmAl3P51T+c8B5fWmcBcUr2Ok/5mzk53cU6cG\n" \
                                           "/kiFHaFpriV1uxPMUgP17VGhi9sVAgMBAAGjggEIMIIBBDAOBgNVHQ8BAf8EBAMC\n" \
                                           "AYYwHQYDVR0lBBYwFAYIKwYBBQUHAwIGCCsGAQUFBwMBMBIGA1UdEwEB/wQIMAYB\n" \
                                           "Af8CAQAwHQYDVR0OBBYEFBQusxe3WFbLrlAJQOYfr52LFMLGMB8GA1UdIwQYMBaA\n" \
                                           "FHm0WeZ7tuXkAXOACIjIGlj26ZtuMDIGCCsGAQUFBwEBBCYwJDAiBggrBgEFBQcw\n" \
                                           "AoYWaHR0cDovL3gxLmkubGVuY3Iub3JnLzAnBgNVHR8EIDAeMBygGqAYhhZodHRw\n" \
                                           "Oi8veDEuYy5sZW5jci5vcmcvMCIGA1UdIAQbMBkwCAYGZ4EMAQIBMA0GCysGAQQB\n" \
                                           "gt8TAQEBMA0GCSqGSIb3DQEBCwUAA4ICAQCFyk5HPqP3hUSFvNVneLKYY611TR6W\n" \
                                           "PTNlclQtgaDqw+34IL9fzLdwALduO/ZelN7kIJ+m74uyA+eitRY8kc607TkC53wl\n" \
                                           "ikfmZW4/RvTZ8M6UK+5UzhK8jCdLuMGYL6KvzXGRSgi3yLgjewQtCPkIVz6D2QQz\n" \
                                           "CkcheAmCJ8MqyJu5zlzyZMjAvnnAT45tRAxekrsu94sQ4egdRCnbWSDtY7kh+BIm\n" \
                                           "lJNXoB1lBMEKIq4QDUOXoRgffuDghje1WrG9ML+Hbisq/yFOGwXD9RiX8F6sw6W4\n" \
                                           "avAuvDszue5L3sz85K+EC4Y/wFVDNvZo4TYXao6Z0f+lQKc0t8DQYzk1OXVu8rp2\n" \
                                           "yJMC6alLbBfODALZvYH7n7do1AZls4I9d1P4jnkDrQoxB3UqQ9hVl3LEKQ73xF1O\n" \
                                           "yK5GhDDX8oVfGKF5u+decIsH4YaTw7mP3GFxJSqv3+0lUFJoi5Lc5da149p90Ids\n" \
                                           "hCExroL1+7mryIkXPeFM5TgO9r0rvZaBFOvV2z0gp35Z0+L4WPlbuEjN/lxPFin+\n" \
                                           "HlUjr8gRsI3qfJOQFy/9rKIJR0Y/8Omwt/8oTWgy1mdeHmmjk7j1nYsvC9JSQ6Zv\n" \
                                           "MldlTTKB3zhThV1+XWYp6rjd5JW1zbVWEkLNxE7GJThEUG3szgBVGP7pSWTUTsqX\n" \
                                           "nLRbwHOoq7hHwg==\n" \
                                           "-----END CERTIFICATE-----\n";

bool Always_Confirm_Status = false;
bool Report_via_Push = false;

// EEPROM
int MaxEEPROM;

//WiFiServer server(80);
WebSocketsClient webSocket;

//Time stuff
unsigned long  StartupTime;
unsigned long  LastNOPTime;
unsigned long  LastKeepAliveRequest;

const unsigned long  RebootAfterThisManySecondsSinceLastStartup = 300000;  // 5 minutes
const unsigned long  RebootAfterThisManySecondsWithoutANOP = 180000;       // 3 minutes
const unsigned long  TwentyFourHours = 86400; // 24 hours * 60 minutes * 60 seconds

unsigned long  secondsSinceLastKeepAliveRequest;

//*****************  Time

void writeEEPROMString(int address, String data)
{

  bool CommmitNeeded = false;

  int _size = data.length();

  if (_size > EEPROM_SIZE) {
    _size = EEPROM_SIZE;
  }

  for (int i = 0; i < _size; i++)
  {

    //EEPROM.update(address + i, data[i]);
    if (data[i] != EEPROM.read(address + i)) {
      EEPROM.write(address + i, data[i]);
      CommmitNeeded = true;
    }

  }

  if ( EEPROM.read(address + _size) != '\0') {
    EEPROM.write(address + _size, '\0');
    CommmitNeeded = true;
  }

  if (CommmitNeeded) {
    EEPROM.commit();
  }

}

String readEEPROMString(int address)
{

  char data[EEPROM_SIZE];
  int len = 0;
  int k;

  k = EEPROM.read(address);

  // (k != 255) handles eeprom never having been set

  if (k != 255) {

    while (k != 0 && len < EEPROM_SIZE)
    {

      data[len] = char(k);

      len++;
      k = EEPROM.read(address + len);

    }
  }

  data[len] = '\0';
  return String(data);

}

void Setup_Time() {

  StartupTime = millis();
  LastNOPTime = millis();

}

void Setup_Max7219() {

  P.begin(); // setup Max 7219
  P.setTextEffect(scrollEffect, scrollEffect);

  slider_scroll_speed = map(scrollSpeed, 1023, 0, 15, 400);
  P.setSpeed(slider_scroll_speed);

}

/*
  void Setup_Buttons() {

  pinMode (External_Button, INPUT);

  }
*/

void Setup_WiFi() {

  bool notyetconnected = true;
  int  counter;

  int TimeBeforeRetryingInSeconds = 3;

  String message;

  DisplayMessageOnMax("Attempting to connect to WiFi", true);

  while (notyetconnected)
  {

    WiFi.mode(WIFI_STA);
    WiFi.begin(wifi_name, wifi_pass);
    delay(1000);

    counter = 0;

    while ( ( WiFi.status() != WL_CONNECTED ) && (counter < TimeBeforeRetryingInSeconds) )
    {
      delay (1000);
      //Serial.print(".");
      counter++;
    }

    if ( WiFi.status() == WL_CONNECTED )
    {
      notyetconnected = false;
      LastTimeWiFiWasConnected = millis();
    }
    else
    {
      TimeBeforeRetryingInSeconds++;

      DisplayMessageOnMax("Attempting to connect to WiFi", true);

      WiFi.disconnect(true);
      delay(1000);

      WiFi.mode(WIFI_STA);
      delay(1000);
    }

  };

  message = "Connected to ";
  message.concat(wifi_name);
  message.concat(" at IP address ");
  message.concat(WiFi.localIP().toString());
  DisplayMessageOnMax(message, true);

};

void Setup_OTAUpdate() {

  // Port defaults to 3232
  // ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
  ArduinoOTA.setHostname("ESP32MsgBoard");

  // No authentication by default
  ArduinoOTA.setPassword("MsgBoard");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA
  .onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  })
  .onEnd([]() {
    Serial.println("\nEnd");
    //ESP.restart();
  })
  .onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  })
  .onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });

  ArduinoOTA.begin();

}

//*****************  every 24 hours send a request to keep the Pushbullet account alive (with out this it would expire every 30 days)

void KeepPushBulletAccountAlive(bool DoCheckNow)
{

  if (SendPushbulletKeepAliveRequest) {

    secondsSinceLastKeepAliveRequest = now() - LastKeepAliveRequest;

    if ( ( secondsSinceLastKeepAliveRequest > TwentyFourHours ) || (DoCheckNow) ) {

      LastKeepAliveRequest = now();

      Serial.print("Sending Pushbullet keep alive request ");

      WiFiClientSecure client;
      client.setCACert(Pushbullet_KeepAlive_root_ca);

      if (!client.connect(KeepAliveHost, https_port)) {
        Serial.println(" Connection failed (check point 4)");
        return;
      }

      String Pushbullet_Message_Out = " { \"name\": \"" + PushBullet_KeepAlive_ID + "\", \"user_iden\": \"" + My_PushBullet_Client_ID  + "\" }";

      client.println("POST / HTTP/1.1");
      client.println("Host: " + String(host));
      client.println("Authorization: Bearer " + My_PushBullet_Access_Token);
      client.println("Content-Type: application/json");
      client.println("Content-Length: " + String(Pushbullet_Message_Out.length()));
      client.println();
      client.println(Pushbullet_Message_Out);

      int WaitLimit = 0;
      while ((!client.available()) && (WaitLimit < 250)) {
        delay(50); //
        WaitLimit++;
      }

      String Response = "";
      WaitLimit = 0;
      while ( (client.connected()) && (WaitLimit < 250) ) {
        String line = client.readStringUntil('\n');
        if (line == "\r") {
          // retrieved header lines can be ignored
          break;
        }
        WaitLimit++;
      }

      while (client.available()) {
        char c = client.read();
        Response += c;
      }

      client.stop();

      if (Response == "{}") {
        Serial.println("succeeded");
      }
      else {
        Serial.println("failed");
        Serial.println(Response);
      }

    }

  }

}

void Setup_PushBullet() {

  DisplayMessageOnMax("Connecting to Pushbullet ...", true);

  String PushBullet_Server_DirectoryAndAccessToken = PushBullet_Server_Directory + My_PushBullet_Access_Token;
  webSocket.beginSSL(PushBullet_Server, PushBullet_Server_Port, PushBullet_Server_DirectoryAndAccessToken);

  webSocket.onEvent(webSocketEvent);      // event handler
  webSocket.setReconnectInterval(5000);   // try ever 5000 again if connection has failed

  GetPushbulletClientID();
  KeepPushBulletAccountAlive(true);

}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {

  static String Last_Pushbullet_Iden;

  switch (type) {

    case WStype_DISCONNECTED:
      Serial.println("Disconnected!");
      break;

    case WStype_CONNECTED:
      webSocket.sendTXT("Connected");
      Serial.println("Pushbullet connection successful!");
      Serial.println();
      break;

    case WStype_TEXT:
      {
        Serial.printf("Incoming: % s\n", payload);

        DynamicJsonDocument jsonDocument(4096);
        deserializeJson(jsonDocument, payload);

        if (jsonDocument["type"] == "nop") {

          Serial.println("nop");
          LastNOPTime = millis();

        }

        if ((jsonDocument["type"] == "tickle") && (jsonDocument["subtype"] == "push")) {

          WiFiClientSecure client1;
          client1.setCACert(Pushbullet_API_root_ca);

          if (!client1.connect(host, https_port)) {
            Serial.println("Connection failed (check point 1)");
            return;
          }
         
          client1.println("GET /v2/pushes?limit=1 HTTP/1.1");        
          client1.println("Host: " + String(host));
          client1.println("Authorization: Bearer " + My_PushBullet_Access_Token);
          client1.println("Content-Type: application/json");
          client1.println("Content-Length: 0");

          client1.println();

          Serial.print(" waiting for the details ");
          int WaitLimit = 0;
          while ((!client1.available()) && (WaitLimit < 250)) {
            delay(50);
            WaitLimit++;
          }

          WaitLimit = 0;
          while ( (client1.connected()) && (WaitLimit < 250) ) {
            String line = client1.readStringUntil('\n');
            if (line == "\r") {
              // retrieved header lines can be ignored
              break;
            }
            WaitLimit++;
          }

          String Response = "";
          while (client1.available()) {
            char c = client1.read();
            Response += c;
          }

          Serial.println(Response);

          client1.stop();

          deserializeJson(jsonDocument, Response);
          String Current_Pushbullet_Iden = jsonDocument["pushes"][0]["iden"];

          if ( Current_Pushbullet_Iden == Last_Pushbullet_Iden) {

            Serial.println(" duplicate - ignoring it");

          }
          else
          {

            Serial.println(" new push : ");
            Last_Pushbullet_Iden = Current_Pushbullet_Iden;

            String Title_Of_Incoming_Push = jsonDocument["pushes"][0]["title"];
            String Body_Of_Incoming_Push = jsonDocument["pushes"][0]["body"];
            bool Dismissed = jsonDocument["pushes"][0]["dismissed"];

            Serial.print(" title = '" + Title_Of_Incoming_Push + "'");
            Serial.print(" ; body = '" + Body_Of_Incoming_Push + "'");

            if (Dismissed) {
              Serial.println(" ; dismissed = true");
            } else {
              Serial.println(" ; dismissed = false");
            }

            Serial.print(" ");

            if ( Title_Of_Incoming_Push == Pushbullet_Note_Title_To_Clear ) {
              DisplayMessageOnMax("", true);
              if (!Dismissed) {
                PushbulletDismissPush(Current_Pushbullet_Iden);
              }
            }

            if ( Title_Of_Incoming_Push == Pushbullet_Note_Title_To_React_To ) {
              DisplayMessageOnMax(String(Body_Of_Incoming_Push), false);
              if (!Dismissed) {
                PushbulletDismissPush(Current_Pushbullet_Iden);
              }
            }

            if ( Title_Of_Incoming_Push == Pushbullet_Note_Title_To_Restart ) {
              DisplayMessageOnMax("Restarting", true);
              if (!Dismissed) {
                PushbulletDismissPush(Current_Pushbullet_Iden);
              }
              ESP.restart();
            }

          }

        }

        break;

      };

    case WStype_BIN:
      Serial.println("Incoming binary data");
      break;

    case WStype_ERROR:
      Serial.println("Error");
      break;

    case WStype_FRAGMENT_TEXT_START:
      Serial.println("Fragment Text Start");
      break;

    case WStype_FRAGMENT_BIN_START:
      Serial.println("Fragment Bin Start");
      break;

    case WStype_FRAGMENT:
      Serial.println("Fragment");
      break;

    case WStype_FRAGMENT_FIN:
      Serial.println("Fragment finish");
      break;

  }

}

void PushbulletDismissPush(String Push_Iden) {

  Serial.println(" dismissing push");

  WiFiClientSecure client;
  client.setCACert(Pushbullet_API_root_ca);

  if (!client.connect(host, https_port)) {
    Serial.println(" Connection failed (check point 2)");
    return;
  }

  String Pushbullet_Message_Out = " { \"dismissed\": true }";
  client.println("POST /v2/pushes/" + Push_Iden + " HTTP/1.1");
  client.println("Host: " + String(host));
  client.println("Authorization: Bearer " + My_PushBullet_Access_Token);
  client.println("Content-Type: application/json");
  client.println("Content-Length: " + String(Pushbullet_Message_Out.length()));
  client.println();
  client.println(Pushbullet_Message_Out);

  int WaitLimit = 0;
  while ((!client.available()) && (WaitLimit < 250)) {
    delay(50); //
    WaitLimit++;
  }

  String Response = "";
  WaitLimit = 0;
  while ( (client.connected()) && (WaitLimit < 250) ) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      // retrieved header lines can be ignored
      break;
    }
    WaitLimit++;
  }

  while (client.available()) {
    char c = client.read();
    Response += c;
  }

  client.stop();
  DynamicJsonDocument jsonDocument(4096);
  deserializeJson(jsonDocument, Response);

  String Dismissed_Pushbullet_Iden = jsonDocument["iden"];
  bool Dismissed_Pushbullet_Status = jsonDocument["dismissed"];

  if ((Dismissed_Pushbullet_Iden == Push_Iden) && (Dismissed_Pushbullet_Status)) {
    Serial.println(" dismiss successful!");
  }
  else {
    Serial.println(" dismiss not successful");
    Serial.println(Response);
  }

}

void GetPushbulletClientID() {

  WiFiClientSecure client;

  client.setCACert(Pushbullet_API_root_ca);
  if (!client.connect(host, https_port)) {
    Serial.println("Connection failed (check point 3)");
    return;
  }

  client.println("GET /v2/users/me HTTP/1.1");
  client.println("Host: " + String(host));
  client.println("Authorization: Bearer " + My_PushBullet_Access_Token);
  client.println("Content-Type: application/json");
  client.println("Content-Length: 0");
  client.println();

  // Serial.print(" waiting for the details ");
  int WaitLimit = 0;
  while ((!client.available()) && (WaitLimit < 250)) {
    delay(50);
    WaitLimit++;
  }

  WaitLimit = 0;
  while ( (client.connected()) && (WaitLimit < 250) ) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      // retrieved header lines can be ignored
      break;
    }
    WaitLimit++;
  }

  String Response = "";
  while (client.available()) {
    char c = client.read();
    Response += c;
  }

  // Serial.println(Response);

  client.stop();

  DynamicJsonDocument jsonDocument(4096);
  deserializeJson(jsonDocument, Response);
  String cid = jsonDocument["iden"];
  My_PushBullet_Client_ID = cid;

}


/*
  void Check_a_Button(int Button_Number, int Pressed) {

  // if the button is pressed, toggle the relay
  if (digitalRead(Button_Number) == Pressed)
  {
    delay(20); // debounce time
    if (digitalRead(Button_Number) == Pressed)
    {

      DisplayMessageOnMax("", true);         // clear display
      delay(1000); // more debounce time

      while (digitalRead(Button_Number) == Pressed) {
        // wait until the button is released
      }

      // added in to reset the msgboard
      ESP.restart();

    }
  }

  }

  void Check_Buttons() {

  Check_a_Button(External_Button, HIGH);  // External button reads HIGH when pressed

  }

*/

void Check_Connection() {

  // if connection has been out for over two minutes restart

  if ( WiFi.status() == WL_CONNECTED) {
    LastTimeWiFiWasConnected = millis();
  }
  else
  {
    if ( (millis() - LastTimeWiFiWasConnected) > TwoMinutes) {
      DisplayMessageOnMax("WiFI connection lost - restarting", true);
      ESP.restart();
    }

  }

}

void Check_Pushbullet()
{
  // Failsafe: If a nop was not received in the specified time (default 2 minutes)
  // and its been over a specified time since startup (default 5 minutes)
  // then restart the system

  unsigned long secondsSinceLastNop = millis() - LastNOPTime;

  if ( secondsSinceLastNop > RebootAfterThisManySecondsWithoutANOP ) {

    secondsSinceStartup = millis() - StartupTime;

    if ( secondsSinceStartup > RebootAfterThisManySecondsSinceLastStartup) {

      Serial.println("Pushbullet triggered restart!");
      DisplayMessageOnMax("Pushbullet connection lost - restarting", true);
      ESP.restart();

    }
  }

}

void Setup_FromEEPROM() {

  EEPROM.begin(EEPROM_SIZE);

  String CurrentMessageSavedInEEPROM;
  CurrentMessageSavedInEEPROM = readEEPROMString(0);

  DisplayMessageOnMax("Setup complete", true);

  if (CurrentMessageSavedInEEPROM != "") {
    DisplayMessageOnMax(CurrentMessageSavedInEEPROM, false);
  }

}


void DisplayMessageOnMax(String message, bool ClearMessageOnceDisplayed) {

  Serial.println(message);

  message.toCharArray(curMessage, message.length() + 1);
  P.displayText(curMessage, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);

  FullyScrollMessageOnMax();

  if (ClearMessageOnceDisplayed) {
    message = "";
    message.toCharArray(curMessage, 1);
    P.displayReset();
  }

  String CurrentMessageSavedInEEPROM = readEEPROMString(0);
  if (CurrentMessageSavedInEEPROM != message) {
    writeEEPROMString(0, message);    // save message on eeprom in case of power failure
  }

}

void FullyScrollMessageOnMax() {

  while ( !P.displayAnimate() ) {
    P.setSpeed(slider_scroll_speed);
  }
  P.displayReset();

}

void setup()
{

  Serial.begin(115200);

  Setup_Time();
  Setup_Max7219();
  DisplayMessageOnMax(ProgramID, true);
  // Setup_Buttons();
  Setup_WiFi();
  Setup_OTAUpdate();
  Setup_PushBullet();
  Setup_FromEEPROM();

}

void loop() {

  Check_Connection();
  Check_Pushbullet();
  KeepPushBulletAccountAlive(false);
  ArduinoOTA.handle();
  // Check_Buttons();
  webSocket.loop();
  FullyScrollMessageOnMax();

}