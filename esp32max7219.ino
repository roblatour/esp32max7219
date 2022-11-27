// ESP32 Message Board v2
// Copyright Rob Latour, 2022 - MIT License
//
// ref: https://hackaday.io/project/170281-voice-controlled-scrolling-message-board
// ref: https://github.com/roblatour/esp32max7219
//
// Notes:
//
//   Before compiling and uploading to your ESP32, please review the key_settings.h file and make updates as you may like.
//
//   The user's WiFi Network name (SSID), Wifi password, and Pushbullet Access Token are not stored in this sketch nor the key_settings.h file.  
//   Rather they are stored in non-volatile memory after being entered by the user via a web browser (see below).
// 
// To operate:
//
//  1. Press and hold the external button for more than 10 seconds when the esp32 is powered on to trigger the process to reset your Wifi Credentials
//     This should be done for initial setup, but may be done anytime afterwards if your Wifi Network name (SSID) or Wifi password change
//
//     1.1 The message board will prompt you to take three steps (which you can do this from your computer or cell phone):
//         Step 1:  connect to Wifi network ScrollingMessageBoard 
//         Step 2:  browse to http://123.123.123.123
//         Step 3:  enter your Wifi network information, and click 'OK'  
//
//  2. Once connected to your Wifi network:
//     
//     2.1 Press and hold the external button for 1 second to have the scrolling message board display the web address at which you can change/clear the text on the scrolling message board
//         for example:  http://192.168.1.100
// 
//     2.2 (Optional) To enter your Pushbullet Access Token, browse to the address identified in step 2.1 with "/pushbullet" (without the quotes) added after it,
//         for example:  http://192.168.1.100/pushbullet
//         For more information on Pushbullet setup please see: https://hackaday.io/project/170281-voice-controlled-scrolling-message-board
//
//     2.3 To change the text on the scrolling message board 
//
//         Method 1: browse to the address identified in step 2.1 above, enter the new text, and press ok
//          
//         Method 2: send a Pushbullet push with the text you would like displayed
//
//         For both methods, here are some special messages (without the quotes) you can use to have special functions performed, these are:
//
//                   "Clear the message board"                  (this clears the message board's display)
//                   "Restart the message board"                (this restarts the message board)
//                   "Clear the memory of the message board"    (this clears the non-volatile memory in which your settings Wifi SSID, Wifi Password, Pushbullet Access Token, and current message are stored
//
//         Note the maximum text message length is about 4,000 characters (which should be more that enough for most use cases)
//
//
//     2.3 To clear the text on the message board:
//         
//         method 1:  Press and hold the external button for more than 5 seconds
//
//         method 2:  Press the clear button on the web page identified in 2.1 above and click 'Clear'
//   
//         method 3:  as described above, send a Pushbullet push with the message "Clear the memory of the message board" (without the quotes)
//
//
// Final Notes:
//
//     If you are using Pushbullet, please note the two certificates stored in the pushbulletCertificates.h file both have expiry dates (as shown in the file)
// 
//     I will endeavor to update this file on Github in the future but there is more information on how you can do this yourself in the pushbulletCertificates.h file 
//     When the certificates expire they will need to be updated and this sketch recompiled and reloaded
// 
//     I am already thinking of ways to make this more seamless in the future ... perhaps a future update?
//
//     Donations welcome at rlatour.com 

//
// Board Manager: DOIT ESP32 DEVKIT V1
//

#include <Arduino.h>
#include <ArduinoJson.h>        // ArdunioJSON by Benoit Blanchon version 6.19.4 https://arduinojson.org/?utm_source=meta&utm_medium=library.properties
#include <WiFi.h>
#include <ESPAsyncWebServer.h>  // https://github.com/esphome/ESPAsyncWebServer (put all files in src directory into the ESPAsyncWebServer directory)
#include <WebSocketsClient.h>   // Websockets by Markus Sattler version 2.3.6 https://github.com/Links2004/arduinoWebSockets
#include <MD_Parola.h>          // MD_Parol by magicDesigns version 3.6.2 https://github.com/MajicDesigns/MD_Parola
#include <MD_MAX72xx.h>         // MD_MAX72XX by magicDesigns version 3.3.1  https://github.com/MajicDesigns/MD_MAX72xx
                                // MD_MAXPanel by magicDesigns version 1.3.1 https://github.com/MajicDesigns/MD_MAXPanel
#include <ArduinoSort.h>        // Arduino Sort by Emil Vikström version https://github.com/emilv/ArduinoSort
#include <time.h>               // Time by Michael Margolis version 1.6.1
#include <TimeLib.h>
#include <EEPROM.h>
#include <ArduinoOTA.h>

#include "key_settings.h"
#include "pushbulletCertificates.h"

//Program ID
const String programID = "ESP32 Message Board v2";

// Connection Pins:
const int externalButtonPin = EXTERNAL_BUTTON_PIN;

// Pins of an ESP32 Dev Kit V1 which connect to the Max7219
const int ClkPin = CLK_PIN;
const int CsPin = CS_PIN;
const int DataPin = DATA_PIN;

// EEPROM stuff
#define EepromSize MAXIMUM_SIZE_OF_NON_VOLATILE_MEMORY
int MaxEEPROM;

// Button stuff
bool buttonCheckUnderway = false;

const int oneSecond = 1000;

const unsigned long buttonDownThresholdForResetingWifiCredentials = 10 * oneSecond;  // 10 seconds 
bool manualResetRequested = false;

const unsigned long buttonDownThresholdForRequestingTheMessageBoardAddress = 1 * oneSecond;  // 1 second
bool messageBoardsAddressRequested = false;                                                  // if you want the board's address to appear as part of the startup message, this can be initially be set to true

const unsigned long buttonDownThresholdForClearingTheMessageBoard = 5 * oneSecond;  // 5 seconds
bool clearMessageRequested = false;

// Max7219 stuff
const int numberOfMax7219Modules = NUMBER_OF_MAX7219_MODULES;

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
MD_Parola P = MD_Parola(HARDWARE_TYPE, CsPin, numberOfMax7219Modules);  // SPI config

textEffect_t scrollEffect = PA_SCROLL_LEFT;
textPosition_t scrollAlign = PA_LEFT;
int scrollSpeed = 1000;
int scrollPause = 0;
int slider_scroll_speed;
bool immediatelyCancelScrollingMessage = false;

#define BUF_SIZE EepromSize + 1       // Maximum characters in a message
char curMessage[BUF_SIZE] = { " " };  // used to hold current message

// Wifi credentials are not stored as literals in this sketch, rather they are stored to and loaded from non-volatile memory (the EEPROM)
String wifiSSID = "";
String wifiPassword = "";

unsigned long LastTimeWiFiWasConnected;
unsigned long secondsSinceStartup;
unsigned long TwoMinutes = 120000;

// Special Commands
const String clearTheMessageBoardCommand = CLEAR_THE_MESSAGE_BOARD_COMMAND;
const String restartCommand = RESTART_COMMAND;
const String clearTheESP32sMemory = CLEAR_ESP32S_MEMORY_COMMAND;

// Pushbullet
String pushbulletAccessToken = "";
String lastPbAccessToken = "";

const String Pushbullet_Title_To_React_To = PUSHBULLET_TITLE_TO_REACT_TO;

const String Pushbullet_Server = "stream.pushbullet.com";
const String Pushbullet_Server_Directory = "/websocket/";
const String Pushbullet_KeepAlive_ID = "rob_messages_active";  // <*** do not change this, it is a special code Pushbullet has provided for use with this program only; used to prevent accounts from expiring for 30 days
const int Pushbullet_Server_Port = 443;
const int https_port = 443;

bool pushbulletIsEnabled = false;
const int lengthOfAValidPushbulletAccessToken = 34;

String My_Pushbullet_Client_ID = "";

bool SendPushbulletKeepAliveRequest = SEND_PUSHBULLET_KEEP_ALIVE_REQUEST;

bool Always_Confirm_Status = false;
bool Report_via_Push = false;

unsigned long StartupTime;
unsigned long LastTimePushbulletWasHeardFrom;
unsigned long LastKeepAliveRequest;

const unsigned long RebootAfterThisManySecondsWithoutHearingFromPushbullet = REBOOT_AFTER_THIS_MANY_MINUTES_WITHOUT_HEARING_FROM_PUSHBULLET * 60 * oneSecond;  // measured in milliseconds
const unsigned long TwentyFourHours = 86400;                                                                                                                   // 24 hours * 60 minutes * 60 seconds (measured in seconds)

unsigned long secondsSinceLastKeepAliveRequest;

WebSocketsClient webSocket;

// WiFi Access Point
const char* AccessPointSSID = ACCESS_POINT_SSID;
IPAddress accessPointIPaddr(123, 123, 123, 123);
IPAddress accessPointIPMask(255, 255, 255, 0);
AsyncWebServer accessPointServer(80);
AsyncEventSource accessPointevents("/events");


String availableNetworks = "";
String lastSSIDSelected = "";
String lastPasswordUsed = "";

// Intranet Web interface stuff
AsyncWebServer server(80);
AsyncEventSource events("/events");

const char* hostName = "admin";

const char* PARAM_INPUT_SSID = "ssid";
const char* PARAM_INPUT_PASSWORD = "password";

const String nothingWasEntered = "**null**";

String inputSSID = nothingWasEntered;
String inputPassword = nothingWasEntered;

String currentMessage = "";
String currentSSID = "";
String currentPassword = "";
String currentPBAccessToken = "";

String htmlFriendlyCurrentMessage = "";
String htmlFriendlyFeedbackColor = "black";
String htmlFriendlyFeedback = "";

const char htmlHeader[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
 body { background: #555; }
 .content {
   max-width: 500px;
   margin: auto;
   background: white;
   padding: 10px;
   word-wrap: break-word;
 }
</style>
</head>
<body>
<div class="content"> 
  <h1><center>Scrolling Message Board</center></h1>
  <p>
    <form action="/get">
      <center>
)rawliteral";

const char htmlFooter[] PROGMEM = R"rawliteral(
     </center>
    </form>    
  </p>
</div>
<script>
       window.history.replaceState('','','/');
</script>
</body>
</html>
)rawliteral";

const char htmlDataEntryWindow[] PROGMEM = R"rawliteral(
     Message:<br>
     <input type="text" name="message" value="$MESSAGE$" width="340" maxlength="4095"><br>
     <br>
     <input type="submit" name="clear" alt="Clear" value="Clear the message">&nbsp&nbsp&nbsp<input type="submit" name="update" alt="Update" value="Update the message"> 
)rawliteral";
const char* PARAM_INPUT_MESSAGE = "message";
const char* PARAM_INPUT_CLEAR = "clear";
const char* PARAM_INPUT_UPDATE = "update";

const char htmlConfirmCleared[] PROGMEM = R"rawliteral(
     The Scrolling Message Board is being cleared.<br>
     <br>
     <input type="submit" name="messageclear" alt="OK" value="OK">  
)rawliteral";
const char* PARAM_MESSAGE_CLEAR = "messageclear";

const char htmlConfirmUpdate[] PROGMEM = R"rawliteral(
     The Scrolling Message Board is being updated with the message:<br>
     <br>
     $MESSAGE$<br>
     <br>
     <input type="submit" name="messageconfirm" alt="OK" value="OK">
)rawliteral";
const char* PARAM_MESSAGE_CONFIRM = "messageconfirm";


const char htmlGetPushbulletCredentials[] PROGMEM = R"rawliteral(
     <br>
     Pushbullet Access Token:&nbsp<input type="password" name="pbaccesstoken" value="$pbaccesstoken$" maxlength="$99$"><br>
     <br>
     <div style="color:$color$">$errors$</div>
     <br>     
     <input type="submit" name="pbUpdate" alt="Update" value="OK"> 
)rawliteral";
const char* PARAM_INPUT_PUSHBULLET_UPDATE = "pbUpdate";
const char* PARAM_INPUT_PUSHBULLET_ACCESS_TOKEN = "pbaccesstoken";

const char htmlConfirmwPusbulletAccessToken[] PROGMEM = R"rawliteral(
     The Pushbullet Access Token has been confirmed.<br>
     <br>
    <input type="submit" name="pbConfirm" alt="Confirmed" value="OK">    
)rawliteral";
const char* PARAM_PUSHBULLET_CONFIRM = "pbConfirm";

const char htmlClearPusbulletAccessToken[] PROGMEM = R"rawliteral(
     The Pushbullet Access Token has been cleared.<br>
     <br>
     The Scrolling Message Board is being restared.<br>     
     <br>
     Please close this window.    
)rawliteral";

const char htmlConfirmPusbulletAccessToken[] PROGMEM = R"rawliteral(
     The Pushbullet Access Token has been updated.<br>
     <br>
     The Scrolling Message Board is being restared.<br>     
     <br>
     Please close this window.    
)rawliteral";

const char htmlGetWifiCredentials[] PROGMEM = R"rawliteral(
     <br>
     Network:&nbsp
     <select name="ssid" id="ssid">
     $options$
     </select>
     <br>
     <br>
     Password:&nbsp<input type="password" name="password" maxlength="63"><br>
     <br>     
     <div style="color:red">$errors$</div>
     <br>    
     <input type="submit" alt="Update" value="OK"> 
)rawliteral";

const char htmlConfirmWIFI[] PROGMEM = R"rawliteral(
     The Wifi credentials entered are now being tested.<br>
     <br>
     If they are good, the Scrolling Message Board will automatically restart and use them from now on.<br><br>
     If in the future they change, just repeat this process by holding down the Scrolling Message Board's button for more than 10 seconds when the Scrolling Message Board is being powered on.<br>
     <br>
     If they are not good, when prompted please reconnect to the '$ACCESS POINT$' network and try again.<br>
     <br>
     Have a great day!<br>
     <br>
     Please close this webpage.<br>
     <br>
)rawliteral";

bool showingConfirmationWindow;

bool setupComplete = false;

void SetupSerial() {

  Serial.begin(115200);
  Serial.println("");
  Serial.println("Starting " + programID);
}

void SetupTime() {

  StartupTime = millis();
  LastTimePushbulletWasHeardFrom = millis();
}

void resetMax7219(bool clear) {

  P.displayReset();

  P.setTextEffect(scrollEffect, scrollEffect);
  slider_scroll_speed = map(scrollSpeed, 1023, 0, 15, 400);
  P.setSpeed(slider_scroll_speed);

  if (clear)
    P.displayClear();
}

void SetupMax7219() {

  P.begin();  // Setup Max 7219
  resetMax7219(false);
}

void IfNeededImmediatelyCancelScrollingMessage() {
  if (!P.displayAnimate())
    immediatelyCancelScrollingMessage = true;
}

void CheckAButton(int Button_Number, int Pressed) {

  // if button was pressed for more than 1 second, set the clear the message flag
  // if button was pressed for more that 15 seconds, set the reset network flag

  if (!buttonCheckUnderway) {

    // if the button is pressed, then reset the message board
    if (digitalRead(Button_Number) == Pressed) {

      unsigned long buttonDownTime = millis();

      delay(20);  // debounce time

      if (digitalRead(Button_Number) == Pressed) {

        buttonCheckUnderway = true;

        while (digitalRead(Button_Number) == Pressed) {

          if (setupComplete) {

            if ((millis() - buttonDownTime) > buttonDownThresholdForRequestingTheMessageBoardAddress) {
              // Serial.println("Message board address requested");
              IfNeededImmediatelyCancelScrollingMessage();
              messageBoardsAddressRequested = true;
            };

            if ((millis() - buttonDownTime) > buttonDownThresholdForClearingTheMessageBoard) {
              // Serial.println("Message board clear requested");
              IfNeededImmediatelyCancelScrollingMessage();
              messageBoardsAddressRequested = false;  // dor't report the address if a clear is requested
              clearMessageRequested = true;
            };

          } else {

            if ((millis() - buttonDownTime) > buttonDownThresholdForResetingWifiCredentials) {
              // Serial.println("Message board Wifi credentials reset requested");
              IfNeededImmediatelyCancelScrollingMessage();
              manualResetRequested = true;
            }
          }
        }
      }
    }
    buttonCheckUnderway = false;
  };
}

void CheckButton() {

  CheckAButton(externalButtonPin, HIGH);  // External button reads HIGH when pressed
}

void FullyScrollMessageOnMax() {

  while ((!P.displayAnimate()) && (!immediatelyCancelScrollingMessage)) {
    P.setSpeed(slider_scroll_speed);  // need to keep this here or the scrolling of the display slows to a crawl
    CheckButton();                    // check if user wants to reset the message board
    webSocket.loop();
  };

  resetMax7219(false);
  immediatelyCancelScrollingMessage = false;
}

void DisplayMessageOnMax(String message, bool displayMessageOnlyOnce) {

  if (message.length() != 0) {
    // display the message

    if (displayMessageOnlyOnce)
      Serial.println("Displaying (only once) : " + message);
    else
      Serial.println("Displaying (forever) : " + message);

    message.toCharArray(curMessage, message.length() + 1);
    P.displayText(curMessage, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);

    FullyScrollMessageOnMax();
  };

  if (displayMessageOnlyOnce) {
    // clear the display
    message = "";
    message.toCharArray(curMessage, 1);
    resetMax7219(true);
  }
}

//*****************  every 24 hours send a request to keep the Pushbullet account alive (with out this it would expire every 30 days)

void KeepPushbulletAccountAlive(bool DoCheckNow) {

  if (!pushbulletIsEnabled) return;

  if (SendPushbulletKeepAliveRequest) {

    secondsSinceLastKeepAliveRequest = now() - LastKeepAliveRequest;

    if ((secondsSinceLastKeepAliveRequest > TwentyFourHours) || (DoCheckNow)) {

      LastKeepAliveRequest = now();

      Serial.print("Sending Pushbullet keep alive request ");

      WiFiClientSecure client;
      client.setCACert(PUSHBULLET_KEEP_ALIVE_ROOT_CA);

      if (!client.connect(PUSHBULLET_KEEP_ALIVE_HOST, https_port)) {
        Serial.println("Connection failed (KeepPushbulletAccountAlive)");
        return;
      }

      String Pushbullet_Message_Out = " { \"name\": \"" + Pushbullet_KeepAlive_ID + "\", \"user_iden\": \"" + My_Pushbullet_Client_ID + "\" }";

      client.println("POST / HTTP/1.1");
      client.println("Host: " + String(PUSHBULLET_API_HOST));
      client.println("Authorization: Bearer " + pushbulletAccessToken);
      client.println("Content-Type: application/json");
      client.println("Content-Length: " + String(Pushbullet_Message_Out.length()));
      client.println();
      client.println(Pushbullet_Message_Out);

      int WaitLimit = 0;
      while ((!client.available()) && (WaitLimit < 250)) {
        delay(50);  //
        WaitLimit++;
      }

      String Response = "";
      WaitLimit = 0;
      while ((client.connected()) && (WaitLimit < 250)) {
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
      } else {
        Serial.println("failed");
        // Serial.println(Response);
      }
    }
  }
}

void SetupPushbullet() {

  Serial.println("Pushbullet setup");

  pushbulletIsEnabled = (pushbulletAccessToken.length() == lengthOfAValidPushbulletAccessToken);

  if (!pushbulletIsEnabled) return;

  DisplayMessageOnMax("Setting up Pushbullet ...", true);
  Serial.println("Pushbullet Access Token: " + pushbulletAccessToken);

  String Pushbullet_Server_DirectoryAndAccessToken = Pushbullet_Server_Directory + pushbulletAccessToken;
  Pushbullet_Server_DirectoryAndAccessToken.trim();
  webSocket.beginSSL(Pushbullet_Server, Pushbullet_Server_Port, Pushbullet_Server_DirectoryAndAccessToken);

  webSocket.onEvent(WebSocketEvent);     // event handler
  webSocket.setReconnectInterval(5000);  // try over 5000 again if connection has failed

  GetPushbulletClientID();
  KeepPushbulletAccountAlive(true);
}

void WebSocketEvent(WStype_t type, uint8_t* payload, size_t length) {

  LastTimePushbulletWasHeardFrom = millis();

  static String Last_Pushbullet_Iden;

  switch (type) {

    case WStype_DISCONNECTED:
      Serial.println("Disconnected!");
      break;

    case WStype_CONNECTED:

      webSocket.sendTXT("Connected");
      Serial.println("Pushbullet connection successful!");
      Serial.println();
      //DisplayMessageOnMax("Pushbullet connection successful!", true);
      break;

    case WStype_TEXT:
      {
        //Serial.printf("Incoming: % s\n", payload);

        DynamicJsonDocument jsonDocument(4096);
        deserializeJson(jsonDocument, payload);

        if (jsonDocument["type"] == "nop") {
          //Serial.println("nop");
        }

        if ((jsonDocument["type"] == "tickle") && (jsonDocument["subtype"] == "push")) {

          WiFiClientSecure client1;
          client1.setCACert(PUSHBULLET_API_ROOT_CA);

          if (!client1.connect(PUSHBULLET_API_HOST, https_port)) {
            Serial.println("Connection failed (WebSocketEvent)");
            //DisplayMessageOnMax("Pushbullet connection failed!", true);
            break;   
          }

          client1.println("GET /v2/pushes?limit=1 HTTP/1.1");
          client1.println("Host: " + String(PUSHBULLET_API_HOST));
          client1.println("Authorization: Bearer " + pushbulletAccessToken);
          client1.println("Content-Type: application/json");
          client1.println("Content-Length: 0");

          client1.println();

          // Serial.print(" waiting for the details ");
          int WaitLimit = 0;
          while ((!client1.available()) && (WaitLimit < 250)) {
            delay(50);
            WaitLimit++;
          }

          WaitLimit = 0;
          while ((client1.connected()) && (WaitLimit < 250)) {
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

          // Serial.println(Response);

          client1.stop();

          deserializeJson(jsonDocument, Response);
          String Current_Pushbullet_Iden = jsonDocument["pushes"][0]["iden"];

          if (Current_Pushbullet_Iden == Last_Pushbullet_Iden) {

            Serial.println(" duplicate - ignoring it");

          } else {

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

            if (Title_Of_Incoming_Push == Pushbullet_Title_To_React_To) {

              if (!Dismissed)
                PushbulletDismissPush(Current_Pushbullet_Iden);

              if (Body_Of_Incoming_Push == restartCommand) {
                DisplayMessageOnMax("Restarting ...", true);
                ESP.restart();
              };

              if (Body_Of_Incoming_Push == clearTheESP32sMemory) {
                DisplayMessageOnMax("Clearing the memory of the message board and restarting ...", true);
                clearTheEEPROM();
                ESP.restart();
              };

              if (Body_Of_Incoming_Push == clearTheMessageBoardCommand) {
                DisplayMessageOnMax(" ", true);
                writeMessageToEEPROM("");

              } else {

                currentMessage = String(Body_Of_Incoming_Push);
                DisplayMessageOnMax(currentMessage, false);
                writeMessageToEEPROM(currentMessage);
              };
            };
          };
        };

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
  };
}

void PushbulletDismissPush(String Push_Iden) {

  // Serial.println(" dismissing push");

  WiFiClientSecure client;
  client.setCACert(PUSHBULLET_API_ROOT_CA);

  if (!client.connect(PUSHBULLET_API_HOST, https_port)) {
    Serial.println("Connection failed (PushbulletDismissPush)");
    return;
  }

  String Pushbullet_Message_Out = " { \"dismissed\": true }";
  client.println("POST /v2/pushes/" + Push_Iden + " HTTP/1.1");
  client.println("Host: " + String(PUSHBULLET_API_HOST));
  client.println("Authorization: Bearer " + pushbulletAccessToken);
  client.println("Content-Type: application/json");
  client.println("Content-Length: " + String(Pushbullet_Message_Out.length()));
  client.println();
  client.println(Pushbullet_Message_Out);

  int WaitLimit = 0;
  while ((!client.available()) && (WaitLimit < 250)) {
    delay(50);  //
    WaitLimit++;
  }

  String Response = "";
  WaitLimit = 0;
  while ((client.connected()) && (WaitLimit < 250)) {
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
    // Serial.println(" dismiss successful!");
  } else {
    // Serial.println(" dismiss not successful");
    // Serial.println(Response);
  }
}

void GetPushbulletClientID() {

  WiFiClientSecure client;

  client.setCACert(PUSHBULLET_API_ROOT_CA);
  if (!client.connect(PUSHBULLET_API_HOST, https_port)) {
    Serial.println("Connection failed (GetPushbulletClientID)");
    return;
  };

  client.println("GET /v2/users/me HTTP/1.1");
  client.println("Host: " + String(PUSHBULLET_API_HOST));
  client.println("Authorization: Bearer " + pushbulletAccessToken);
  client.println("Content-Type: application/json");
  client.println("Content-Length: 0");
  client.println();

  //Serial.print(" waiting for the details ");
  int WaitLimit = 0;
  while ((!client.available()) && (WaitLimit < 250)) {
    delay(50);
    WaitLimit++;
  };

  WaitLimit = 0;
  while ((client.connected()) && (WaitLimit < 250)) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      // retrieved header lines can be ignored
      break;
    }
    WaitLimit++;
  };

  String Response = "";
  while (client.available()) {
    char c = client.read();
    Response += c;
  };

  // Serial.println(Response);

  client.stop();

  DynamicJsonDocument jsonDocument(4096);
  deserializeJson(jsonDocument, Response);
  String cid = jsonDocument["iden"];
  My_Pushbullet_Client_ID = cid;
}


void CheckPushbulletConnection() {
  // Failsafe:
  // If the LastTimePushbulletWasHeardFrom exceeds the specified time (default 10 minutes)
  // then restart the system

  if (!pushbulletIsEnabled) return;

  unsigned long secondsSinceTheLastTimePushbulletWasHeardFrom = millis() - LastTimePushbulletWasHeardFrom;

  if (secondsSinceTheLastTimePushbulletWasHeardFrom > RebootAfterThisManySecondsWithoutHearingFromPushbullet) {

    Serial.println("Pushbullet triggered restart!");
    DisplayMessageOnMax("Pushbullet connection lost - restarting", true);
    ESP.restart();
  }
}

void CheckForNewMessageFromPushbullet() {
  webSocket.loop();
}

void SetupButtons() {

  pinMode(externalButtonPin, INPUT);
}

void writePBAccessTokenToEEPROM(String data) {

  const char fieldSeperator = 254;

  String EEPROMData = wifiSSID + fieldSeperator + wifiPassword + fieldSeperator + data + fieldSeperator + currentMessage;

  writeEEPROMString(0, EEPROMData);

  loadDataFromEEPROM();
};

void writeMessageToEEPROM(String data) {

  const char fieldSeperator = 254;

  String EEPROMData = wifiSSID + fieldSeperator + wifiPassword + fieldSeperator + pushbulletAccessToken + fieldSeperator + data;

  writeEEPROMString(0, EEPROMData);

  // after writing to the EEPROM, the data from the EEPROM is reloaded
  // this is required as the EPPROM can only hold EepromSize bytes (inluding the SSID, Password, and Message)
  // therefore the message lenght can only be equatl to the EepromSize - the SSID length - the Password length - 2
  // the -3 is for: (1) the seperator between the SSID and Password, and (2) the seperator between the Password and the Message
  // thus reloading the data from the EEPROM effectively truncates the message (stored in currMessage) to its maximum length
  loadDataFromEEPROM();
};

void writeEEPROMString(int address, String data) {

  Serial.println("Write to EEPROM: " + data);
  bool CommmitNeeded = false;

  int dataLength = data.length();

  if (dataLength > EepromSize)
    dataLength = EepromSize;

  for (int i = 0; i < dataLength; i++) {

    if (data[i] != EEPROM.read(address + i)) {
      EEPROM.write(address + i, data[i]);
      CommmitNeeded = true;
    }
  }

  if (EEPROM.read(address + dataLength) != '\0') {
    EEPROM.write(address + dataLength, '\0');
    CommmitNeeded = true;
  }

  if (CommmitNeeded)
    EEPROM.commit();
  else
    Serial.println("EEPPROM did not require an update");
}

String readEEPROMString(int address) {

  String returnValue = "";

  int i = address;
  int EEPROMByte = EEPROM.read(i);

  // if the eeprom has previousily had data stored in it, the string will terminate in a character value = 0
  // if the eeprom has not previousily had data stored in it, the string should be terminated right off the bat by a character value = 255
  while ((EEPROMByte != 0) && (EEPROMByte != 255) && (i < EepromSize)) {
    returnValue.concat(char(EEPROMByte));
    EEPROMByte = EEPROM.read(++i);
  };

  Serial.print("Read from EEPROM: ");
  Serial.println(returnValue);

  return returnValue;
}

void loadDataFromEEPROM() {

  // EEPROM data is stored as:
  //    WIFISSID(fieldSeperator)WIFIPassword(fieldSeperator)PushbulletAccessToken(fieldSeperator)Message

  const char fieldSeperator = 254;

  EEPROM.begin(EepromSize);

  String CurrentDataSavedInEEPROM = readEEPROMString(0);

  String EEPROMData[4] = { "", "", "", "" };
  // EEPROMData[0] = SSIDStoredInEEPROM
  // EEPROMData[1] = PasswordStoredInEEPROM
  // EEPROMData[2] = PushbulletAccessTokenStoredInEEPROM
  // EEPROMData[3] = MessageStoredInEEPROM

  if (CurrentDataSavedInEEPROM.length() > 0) {

    int field = 0;

    char c;

    for (int i = 0; i < CurrentDataSavedInEEPROM.length(); i++) {

      c = CurrentDataSavedInEEPROM.charAt(i);

      if ((c == fieldSeperator) && (field < 3))
        field++;
      else
        EEPROMData[field] += String(c);
    };

    wifiSSID = EEPROMData[0];
    wifiPassword = EEPROMData[1];
    pushbulletAccessToken = EEPROMData[2];
    currentMessage = EEPROMData[3];

    Serial.println("Loaded from EEPROM ... SSID: " + wifiSSID + " Password: " + wifiPassword + " Pushbullet Access Token: " + pushbulletAccessToken + " Message: " + currentMessage);
  }
}

void clearTheEEPROM() {

  bool CommmitNeeded = false;

  for (int i = 0; i < EepromSize; i++) {

    if (EEPROM.read(i) != 255) {
      EEPROM.write(i, 255);
      CommmitNeeded = true;
    };
  };

  if (CommmitNeeded)
    EEPROM.commit();
}

void SetupFromEEPROM() {
  loadDataFromEEPROM();
}

void SetupOpeningDisplays() {

  DisplayMessageOnMax(programID, true);
}


bool SetupWiFiWithExistingCredentials(int maxAttempts) {

  // if maxAttempts = 0 there are no limit to the number of attempts that will be made
  // returns true if connected within maxAttempts

  bool notyetconnected = true;

  int timeCounter = 0;
  int attemptCounter = 1;

  int secondsBeforeRetrying = 5;
  const int maxSecondsBeforeRetrying = 60;

  String message;

  while ((notyetconnected) && ((maxAttempts == 0) || ((maxAttempts > 0) && (attemptCounter <= maxAttempts)))) {

    if (maxAttempts > 0)
      DisplayMessageOnMax("Attempting to connect to " + wifiSSID + " - try " + String(attemptCounter) + " of " + String(maxAttempts), true);
    else
      DisplayMessageOnMax("Attempting to connect to " + wifiSSID, true);

    WiFi.mode(WIFI_STA);

    WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
    delay(1000);

    timeCounter = 0;

    while ((WiFi.status() != WL_CONNECTED) && (timeCounter < secondsBeforeRetrying)) {
      delay(1000);
      timeCounter++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      notyetconnected = false;
      LastTimeWiFiWasConnected = millis();
    } else {

      if (secondsBeforeRetrying < maxSecondsBeforeRetrying)
        secondsBeforeRetrying++;

      WiFi.disconnect(true);
      delay(1000);

      WiFi.mode(WIFI_STA);
      delay(1000);
    };

    attemptCounter++;
  };

  if (notyetconnected) {
    message = "Failed to connected to ";
    message.concat(wifiSSID);
  } else {
    message = "Connected to ";
    message.concat(wifiSSID);
  };

  DisplayMessageOnMax(message, true);

  return !notyetconnected;
};

void scanAvailableNetworks() {

  Serial.println("** Scan Networks **");

  int numSsid = WiFi.scanNetworks();

  if (numSsid < 1)

    Serial.println("Couldn't find any wifi networks");

  else {

    // print the list of networks seen:
    Serial.print("Number of available networks: ");
    Serial.println(numSsid);

    // build list of options for WiFi selection window
    // format per SSID found:   <option value="SSID">SSID</option>

    String discoveredSSIDs[numSsid];

    for (int i = 0; i < numSsid; i++) {

      Serial.print(i);
      Serial.print(") ");
      Serial.print(WiFi.SSID(i));
      Serial.print("\tSignal: ");
      Serial.print(WiFi.RSSI(i));
      Serial.println(" dBm");

      discoveredSSIDs[i] = WiFi.SSID(i);
    };

    sortArray(discoveredSSIDs, numSsid);

    availableNetworks = "";
    String lastEntryAdded = "";

    for (int i = 0; i < numSsid; i++) {

      // ensure duplicate SSIDS are not added twice
      if (discoveredSSIDs[i] != lastEntryAdded) {

        lastEntryAdded = discoveredSSIDs[i];

        availableNetworks.concat("<option value=\"");
        availableNetworks.concat(discoveredSSIDs[i]);
        availableNetworks.concat("\">");
        availableNetworks.concat(discoveredSSIDs[i]);
        availableNetworks.concat("</option>");
      };
    };
  };
}

void SetupWifiWithNewCredentials() {

  bool accessPointNeedsToBeConfigured = true;

  while (accessPointNeedsToBeConfigured) {

    String message = message;
    DisplayMessageOnMax("Wifi Network Setup", true);

    Serial.print("Configuring access point...");
    WiFi.softAP(AccessPointSSID);
    WiFi.softAPConfig(accessPointIPaddr, accessPointIPaddr, accessPointIPMask);
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("Access Point IP Address: ");
    Serial.println(myIP);

    accessPointServer.onNotFound([](AsyncWebServerRequest* request) {
      Serial.println("Not found: " + String(request->url()));
      request->redirect("/");
    });

    accessPointServer.on("/confirmed", HTTP_GET, [](AsyncWebServerRequest* request) {
      String html = String(htmlHeader);
      html.concat(htmlConfirmWIFI);
      html.replace("$ACCESS POINT$", String(AccessPointSSID));
      html.concat(htmlFooter);
      request->send(200, "text/html", html);
    });

    accessPointServer.on("/get", HTTP_GET, [](AsyncWebServerRequest* request) {
      String html = String(htmlHeader);
      String errorMessage = "";

      if (request->hasParam(PARAM_INPUT_SSID)) {
        inputSSID = request->getParam(PARAM_INPUT_SSID)->value();
        if (inputSSID.length() == 0) {
          inputSSID = nothingWasEntered;
        };
      } else {
        inputSSID = nothingWasEntered;
      };

      if (inputSSID != nothingWasEntered)
        lastSSIDSelected = inputSSID;

      if (request->hasParam(PARAM_INPUT_PASSWORD)) {
        inputPassword = request->getParam(PARAM_INPUT_PASSWORD)->value();
        if (inputPassword.length() == 0) {
          inputPassword = nothingWasEntered;
        };
      } else {
        inputPassword = nothingWasEntered;
      };

      if (inputPassword != nothingWasEntered)
        lastPasswordUsed = inputPassword;

      Serial.println("------------------");
      Serial.println(inputSSID);
      Serial.println(inputPassword);
      Serial.println("------------------");

      if (inputSSID == nothingWasEntered)
        errorMessage.concat("The SSID was not selected");

      if (inputPassword == nothingWasEntered)
        if (errorMessage.length() == 0)
          errorMessage.concat("The password was not entered");
        else
          errorMessage.concat(";<br>the password was not entered");

      if (errorMessage.length() > 0) {

        errorMessage.concat(".");
        const String quote = String('"');
        html.concat(htmlGetWifiCredentials);
        html.replace("$options$", availableNetworks);

        if (lastSSIDSelected.length() > 0) {
          String original = "<option value=" + quote + lastSSIDSelected + quote + ">" + lastSSIDSelected + "</option>";
          String revised = "<option value=" + quote + lastSSIDSelected + quote + " selected=" + quote + "selected" + quote + ">" + lastSSIDSelected + "</option>";
          html.replace(original, revised);
        };

        if (lastPasswordUsed.length() > 0) {
          String original = "name=" + quote + "password" + quote;
          String revised = "name=" + quote + "password" + quote + " value=" + quote + lastPasswordUsed + quote;
          html.replace(original, revised);
        };

        html.replace("$errors$", errorMessage);
        showingConfirmationWindow = false;

      } else {

        showingConfirmationWindow = true;

        currentSSID = inputSSID;
        currentPassword = inputPassword;

        inputSSID = "";
        inputPassword = "";

        request->redirect("/confirmed");
        return;
      };

      html.concat(htmlFooter);
      request->send(200, "text/html", html);

      resetMax7219(true);
      immediatelyCancelScrollingMessage = true;
    });


    accessPointevents.onConnect([](AsyncEventSourceClient* client) {
      client->send("hello!", NULL, millis(), 1000);
      Serial.println("Access Point server connected");
    });

    accessPointServer.addHandler(&accessPointevents);

    accessPointServer.begin();

    scanAvailableNetworks();

    accessPointServer.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
      String html = String(htmlHeader);
      html.concat(htmlGetWifiCredentials);
      html.replace("$ssid$", "");
      html.replace("$errors$", " ");
      html.replace("$options$", availableNetworks);
      html.concat(htmlFooter);
      request->send(200, "text/html", html);
    });

    Serial.println("Access Point server started");
    message = "Step 1: please connect to Wifi network  " + String(AccessPointSSID) + "      Step 2: browse to http://" + myIP.toString() + "      Step 3: enter your Wifi network information and click 'OK'      ";
    DisplayMessageOnMax(message, false);

    Serial.println("Waiting for user to update Wifi info in browser");

    // assumes network requires a password
    while ((currentSSID == "") || (currentPassword == "")) {
      FullyScrollMessageOnMax();
      delay(10);
    };

    String newSSID = currentSSID;
    String newPassword = currentPassword;
    String newPushbulletAccessToken = currentPBAccessToken;

    wifiSSID = newSSID;
    wifiPassword = newPassword;

    // set the current SSID and Passwordto null
    // these will be reloaded if the Wifi Connection can be established
    currentSSID = "";
    currentPassword = "";

    WiFi.softAPdisconnect(true);
    WiFi.disconnect(true, true);

    while (WiFi.status() == WL_CONNECTED)
      delay(100);

    Serial.println("user has updated Wifi info in browser");

    // confirm Wifi credentials; attempt to connect ten times

    bool WifiSetupSucceeded = false;

    if (SetupWiFiWithExistingCredentials(10)) {

      WifiSetupSucceeded = true;

      // update EEPROM
      //
      // while the stored SSID and password are not needed at this point as they are about to be replaced
      // the stored message is required as it will need to be stored with the new SSID, Password and Pushbullet Access Token.
      // Accordingly, first read the EEPROM which will get the stored SSID, Password, Pushbullet Access Token and Message
      // and then second write the new SSID, new Password, new Pushbullet Access Token and perviousily stored message
      //
      loadDataFromEEPROM();

      const char fieldSeperator = 254;

      String EEPROMData = newSSID + fieldSeperator + newPassword + fieldSeperator + pushbulletAccessToken + fieldSeperator + currentMessage;

      writeEEPROMString(0, EEPROMData);

      DisplayMessageOnMax("Wifi name and password confirmed!", true);
      DisplayMessageOnMax("Automatically restarting ... ", true);
      ESP.restart();
    };

    DisplayMessageOnMax("*** Wifi access failed! ***", true);
    DisplayMessageOnMax("Automatically restarting ... ", true);
    ESP.restart();
  };
};


// ************************************************************************************************************************************************************
void SetupWebServer() {

  showingConfirmationWindow = false;

  server.onNotFound([](AsyncWebServerRequest* request) {
    Serial.println("Not found: " + String(request->url()));
    request->redirect("/");
  });

  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    // present htmlGetPushbulletCredentials window

    String html = String(htmlHeader);

    html.concat(htmlDataEntryWindow);

    // special handling for when a quote is used in the message
    const String quote = String('"');
    const String quoteCode = String("&quot;");
    String htmlFriendlyCurrentMessage = currentMessage;
    htmlFriendlyCurrentMessage.replace(quote, quoteCode);
    html.replace("$MESSAGE$", htmlFriendlyCurrentMessage);
    html.concat(htmlFooter);
    request->send(200, "text/html", html);
  });

  server.on("/confirm_message_clear", HTTP_GET, [](AsyncWebServerRequest* request) {
    // present htmlConfirmCleared window

    String html = String(htmlHeader);

    html.concat(htmlConfirmCleared);

    html.concat(htmlFooter);
    request->send(200, "text/html", html);
  });

  server.on("/confirm_message_update", HTTP_GET, [](AsyncWebServerRequest* request) {
    // present htmlConfirmUpdate window

    String html = String(htmlHeader);
    html.concat(htmlConfirmUpdate);
    html.replace("$MESSAGE$", htmlFriendlyCurrentMessage);

    html.concat(htmlFooter);
    request->send(200, "text/html", html);
  });

  server.on("/pushbullet", HTTP_GET, [](AsyncWebServerRequest* request) {
    // present htmlGetPushbulletCredentials window

    const String quote = String('"');

    String html = String(htmlHeader);

    html.concat(htmlGetPushbulletCredentials);

    html.replace("$99$", String(lengthOfAValidPushbulletAccessToken));

    if (pushbulletAccessToken.length() == lengthOfAValidPushbulletAccessToken)
      html.replace("$pbaccesstoken$", String(pushbulletAccessToken));
    else
      html.replace("$pbaccesstoken$", "");

    html.replace("$errors$", htmlFriendlyFeedback);

    html.replace("$color$", htmlFriendlyFeedbackColor);

    html.concat(htmlFooter);
    request->send(200, "text/html", html);
  });

  server.on("/clearpushbulletaccesstoken", HTTP_GET, [](AsyncWebServerRequest* request) {
    // present confirmwPusbulletAccessToken window

    String html = String(htmlHeader);

    html.concat(htmlClearPusbulletAccessToken);

    html.concat(htmlFooter);
    request->send(200, "text/html", html);

    delay(1000);
    ESP.restart();
  });

  server.on("/confirmpushbulletaccesstoken", HTTP_GET, [](AsyncWebServerRequest* request) {
    // present confirmwPusbulletAccessToken window

    String html = String(htmlHeader);

    html.concat(htmlConfirmPusbulletAccessToken);

    html.concat(htmlFooter);
    request->send(200, "text/html", html);

    delay(1000);
    ESP.restart();
  });

  server.on("/get", HTTP_GET, [](AsyncWebServerRequest* request) {
    //
    htmlFriendlyCurrentMessage = "";
    htmlFriendlyFeedback = "";

    String html = String(htmlHeader);

    if (request->hasParam(PARAM_INPUT_CLEAR)) {
      immediatelyCancelScrollingMessage = true;
      clearMessageRequested = true;
      request->redirect("/confirm_message_clear");
      return;
    };

    if (request->hasParam(PARAM_INPUT_MESSAGE)) {

      String inputMessage = "";
      inputMessage = request->getParam(PARAM_INPUT_MESSAGE)->value();
      inputMessage.trim();

      if (inputMessage == restartCommand) {
        DisplayMessageOnMax("Restarting ...", true);
        ESP.restart();
      };

      if (inputMessage == clearTheESP32sMemory) {
        DisplayMessageOnMax("Clearing the message board's memory and restarting ...", true);
        clearTheEEPROM();
        ESP.restart();
      };

      if ((inputMessage.length() == 0) || (inputMessage == clearTheMessageBoardCommand)) {
        immediatelyCancelScrollingMessage = true;
        writeMessageToEEPROM(" ");
        DisplayMessageOnMax(" ", true);
        request->redirect("/confirm_message_clear");
        return;

      } else {

        writeMessageToEEPROM(inputMessage);  // save input message to eeprom and make it the current message

        // special handling for when a quote is used in the message
        const String quote = String('"');
        const String quoteCode = String("&quot;");
        htmlFriendlyCurrentMessage = currentMessage;
        htmlFriendlyCurrentMessage.replace(quote, quoteCode);

        immediatelyCancelScrollingMessage = true;

        if (currentMessage == "")
          DisplayMessageOnMax(" ", true);
        else
          DisplayMessageOnMax(currentMessage, false);

        request->redirect("/confirm_message_update");
        return;
      };
    };

    if (request->hasParam(PARAM_MESSAGE_CLEAR) || request->hasParam(PARAM_MESSAGE_CONFIRM)) {
      request->redirect("/");
      return;
    };

    if (request->hasParam(PARAM_INPUT_PUSHBULLET_UPDATE)) {

      String inputPBAccessToken = "";
      htmlFriendlyFeedback = "";

      inputPBAccessToken = request->getParam(PARAM_INPUT_PUSHBULLET_ACCESS_TOKEN)->value();

      Serial.println(inputPBAccessToken);

      if (inputPBAccessToken == "") {

        pushbulletIsEnabled = false;
        writePBAccessTokenToEEPROM("");
        request->redirect("/clearpushbulletaccesstoken");
        return;

      } else {

        if (inputPBAccessToken.length() == lengthOfAValidPushbulletAccessToken) {

          writePBAccessTokenToEEPROM(inputPBAccessToken);

          request->redirect("/confirmpushbulletaccesstoken");

        } else {

          pushbulletIsEnabled = false;
          writePBAccessTokenToEEPROM("");
          htmlFriendlyFeedback = "The Pushbullet Access Token appears invalid";
          htmlFriendlyFeedbackColor = "red";
          request->redirect("/pushbullet");
        };
      };
    };


  });

  events.onConnect([](AsyncEventSourceClient* client) {
    client->send("hello!", NULL, millis(), 1000);
    Serial.println("Webserver connected");
  });

  server.addHandler(&events);

  server.begin();

  // request the user enter the message on the webpage
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    String html = String(htmlHeader);
    html.concat(htmlDataEntryWindow);
    html.replace("$MESSAGE$", currentMessage);
    html.concat(htmlFooter);
    request->send(200, "text/html", html);
  });
}

void SetupOTAUpdate() {
  // Port defaults to 3232
  // ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
  ArduinoOTA.setHostname(SECRET_OTA_HostName);

  // No authentication by default
  ArduinoOTA.setPassword(SECRET_OTA_PASSWORD);

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else  // U_SPIFFS
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

void SetupFinalDisplays() {
  DisplayMessageOnMax("Setup complete", true);

  DisplayMessageBoardsAddressAsNeeded();

  DisplayCurrentMessage();
}


void CheckWifiConnection() {
  // if connection has been out for over two minutes restart

  if (WiFi.status() == WL_CONNECTED) {
    LastTimeWiFiWasConnected = millis();
  } else {
    if ((millis() - LastTimeWiFiWasConnected) > TwoMinutes) {
      DisplayMessageOnMax("WiFI connection lost - restarting", true);
      ESP.restart();
    }
  }
}

void DisplayCurrentMessage() {
  if (currentMessage.length() > 0)
    DisplayMessageOnMax(currentMessage, false);
};

void ClearDisplayAsNeeded() {
  if (clearMessageRequested) {

    Serial.println("Clearing the message board");
    writeMessageToEEPROM(" ");
    DisplayMessageOnMax(" ", true);
    clearMessageRequested = false;
  };
}

void DisplayMessageBoardsAddressAsNeeded() {
  if (messageBoardsAddressRequested) {

    String message = "To update the scrolling message please browse to http://";
    message.concat(WiFi.localIP().toString());

    DisplayMessageOnMax(message, true);

    messageBoardsAddressRequested = false;

    // if the message board was previousily displaying a message, resume it now
    DisplayCurrentMessage();
  };
};

void setup() {

  SetupSerial();
  SetupTime();
  SetupMax7219();
  SetupButtons();
  SetupFromEEPROM();
  SetupOpeningDisplays();

  CheckButton();
  if (manualResetRequested || (wifiSSID.length() == 0))
    SetupWifiWithNewCredentials();
  else
    SetupWiFiWithExistingCredentials(0);

  SetupPushbullet();
  SetupWebServer();
  SetupOTAUpdate();
  SetupFinalDisplays();

  setupComplete = true;
}

void loop() {

  CheckWifiConnection();
  CheckPushbulletConnection();
  KeepPushbulletAccountAlive(false);
  ArduinoOTA.handle();

  CheckButton();
  ClearDisplayAsNeeded();
  DisplayMessageBoardsAddressAsNeeded();

  CheckForNewMessageFromPushbullet();

  FullyScrollMessageOnMax();
}
