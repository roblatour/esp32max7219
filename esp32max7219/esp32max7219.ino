// ESP32 Message Board v2.6
// Copyright Rob Latour, 2025 - MIT License
//
// ref: https://hackaday.io/project/170281-voice-controlled-scrolling-message-board
// ref: https://github.com/roblatour/esp32max7219
//
// Notes:
//
//   Before compiling and uploading to your ESP32, please review the key_settings.h file and make updates as you may like.
//
//   The user's WiFi Network name (SSID), Wi-Fi password, and Pushbullet Access Token are not stored in this sketch nor the key_settings.h file.
//   Rather they are stored in non-volatile memory after being entered by the user via a web browser (see below).
//
// To operate:
//
//  1. Press and hold the external button for more than 10 seconds when the esp32 is powered on to trigger the process to reset your Wi-Fi Credentials
//     This should be done for initial setup, but may be done anytime afterwards if your Wi-Fi Network name (SSID) or Wi-Fi password change
//
//     1.1 The message board will prompt you to take three steps (which you can do this from your computer or cell phone):
//         Step 1:  connect to Wi-Fi network ScrollingMessageBoard
//         Step 2:  open your browser in Incognito mode
//         Step 3:  browse to http://123.123.123.123
//         Step 4:  enter your Wi-Fi network information, and click 'OK'
//
//  2. Once connected to your Wi-Fi network:
//
//     2.1 Press and hold the external button for 1 second to have the scrolling message board display the web address at which you can change/clear the text on the scrolling message board
//         for example:  http://192.168.1.100
//
//     2.3 (Optional) To enter a password for updating the message on the Scrolling Message Board, browse to the address identified in step 2.1 with "/password" (without the quotes) added after it,
//         for example:  http://192.168.1.100/password
//         To start, the current password is blank, that is to say nothing is entered in the current password box
//         If the current password is changed from being blank to something else, it can later be changed back to being blank by leaving the new and confirmed new passwords as blank when updating the password
//         If the current password is not blank, the user will need ao enter it to to change the text of the scrolling message board via the web interface (see 2.4 below)
//
//     2.3 (Optional) To enter your Pushbullet Access Token, browse to the address identified in step 2.1 with "/pushbullet" (without the quotes) added after it,
//         for example:  http://192.168.1.100/pushbullet
//         For more information on Pushbullet setup please see: https://hackaday.io/project/170281-voice-controlled-scrolling-message-board
//
//     2.4 To change the text on the scrolling message board
//
//         Method 1: browse to the address identified in step 2.1 above, enter the new text, and click 'OK'
//
//         Method 2: send a Pushbullet push with the text you would like displayed
//
//         For both methods, here are some special messages (without the quotes) you can use to have special functions performed, these are:
//
//                   "Clear the message board"                  (this clears the message board's display)
//                   "Restart the message board"                (this restarts the message board)
//                   "Clear the memory of the message board"    (this clears the non-volatile memory in which your settings Wi-Fi SSID, Wi-Fi Password, Pushbullet Access Token, and current message are stored)
//
//         For Method 1, an additional method exists, it is:
//                   "Show uptime"                              (this will show in the browser the amount of time since the Scrolling Message Board was last started)
//
//          Notes:
//
//             the above special commands are not case sensitive; i.e.  "Show uptime" (without the quotes) and "show uptime" (without the quotes) will work equally as well
//
//             the maximum text message length is about 4,000 characters (which should be more that enough for most use cases)
//
//             if the message starts with "[1]" (without the quotes) it will appear only once
//
//     2.5 To clear the text on the message board:
//
//         method 1:  Press and hold the external button for more than 5 seconds
//
//         method 2:  Click the clear button on the web page identified in 2.1 above
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
//     Donations welcome at https://github.com/roblatour/esp32max7219

//
// Board Manager: esp32 By Espressif Systems - version 2.0.11
//                DOIT ESP32 DEVKIT V1
//
//  Tools:
//  Core Level Debug:                      None
//  Erase all flash before sketch upload:  Disabled
//  Flash frequency:                       80MHz
//  Upload speed:                          92160
//
// Libraries:
// ArduinoJson         6.19.4 Benoit Blanchon https://arduinojson.org/?utm_source=meta&utm_medium=library.properties
// ArduinoOTA          2.0.0  (native to the Arduino IDE libraries)
// ArduinoSort         1.0.0  Emil Vikström https://github.com/emilv/ArduinoSort
// AsyncTCP            1.1.4  dvarrel https://github.com/dvarrel/AsyncTCP
// EEPROM              2.0.0  (native to the Arduino IDE libraries)
// ESP Async WebServer 2.10.8 https://github.com/esphome/ESPAsyncWebServer
// ESPmDNS             2.0.0  (native to the Arduino IDE libraries)
// FS                  2.0.0  (native to the Arduino IDE libraries)
// MD_MAX72XX          3.5.1  magicDesigns https://github.com/MajicDesigns/MD_MAX72xx
// MD_Parola           3.7.3  magicDesigns https://github.com/MajicDesigns/MD_Parola
// MD_MAXPanel         1.4.0  (dependency of MD_Parole - included above)
// SPI                 2.0.0  (native to the Arduino IDE libraries)
// Time                1.6.1  Paul Stoffregen version 1.6.1 https://github.com/PaulStoffregen/Time
// Update              2.0.0  (native to the Arduino IDE libraries)
// WebSockets          2.7.1  Markus Sattler https://github.com/Links2004/arduinoWebSockets
// WiFi                2.0.0  (native to the Arduino IDE libraries)
// WiFiClientSecure    2.0.0  (native to the Arduino IDE libraries)

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsClient.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>

#include <ArduinoSort.h>
#include <time.h>
#include <WiFi.h>
#include <TimeLib.h>

#include <EEPROM.h>
#include <ArduinoOTA.h>

#include "key_settings.h"
#include "pushbulletCertificates.h"

// Program ID
const String programID = "ESP32 Message Board v2.6";

// Connection Pins:
const int externalButtonPin = EXTERNAL_BUTTON_PIN;

// Pins of an ESP32 Dev Kit V1 which connect to the Max7219
const int clkPin = CLK_PIN;
const int csPin = CS_PIN;
const int dataPin = DATA_PIN;

// EEPROM stuff
#define EepromSize MAXIMUM_SIZE_OF_NON_VOLATILE_MEMORY
int maxEEPROM;

// Button stuff
bool buttonCheckUnderway = false;

const int oneSecond = 1000;

const unsigned long buttonDownThresholdForResetingNonVolatileMemory = 10 * oneSecond; // 10 seconds
bool manualResetRequested = false;

const unsigned long buttonDownThresholdForRequestingTheMessageBoardAddress = 1 * oneSecond; // 1 second
bool messageBoardsAddressRequested = false;                                                 // if you want the board's address to appear as part of the startup message, this can be initially be set to true

const unsigned long buttonDownThresholdForClearingTheMessageBoard = 5 * oneSecond; // 5 seconds
bool clearMessageRequested = false;
bool restartRequested = false;
bool EEPROMClearRequested = false;

// Max7219 stuff
const int numberOfMax7219Modules = NUMBER_OF_MAX7219_MODULES;

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
MD_Parola P = MD_Parola(HARDWARE_TYPE, csPin, numberOfMax7219Modules); // SPI config

textEffect_t scrollEffect = PA_SCROLL_LEFT;
textPosition_t scrollAlign = PA_LEFT;
int scrollSpeed = 1000;
int scrollPause = 0;
int slider_scroll_speed;

#define BUF_SIZE EepromSize + 1    // Maximum characters in a message
char curMessage[BUF_SIZE] = {" "}; // used to hold current message

// Wi-Fi credentials are not stored as literals in this sketch, rather they are stored to and loaded from non-volatile memory (the EEPROM)
String wifiSSID = "";
String wifiPassword = "";

// Time stuff
const char *primaryNTPServer = PRIMARY_TIME_SERVER;
const char *secondaryNTPServer = SECONDARY_TIME_SERVER;
const char *tertiaryNTPSever = TERTIARY_TIME_SERVER;
const char *timeZone = MY_TIME_ZONE;

char timeHour[3] = "00";
char timeMin[3] = "00";
char timeSec[3];
String timeAmPm;

unsigned long lastTimeWiFiWasConnected;
unsigned long secondsSinceStartup;
const unsigned long twoMinutes = 120000;

unsigned long nextTimeCheck = 0;
const unsigned long oneWeekFromNow = 7 * 24 * 60 * 60 * 1000;

// Special Commands
const String clearTheESP32sMemory = CLEAR_ESP32S_MEMORY_COMMAND;
const String clearTheMessageBoardCommand = CLEAR_THE_MESSAGE_BOARD_COMMAND;
const String restartCommand = RESTART_COMMAND;
const String showUptime = SHOW_UPTIME_COMMAND;

// Pushbullet
String pushbulletAccessToken = "";
String lastPbAccessToken = "";

const String pushbulletTitleToReactTo = PUSHBULLET_TITLE_TO_REACT_TO;

String pushbulletServer = "stream.pushbullet.com";
String pushbulletServerDirectory = "/websocket/";
const String pushbulletKeepAliveID = "rob_messages_active"; // <*** do not change this, it is a special code Pushbullet has provided for use with this program only; used to prevent accounts from expiring for 30 days

bool pushbulletIsEnabled = false;
const int lengthOfAValidPushbulletAccessToken = 34;

String myPushbulletClientID = "";

bool sendPushbulletKeepAliveRequest = SEND_PUSHBULLET_KEEP_ALIVE_REQUEST;

bool alwaysConfirmStatus = false;
bool reportViaPush = false;

unsigned long startupTime;
unsigned long lastTimePushbulletWasHeardFrom;
unsigned long lastKeepAliveRequest;

const unsigned long rebootAfterThisManySecondsWithoutHearingFromPushbullet = REBOOT_AFTER_THIS_MANY_MINUTES_WITHOUT_HEARING_FROM_PUSHBULLET * 60 * oneSecond; // measured in milliseconds
const unsigned long twentyFourHours = 86400;                                                                                                                  // 24 hours * 60 minutes * 60 seconds (measured in seconds)

unsigned long secondsSinceLastKeepAliveRequest;

WebSocketsClient webSocket;

// WiFi Access Point
const char *accessPointSSID = ACCESS_POINT_SSID;
IPAddress accessPointIPAddr(123, 123, 123, 123);
IPAddress accessPointIPMask(255, 255, 255, 0);
AsyncWebServer accessPointServer(80);
AsyncEventSource accessPointevents("/events");

String availableNetworks = "";
String lastSSIDSelected = "";
String lastPasswordUsed = "";

// Intranet Web interface stuff
AsyncWebServer server(80);
AsyncEventSource events("/events");

const char *hostName = "admin";

const char *PARAM_INPUT_SSID = "ssid";
const char *PARAM_INPUT_PASSWORD = "password";

const String nothingWasEntered = "**null**";

String inputSSID = nothingWasEntered;
String inputPassword = nothingWasEntered;

String currentMessage = "";
String currentMessageTimeAndDate = "";
String currentSSID = "";
String currentPassword = "";
String currentPBAccessToken = "";

String htmlFriendlyCurrentMessage = "";
String htmlFriendlyFeedbackColor = "black";
String htmlFriendlyFeedback = "";

String messageboardPassword = "";

// note: in the htmlHeader the .form-action style is added to cause the button order to be reversed when presented.
// this is used on the htmlDataEntryWindow to make the right most button (the 'Update' button) the default sumbit button.
// without this the left most button would be the default submit button.

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
 .form-actions {
  display: flex;
  flex-direction: row-reverse; /* reverse the elements inside */
  justify-content: center;
  align-items: center;
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

// if you would rather the 'include time and date' checkbox set to checked as a default, use:
// <input type="checkbox" id="includetime" name="includetime" checked><label for="includetime">Include $time$</label><br>

const char htmlDataEntryWindow[] PROGMEM = R"rawliteral(
     Message:<br>
     <input type="text" name="message" value="$MESSAGE$" width="340" maxlength="4095"><br>
     <br>
     <input type="checkbox" id="includetime" name="includetime"><label for="includetime">Include $time$</label><br>
     <br> 
     <div name="passwordneeded">
        Update password:<br><input type="password" name="DEpassword" value="" width="340" maxlength="128"><br><br>
     </div>
     <div name="errormessages" style="color:$color$">
       $errors$
       <br>
       <br>
     </div>
     <div class="form-actions">
         <input type="submit" name="update" alt="Update" value="Update">&nbsp&nbsp&nbsp<input type="submit" name="clear" alt="Clear" value="Clear">
     </div>
)rawliteral";
const char *PARAM_INPUT_MESSAGE = "message";
const char *PARAM_INPUT_CLEAR = "clear";
const char *PARAM_INPUT_UPDATE = "update";
const char *PARAM_INPUT_DATA_ENTRY_PASSWORD = "DEpassword";
const char *PARAM_INPUT_INCLUDETIME = "includetime";

const char htmlConfirmCleared[] PROGMEM = R"rawliteral(
     The Scrolling Message Board is being cleared.<br>
     <br>
     <input type="submit" name="messageclear" alt="OK" value="OK">  
)rawliteral";
const char *PARAM_MESSAGE_CLEAR = "messageclear";

const char htmlConfirmMemoryCleared[] PROGMEM = R"rawliteral(
     The memory of the Scrolling Message Board will be cleared<br>    
     and the Scrolling Message Board restarted.<br>
     <br>
     Please close this window.  
)rawliteral";

const char htmlConfirmRestart[] PROGMEM = R"rawliteral(
     The Scrolling Message Board is being restarted.<br>
     <br>
     Please close this window.  
)rawliteral";

const char htmlConfirmUptime[] PROGMEM = R"rawliteral(
     The Scrolling Message Board has been up for<br>    
     <br>
     $uptime$<br>
     <br>
     <input type="submit" name="uptimeok" alt="OK" value="OK">  
)rawliteral";
const char *PARAM_UPTIME_OK = "uptimeok";

const char htmlConfirmUpdate[] PROGMEM = R"rawliteral(
     The Scrolling Message Board is being updated with the message:<br>
     <br>
     $MESSAGE$<br>
     <br>
     <input type="submit" name="messageconfirm" alt="OK" value="OK">
)rawliteral";
const char *PARAM_MESSAGE_CONFIRM = "messageconfirm";

const char htmlGetPushbulletCredentials[] PROGMEM = R"rawliteral(
     <br>
     Pushbullet Access Token:&nbsp<input type="password" name="pbaccesstoken" value="$pbaccesstoken$" maxlength="$99$"><br>
     <br>
     <div style="color:$color$">$errors$</div>
     <br>     
     <input type="submit" name="pbUpdate" alt="Update" value="OK"> 
)rawliteral";
const char *PARAM_INPUT_PUSHBULLET_UPDATE = "pbUpdate";
const char *PARAM_INPUT_PUSHBULLET_ACCESS_TOKEN = "pbaccesstoken";

const char htmlClearPushbulletAccessToken[] PROGMEM = R"rawliteral(
     The Pushbullet Access Token has been cleared.<br>
     <br>
     The Scrolling Message Board is being restarted.<br>     
     <br>
     Please close this window.    
)rawliteral";

const char htmlConfirmPushbulletAccessToken[] PROGMEM = R"rawliteral(
     The Pushbullet Access Token has been updated.<br>
     <br>
     The Scrolling Message Board is being restarted.<br>     
     <br>
     Please close this window.    
)rawliteral";

const char htmlGetMessageboardPasswordCredentials[] PROGMEM = R"rawliteral(
     <br>
     Password change for webpage updates:<br>
     <br>
     Current password:&emsp;&emsp;&ensp;<input type="password" name="old" value=""><br>
     <br>
     New password:&emsp;&emsp;&emsp;&ensp;&nbsp;<input type="password" name="new1" value=""><br>
     <br>
     Confirm new password:&ensp;<input type="password" name="new2" value=""><br>
     <br>
     <div style="color:$color$">$errors$</div>
     <br>     
     <input type="submit" name="messageboardupdate" alt="Update" value="OK"> 
)rawliteral";
const char *PARAM_INPUT_MESSAGEBOARD_PASSWORD_OLD = "old";
const char *PARAM_INPUT_MESSAGEBOARD_PASSWORD_NEW_1 = "new1";
const char *PARAM_INPUT_MESSAGEBOARD_PASSWORD_NEW_2 = "new2";
const char *PARAM_INPUT_MESSAGEBOARD_PASSWORD_UPDATE = "messageboardupdate";

const char htmlMessageboardPasswordConfirmed[] PROGMEM = R"rawliteral(
     The Message Board's web update password has been changed.<br>
     <br>     
     <input type="submit" name="messageboardupdateconfirm" alt="Update" value="OK">    
)rawliteral";
const char *PARAM_INPUT_MESSAGEBOARD_PASSWORD_UPDATE_CONFIRM = "messageboardupdateconfirm";

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
     The Wi-Fi credentials entered are now being tested.<br>
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

bool setupComplete = false;

void setupSerial()
{

  Serial.begin(115200);
  delay(1000);
  Serial.println("");
  Serial.println("Starting " + programID);
}

bool getNTPTime()
{

  Serial.println("Getting NTP time");

  bool returnValue = false;

  struct tm timeinfo;

  if (getLocalTime(&timeinfo))
  {
    time_t t = mktime(&timeinfo);
    struct timeval now = {.tv_sec = t};
    settimeofday(&now, NULL);
    Serial.printf("Time set to: %s", asctime(&timeinfo));
    Serial.println();
    returnValue = true;
  };

  return returnValue;
}

void setupTime()
{

  const unsigned long oneWeekFromNow = 7 * 24 * 60 * 60 * 1000;

  Serial.println("Setting time ... ");

  bool timeWasSuccessfullySet = false;

  configTime(0, 0, primaryNTPServer, secondaryNTPServer, tertiaryNTPSever);
  setenv("TZ", timeZone, 1);
  tzset();

  timeWasSuccessfullySet = getNTPTime();

  if (!timeWasSuccessfullySet)
  {

    Serial.println("Time could not be set from NTP server");

    const int epochYear = 1900;
    const int monthOffset = 1;

    struct tm timeinfo;
    timeinfo.tm_year = 2023 - epochYear;
    timeinfo.tm_mon = 1 - monthOffset;
    timeinfo.tm_mday = 1;
    timeinfo.tm_hour = 00;
    timeinfo.tm_min = 00;
    timeinfo.tm_sec = 00;

    time_t t = mktime(&timeinfo);
    struct timeval now = {.tv_sec = t};
    settimeofday(&now, NULL);

    nextTimeCheck = millis() + oneWeekFromNow;
  };

  startupTime = millis();

  lastTimePushbulletWasHeardFrom = millis();
}

void refreshTimeOnceAWeek()
{

  if (millis() > nextTimeCheck)
  {

    const unsigned long oneWeekFromNow = 7 * 24 * 60 * 60 * 1000;
    const unsigned long oneHourFromNow = 60 * 60 * 1000;

    if (getNTPTime())
      nextTimeCheck = millis() + oneWeekFromNow;
    else
      nextTimeCheck = millis() + oneHourFromNow;
  };
}

void resetMax7219(bool clear)
{

  P.displayReset();

  P.setTextEffect(scrollEffect, scrollEffect);
  slider_scroll_speed = map(scrollSpeed, 1023, 0, 15, 400);
  P.setSpeed(slider_scroll_speed);

  if (clear)
    P.displayClear();
}

void setupMax7219()
{

  P.begin(); // Setup Max 7219
  resetMax7219(false);
}

void checkAButton(int Button_Number, int Pressed)
{

  // if button was pressed for more than 1 second, set the clear the message flag
  // if button was pressed for more that 15 seconds, set the reset network flag

  if (!buttonCheckUnderway)
  {

    // if the button is pressed, then reset the message board
    if (digitalRead(Button_Number) == Pressed)
    {

      unsigned long buttonDownTime = millis();

      delay(20); // debounce time

      if (digitalRead(Button_Number) == Pressed)
      {

        buttonCheckUnderway = true;

        while (digitalRead(Button_Number) == Pressed)
        {

          if (setupComplete)
          {

            if ((millis() - buttonDownTime) > buttonDownThresholdForRequestingTheMessageBoardAddress)
            {
              Serial.println("Message board address requested");
              messageBoardsAddressRequested = true;
            };

            if ((millis() - buttonDownTime) > buttonDownThresholdForClearingTheMessageBoard)
            {
              Serial.println("Message board clear requested");
              clearMessageRequested = true;
            };
          }
          else
          {

            if ((millis() - buttonDownTime) > buttonDownThresholdForResetingNonVolatileMemory)
            {
              manualResetRequested = true;
            }
          }
        }
      }
    }
    buttonCheckUnderway = false;
  };
}

void checkButton()
{

  checkAButton(externalButtonPin, HIGH); // External button reads HIGH when pressed
}

void fullyScrollMessageOnMax()
{

  while (!P.displayAnimate() && (!clearMessageRequested))
  {

    P.setSpeed(slider_scroll_speed); // need to keep this here or the scrolling of the display slows to a crawl
    checkButton();                   // check if user wants to reset the message board
    webSocket.loop();
  };

  resetMax7219(false);
}

void displayMessageOnMax(String message, bool displayMessageOnlyOnce)
{

  if (message.length() != 0)
  {

    // display the message

    if (displayMessageOnlyOnce)
      Serial.println("Displaying (only once) : " + message);
    else
      Serial.println("Displaying (forever) : " + message);

    message.toCharArray(curMessage, message.length() + 1);
    P.displayText(curMessage, scrollAlign, scrollSpeed, scrollPause, scrollEffect, scrollEffect);

    if (setupComplete)
      webSocket.loop();

    // FullyScrollMessageOnMax(); - do not uncomment this line as it causes watchdog problems. Rather let the FullyScrollMessageOnMax(); coded in the loop() subroutine handle it
  };

  if (displayMessageOnlyOnce)
  {

    // display the message once
    fullyScrollMessageOnMax();

    // clear the display
    message = "";
    message.toCharArray(curMessage, 1);
    resetMax7219(true);
  }
}

void displayTheCurrentMessageOnMax(bool displayMessageOnlyOnce)
{

  if (currentMessage.length() > 0)
  {
    String fullMessage = currentMessage + currentMessageTimeAndDate;
    displayMessageOnMax(fullMessage, displayMessageOnlyOnce);
  };
}

//*****************  every 24 hours send a request to keep the Pushbullet account alive (with out this it would expire every 30 days)

void keepPushbulletAccountAlive(bool DoCheckNow)
{

  if (!pushbulletIsEnabled)
    return;

  if (sendPushbulletKeepAliveRequest)
  {

    secondsSinceLastKeepAliveRequest = now() - lastKeepAliveRequest;

    if ((secondsSinceLastKeepAliveRequest > twentyFourHours) || (DoCheckNow))
    {

      lastKeepAliveRequest = now();

      Serial.print("Sending Pushbullet keep alive request ");

      WiFiClientSecure client;
      client.setCACert(ZEBRA_PUSHBULLET_COM_CERTIFICATE);

      if (!client.connect(PUSHBULLET_KEEP_ALIVE_HOST, PUSHBULLET_KEEP_ALIVE_PORT))
      {
        Serial.println("Connection failed (KeepPushbulletAccountAlive)");
        return;
      }

      String Pushbullet_Message_Out = " { \"name\": \"" + pushbulletKeepAliveID + "\", \"user_iden\": \"" + myPushbulletClientID + "\" }";

      client.println("POST / HTTP/1.1");
      client.println("Host: " + String(PUSHBULLET_KEEP_ALIVE_HOST));
      client.println("Authorization: Bearer " + pushbulletAccessToken);
      client.println("Content-Type: application/json");
      client.println("Content-Length: " + String(Pushbullet_Message_Out.length()));
      client.println();
      client.println(Pushbullet_Message_Out);

      int WaitLimit = 0;
      while ((!client.available()) && (WaitLimit < 250))
      {
        delay(50); //
        WaitLimit++;
      }

      String Response = "";
      WaitLimit = 0;
      while ((client.connected()) && (WaitLimit < 250))
      {
        String line = client.readStringUntil('\n');
        if (line == "\r")
        {
          // retrieved header lines can be ignored
          break;
        }
        WaitLimit++;
      }

      while (client.available())
      {
        char c = client.read();
        Response += c;
      }

      client.stop();

      if (Response == "{}")
      {
        Serial.println("succeeded");
      }
      else
      {
        Serial.println("failed");
        Serial.println(Response);
      }
    }
  }
}

void setupPushbullet()
{

  Serial.println("Pushbullet setup");

  if (pushbulletAccessToken.length() == 0)
  {
    Serial.println("Pushbullet Access Token has not been setup");
    pushbulletIsEnabled = false;
    return;
  }
  else if (pushbulletAccessToken.length() != lengthOfAValidPushbulletAccessToken)
  {
    Serial.println("Invalid Pushbullet Access Token");
    pushbulletIsEnabled = false;
    return;
  };

  pushbulletIsEnabled = true;

  displayMessageOnMax("Setting up Pushbullet ...", true);

  String Pushbullet_Server_DirectoryAndAccessToken = pushbulletServerDirectory + pushbulletAccessToken;
  Pushbullet_Server_DirectoryAndAccessToken.trim();

  webSocket.beginSSL(pushbulletServer, PUSHBULLET_SERVER_PORT, Pushbullet_Server_DirectoryAndAccessToken);
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);

  getPushbulletClientID();
  keepPushbulletAccountAlive(true);
}

void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{

  lastTimePushbulletWasHeardFrom = millis();

  static String Last_Pushbullet_Iden;

  switch (type)
  {

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
    // Serial.printf("Incoming: % s\n", payload);

    DynamicJsonDocument jsonDocument(4096);
    deserializeJson(jsonDocument, payload);

    if (jsonDocument["type"] == "nop")
    {
      Serial.println("nop");
    }

    if ((jsonDocument["type"] == "tickle") && (jsonDocument["subtype"] == "push"))
    {

      WiFiClientSecure client1;
      client1.setCACert(API_PUSHBULLET_COM_CERTIFICATE);

      if (!client1.connect(PUSHBULLET_API_HOST, PUSHBULLET_SERVER_PORT))
      {
        Serial.println("Connection failed (WebSocketEvent)");
        // DisplayMessageOnMax("Pushbullet connection failed!", true);
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
      while ((!client1.available()) && (WaitLimit < 250))
      {
        delay(50);
        WaitLimit++;
      }

      WaitLimit = 0;
      while ((client1.connected()) && (WaitLimit < 250))
      {
        String line = client1.readStringUntil('\n');
        if (line == "\r")
        {
          // retrieved header lines can be ignored
          break;
        }
        WaitLimit++;
      }

      String Response = "";
      while (client1.available())
      {
        char c = client1.read();
        Response += c;
      }

      // Serial.println(Response);

      client1.stop();

      deserializeJson(jsonDocument, Response);
      String Current_Pushbullet_Iden = jsonDocument["pushes"][0]["iden"];

      if (Current_Pushbullet_Iden == Last_Pushbullet_Iden)
      {

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

        if (Dismissed)
        {
          Serial.println(" ; dismissed = true");
        }
        else
        {
          Serial.println(" ; dismissed = false");
        }

        Serial.print(" ");

        if (Title_Of_Incoming_Push == pushbulletTitleToReactTo)
        {

          if (!Dismissed)
            pushbulletDismissPush(Current_Pushbullet_Iden);

          if (stringsAreMatchRegardlessOfCase(Body_Of_Incoming_Push, restartCommand))
          {
            Body_Of_Incoming_Push = "";
            restartRequested = true;
          };

          if (stringsAreMatchRegardlessOfCase(Body_Of_Incoming_Push, clearTheESP32sMemory))
          {
            Body_Of_Incoming_Push = "";
            EEPROMClearRequested = true;
            restartRequested = true;
          };

          if (stringsAreMatchRegardlessOfCase(Body_Of_Incoming_Push, clearTheMessageBoardCommand))
          {
            Body_Of_Incoming_Push = "";
            clearMessageRequested = true;
          };

          if (Body_Of_Incoming_Push != "")
          {
            currentMessage = String(Body_Of_Incoming_Push);
            displayTheCurrentMessage();
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

void pushbulletDismissPush(String Push_Iden)
{

  // Serial.println(" dismissing push");

  WiFiClientSecure client;
  client.setCACert(API_PUSHBULLET_COM_CERTIFICATE);

  if (!client.connect(PUSHBULLET_API_HOST, PUSHBULLET_SERVER_PORT))
  {
    Serial.println("Connection failed (PushbulletDismissPush)");
    return;
  }

  String pushbullet_Message_Out = " { \"dismissed\": true }";
  client.println("POST /v2/pushes/" + Push_Iden + " HTTP/1.1");
  client.println("Host: " + String(PUSHBULLET_API_HOST));
  client.println("Authorization: Bearer " + pushbulletAccessToken);
  client.println("Content-Type: application/json");
  client.println("Content-Length: " + String(pushbullet_Message_Out.length()));
  client.println();
  client.println(pushbullet_Message_Out);

  int waitLimit = 0;
  while ((!client.available()) && (waitLimit < 250))
  {
    delay(50); //
    waitLimit++;
  }

  String response = "";
  waitLimit = 0;
  while ((client.connected()) && (waitLimit < 250))
  {
    String line = client.readStringUntil('\n');
    if (line == "\r")
    {
      // retrieved header lines can be ignored
      break;
    }
    waitLimit++;
  }

  while (client.available())
  {
    char c = client.read();
    response += c;
  }

  client.stop();
  DynamicJsonDocument jsonDocument(4096);
  deserializeJson(jsonDocument, response);

  String dismissed_Pushbullet_Iden = jsonDocument["iden"];
  bool dismissed_Pushbullet_Status = jsonDocument["dismissed"];

  /*
  if ((dismissed_Pushbullet_Iden == Push_Iden) && (dismissed_Pushbullet_Status))
  {
     Serial.println(" dismiss successful!");
  }
  else
  {
    Serial.println(" dismiss not successful");
    Serial.println(Response);
  }
  */
}

void getPushbulletClientID()
{
  if (pushbulletAccessToken.length() != lengthOfAValidPushbulletAccessToken)
  {
    Serial.println("GetPushbulletClientID - Invalid Pushbullet Access Token");
    return;
  }

  WiFiClientSecure client;

  client.setCACert(API_PUSHBULLET_COM_CERTIFICATE);
  if (!client.connect(PUSHBULLET_API_HOST, PUSHBULLET_SERVER_PORT))
  {
    Serial.println("Connection failed (GetPushbulletClientID)");
    return;
  };

  client.println("GET /v2/users/me HTTP/1.1");
  client.println("Host: " + String(PUSHBULLET_API_HOST));
  client.println("Authorization: Bearer " + pushbulletAccessToken);
  client.println("Content-Type: application/json");
  client.println("Content-Length: 0");
  client.println();

  // Serial.print(" waiting for the details ");
  int waitLimit = 0;
  while ((!client.available()) && (waitLimit < 250))
  {
    delay(50);
    waitLimit++;
  };

  waitLimit = 0;
  while ((client.connected()) && (waitLimit < 250))
  {
    String line = client.readStringUntil('\n');
    if (line == "\r")
    {
      // retrieved header lines can be ignored
      break;
    }
    waitLimit++;
  };

  String response = "";
  while (client.available())
  {
    char c = client.read();
    response += c;
  };

  // Serial.println(Response);

  client.stop();

  DynamicJsonDocument jsonDocument(4096);
  deserializeJson(jsonDocument, response);
  String cid = jsonDocument["iden"];
  myPushbulletClientID = cid;
}

void checkPushbulletConnection()
{
  // Failsafe:
  // If the LastTimePushbulletWasHeardFrom exceeds the specified time (default 10 minutes)
  // then restart the system

  if (!pushbulletIsEnabled)
    return;

  unsigned long secondsSinceTheLastTimePushbulletWasHeardFrom = millis() - lastTimePushbulletWasHeardFrom;

  if (secondsSinceTheLastTimePushbulletWasHeardFrom > rebootAfterThisManySecondsWithoutHearingFromPushbullet)
  {

    Serial.println("Pushbullet triggered restart!");
    displayMessageOnMax("Pushbullet connection lost", true);
    restartRequested = true;
  }
}

void keepPushbulletSessionAlive()
{
  webSocket.loop();
}

void setupButtons()
{

  pinMode(externalButtonPin, INPUT);
}

void writeMessageBoardPasswordToEEPROM(String data)
{

  char highValues = 255;
  char fieldSeparator = 254;

  data.replace(String(highValues), String(""));
  data.replace(String(fieldSeparator), String(""));

  String EEPROMData = wifiSSID + fieldSeparator + wifiPassword + fieldSeparator + pushbulletAccessToken + fieldSeparator + data + fieldSeparator + currentMessage + fieldSeparator + currentMessageTimeAndDate;

  writeEEPROMString(0, EEPROMData);

  loadDataFromEEPROM();
};

void writePBAccessTokenToEEPROM(String data)
{

  const char highValues = 255;
  const char fieldSeparator = 254;

  data.replace(String(highValues), String(""));
  data.replace(String(fieldSeparator), String(""));

  String EEPROMData = wifiSSID + fieldSeparator + wifiPassword + fieldSeparator + data + fieldSeparator + messageboardPassword + fieldSeparator + currentMessage + fieldSeparator + currentMessageTimeAndDate;

  writeEEPROMString(0, EEPROMData);

  loadDataFromEEPROM();
};

void writeMessageToEEPROM(String data)
{

  const char highValues = 255;
  const char fieldSeparator = 254;
  data.replace(String(highValues), String(""));
  // note: data may contain a field separator (separating the message from the time of the message)

  String EEPROMData = wifiSSID + fieldSeparator + wifiPassword + fieldSeparator + pushbulletAccessToken + fieldSeparator + messageboardPassword + fieldSeparator + data;

  writeEEPROMString(0, EEPROMData);

  // after writing to the EEPROM, the data from the EEPROM is reloaded
  // this is required as the EEPROM can only hold EepromSize bytes (including the SSID, Password, and Message)
  // therefore the message length can only be equal to the EepromSize - the SSID length - the Password length - 2
  // the -3 is for: (1) the separator between the SSID and Password, and (2) the separator between the Password and the Message
  // thus reloading the data from the EEPROM effectively truncates the message (stored in currMessage) to its maximum length
  loadDataFromEEPROM();
};

void writeEEPROMString(int address, String data)
{

  Serial.println("Write to EEPROM: " + data);
  bool commitNeeded = false;

  int dataLength = data.length();

  if (dataLength > EepromSize)
    dataLength = EepromSize;

  for (int i = 0; i < dataLength; i++)
  {

    if (data[i] != EEPROM.read(address + i))
    {
      EEPROM.write(address + i, data[i]);
      commitNeeded = true;
    }
  }

  if (EEPROM.read(address + dataLength) != '\0')
  {
    EEPROM.write(address + dataLength, '\0');
    commitNeeded = true;
  }

  if (commitNeeded)
    EEPROM.commit();
  else
    Serial.println("EEPROM did not require an update");
}

String readEEPROMString(int address)
{

  String returnValue = "";

  int i = address;
  int EEPROMByte = EEPROM.read(i);

  // if the eeprom has previously had data stored in it, the string will terminate in a character value = 0
  // if the eeprom has not previously had data stored in it, the string should be terminated right off the bat by a character value = 255
  while ((EEPROMByte != 0) && (EEPROMByte != 255) && (i < EepromSize))
  {
    returnValue.concat(char(EEPROMByte));
    EEPROMByte = EEPROM.read(++i);
  };

  Serial.print("Read from EEPROM: ");
  Serial.println(returnValue);

  return returnValue;
}

void loadDataFromEEPROM()
{

  // EEPROM data is stored as:
  //    WiFiSSID(fieldSeparator)WiFiPassword(fieldSeparator)PushbulletAccessToken(fieldSeparator)messageboardPassword(fieldSeparator)currentMessage(fieldSeparator)currentMessageTimeAndDate

  const char fieldSeparator = 254;

  EEPROM.begin(EepromSize);

  String currentDataSavedInEEPROM = readEEPROMString(0);

  String EEPROMData[6] = {"", "", "", "", "", ""};
  // EEPROMData[0] = Wi-Fi network name (SSID)
  // EEPROMData[1] = Wi-Fi password
  // EEPROMData[2] = Pushbullet Access Token
  // EEPROMData[3] = Password to update message from web site (default is null, i.e.: "")
  // EEPROMData[4] = Message
  // EEPROMData[5] = Message's time and date

  if (currentDataSavedInEEPROM.length() > 0)
  {

    int field = 0;

    char c;

    for (int i = 0; i < currentDataSavedInEEPROM.length(); i++)
    {

      c = currentDataSavedInEEPROM.charAt(i);

      if ((c == fieldSeparator) && (field < 5))
        field++;
      else
        EEPROMData[field] += String(c);
    };

    wifiSSID = EEPROMData[0];
    wifiPassword = EEPROMData[1];
    pushbulletAccessToken = EEPROMData[2];
    messageboardPassword = EEPROMData[3];
    currentMessage = EEPROMData[4];
    currentMessageTimeAndDate = EEPROMData[5];

    Serial.println("Loaded from EEPROM ... SSID: " + wifiSSID + " Password: " + wifiPassword + " Pushbullet Access Token: " + pushbulletAccessToken + " Website update password: " + messageboardPassword + " Message: " + currentMessage + " Message's Time and Date: " + currentMessageTimeAndDate);
  }
}

void clearTheEEPROM()
{

  Serial.println("Start EEPROM clear");

  for (int i = 0; i < EepromSize; i++)
    if (EEPROM.read(i) != 255)
      EEPROM.write(i, 255);

  EEPROM.commit();
  delay(5000); // allow time for commit to happen

  Serial.println("End EEPROM clear");
}

void setupFromEEPROM()
{
  loadDataFromEEPROM();
}

void setupOpeningDisplays()
{

  displayMessageOnMax(programID, true);
}

bool setupWiFiWithExistingCredentials(int maxAttempts)
{

  // if maxAttempts = 0 there are no limit to the number of attempts that will be made
  // returns true if connected within maxAttempts

  bool notYetConnected = true;

  int timeCounter = 0;
  int attemptCounter = 1;

  int secondsBeforeRetrying = 5;
  const int maxSecondsBeforeRetrying = 60;

  String message;

  while ((notYetConnected) && ((maxAttempts == 0) || ((maxAttempts > 0) && (attemptCounter <= maxAttempts))))
  {

    if (maxAttempts > 0)
      displayMessageOnMax("Attempting to connect to " + wifiSSID + " - try " + String(attemptCounter) + " of " + String(maxAttempts), true);
    else
      displayMessageOnMax("Attempting to connect to " + wifiSSID, true);

    WiFi.mode(WIFI_STA);

    WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
    delay(1000);

    timeCounter = 0;

    while ((WiFi.status() != WL_CONNECTED) && (timeCounter < secondsBeforeRetrying))
    {
      delay(1000);
      timeCounter++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
      notYetConnected = false;
      lastTimeWiFiWasConnected = millis();
    }
    else
    {

      if (secondsBeforeRetrying < maxSecondsBeforeRetrying)
        secondsBeforeRetrying++;

      WiFi.disconnect(true);
      delay(1000);

      WiFi.mode(WIFI_STA);
      delay(1000);
    };

    attemptCounter++;
  };

  if (notYetConnected)
  {
    message = "Failed to connected to ";
    message.concat(wifiSSID);
  }
  else
  {
    message = "Connected at ";
    message.concat(WiFi.localIP().toString());
  };

  displayMessageOnMax(message, true);

  return !notYetConnected;
};

void scanAvailableNetworks()
{

  Serial.println("** Scan Networks **");

  int numSsid = WiFi.scanNetworks();

  if (numSsid < 1)

    Serial.println("Couldn't find any Wi-Fi networks");

  else
  {

    // print the list of networks seen:
    Serial.print("Number of available networks: ");
    Serial.println(numSsid);

    // build list of options for WiFi selection window
    // format per SSID found:   <option value="SSID">SSID</option>

    String discoveredSSIDs[numSsid];

    for (int i = 0; i < numSsid; i++)
    {

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

    for (int i = 0; i < numSsid; i++)
    {

      // ensure duplicate SSIDS are not added twice
      if (discoveredSSIDs[i] != lastEntryAdded)
      {

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

void setupWifiWithNewCredentials()
{

  bool accessPointNeedsToBeConfigured = true;

  while (accessPointNeedsToBeConfigured)
  {

    String message = message;
    displayMessageOnMax("Wi-Fi Network Setup", true);

    Serial.print("Configuring access point...");
    WiFi.softAP(accessPointSSID);
    WiFi.softAPConfig(accessPointIPAddr, accessPointIPAddr, accessPointIPMask);
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("Access Point IP Address: ");
    Serial.println(myIP);

    accessPointServer.onNotFound([](AsyncWebServerRequest *request)
                                 {
      Serial.println("Not found: " + String(request->url()));
      request->redirect("/"); });

    accessPointServer.on("/confirmed", HTTP_GET, [](AsyncWebServerRequest *request)
                         {
      String html = String(htmlHeader);
      html.concat(htmlConfirmWIFI);
      html.replace("$ACCESS POINT$", String(accessPointSSID));
      html.concat(htmlFooter);
      request->send(200, "text/html", html); });

    accessPointServer.on("/get", HTTP_GET, [](AsyncWebServerRequest *request)
                         {
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
          String revised_ = "<option value=" + quote + lastSSIDSelected + quote + " selected=" + quote + "selected" + quote + ">" + lastSSIDSelected + "</option>";
          html.replace(original, revised_);
        };

        if (lastPasswordUsed.length() > 0) {
          String original = "name=" + quote + "password" + quote;
          String revised_ = "name=" + quote + "password" + quote + " value=" + quote + lastPasswordUsed + quote;
          html.replace(original, revised_);
        };

        html.replace("$errors$", errorMessage);

      } else {

        currentSSID = inputSSID;
        currentPassword = inputPassword;

        inputSSID = "";
        inputPassword = "";

        request->redirect("/confirmed");
        return;
      };

      html.concat(htmlFooter);
      request->send(200, "text/html", html);

      resetMax7219(true); });

    accessPointevents.onConnect([](AsyncEventSourceClient *client)
                                {
      // client->send("hello!", NULL, millis(), 1000);
      Serial.println("Access Point server connected"); });

    accessPointServer.addHandler(&accessPointevents);

    accessPointServer.begin();

    scanAvailableNetworks();

    accessPointServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
                         {
      String html = String(htmlHeader);
      html.concat(htmlGetWifiCredentials);
      html.replace("$ssid$", "");
      html.replace("$errors$", " ");
      html.replace("$options$", availableNetworks);
      html.concat(htmlFooter);
      request->send(200, "text/html", html); });

    Serial.println("Access Point server started");
    message = "Step 1: please connect to Wi-Fi network  " + String(accessPointSSID) + "     Step 2: open your browser in Incognito mode     Step 3: browse to http://" + myIP.toString() + "     Step 4: enter your Wi-Fi network information and click 'OK'";
    displayMessageOnMax(message, false);

    Serial.println("Waiting for user to update Wi-Fi info in browser");

    // assumes network requires a password
    while ((currentSSID == "") || (currentPassword == ""))
    {
      fullyScrollMessageOnMax();
      delay(10);
    };

    String newSSID = currentSSID;
    String newPassword = currentPassword;
    String newPushbulletAccessToken = currentPBAccessToken;

    wifiSSID = newSSID;
    wifiPassword = newPassword;

    // set the current SSID and Passwordto null
    // these will be reloaded if the Wi-Fi Connection can be established
    currentSSID = "";
    currentPassword = "";

    WiFi.softAPdisconnect(true);
    WiFi.disconnect(true, true);

    while (WiFi.status() == WL_CONNECTED)
      delay(100);

    Serial.println("user has updated Wi-Fi info in browser");

    // confirm Wi-Fi credentials; attempt to connect ten times

    bool WifiSetupSucceeded = false;

    if (setupWiFiWithExistingCredentials(10))
    {

      WifiSetupSucceeded = true;

      // update EEPROM
      //
      // while the stored SSID and password are not needed at this point as they are about to be replaced
      // the stored message is required as it will need to be stored with the new SSID, Password and Pushbullet Access Token.
      // Accordingly, first read the EEPROM which will get the stored SSID, Password, Pushbullet Access Token and Message
      // and then second write the new SSID, new Password, new Pushbullet Access Token and perviously stored message
      //
      loadDataFromEEPROM();

      const char fieldSeparator = 254;

      String EEPROMData = newSSID + fieldSeparator + newPassword + fieldSeparator + pushbulletAccessToken + fieldSeparator + currentMessage + fieldSeparator + currentMessageTimeAndDate;

      writeEEPROMString(0, EEPROMData);

      displayMessageOnMax("Wi-Fi name and password confirmed!", true);
      displayMessageOnMax("Automatically restarting ... ", true);
      ESP.restart();
    };

    displayMessageOnMax("*** Wi-Fi access failed! ***", true);
    displayMessageOnMax("Automatically restarting ... ", true);
    ESP.restart();
  };
};

bool stringsAreMatchRegardlessOfCase(String s1, String s2)
{

  s1.toUpperCase();
  s2.toUpperCase();
  return (s1 == s2);
}

String getUpTime()
{

  unsigned long ms = millis();

  const int oneSecond = 1000;
  const int oneMinute = oneSecond * 60;
  const int oneHour = oneMinute * 60;
  const int oneDay = oneHour * 24;

  int numberOfDays = ms / oneDay;
  ms = ms - numberOfDays * oneDay;

  int numberOfHours = ms / oneHour;
  ms = ms - numberOfHours * oneHour;

  int numberOfMinutes = ms / oneMinute;
  ms = ms - numberOfMinutes * oneMinute;

  int numberOfSeconds = ms / oneSecond;

  String returnValue = "";

  if (numberOfDays == 1)
  {
    returnValue.concat("1 day, ");
  }
  else
  {
    if (numberOfDays > 1)
    {
      returnValue.concat(String(numberOfDays));
      returnValue.concat(" days, ");
    };
  };

  if (numberOfHours == 1)
  {
    returnValue.concat("1 hour, ");
  }
  else
  {
    if (numberOfHours > 1)
    {
      returnValue.concat(String(numberOfHours));
      returnValue.concat(" hours, ");
    };
  };

  if (numberOfMinutes == 1)
  {
    returnValue.concat("1 minute, ");
  }
  else
  {
    if (numberOfMinutes > 1)
    {
      returnValue.concat(String(numberOfMinutes));
      returnValue.concat(" minutes, ");
    };
  };

  if (numberOfSeconds == 1)
  {
    returnValue.concat("1 second.");
  }
  else
  {
    if (numberOfSeconds > 1)
    {
      returnValue.concat(String(numberOfSeconds));
      returnValue.concat(" seconds.");
    };
  };

  // tweak the return value so that it is more English like
  returnValue.trim();
  if (returnValue.endsWith(","))
  {
    returnValue = returnValue.substring(0, returnValue.length() - 1);
    returnValue.concat(".");
  };

  int pos = returnValue.lastIndexOf(",");
  if (pos > -1)
  {
    String firstSection = returnValue.substring(0, pos);
    String secondSection = returnValue.substring(pos + 1);
    returnValue = firstSection + " and" + secondSection;
  };

  return returnValue;
}

// ************************************************************************************************************************************************************
void setupWebServer()
{

  server.onNotFound([](AsyncWebServerRequest *request)
                    {
    Serial.println("Not found: " + String(request->url()));
    request->redirect("/"); });

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    // present htmlDataEntryWindow window

    String html = String(htmlHeader);

    html.concat(htmlDataEntryWindow);

    // special handling for when a quote is used in the message
    const String quote = String('"');
    const String quoteCode = String("&quot;");

    String htmlFriendlyCurrentMessage = currentMessage;
    htmlFriendlyCurrentMessage.trim();
    htmlFriendlyCurrentMessage.replace(quote, quoteCode);

    html.replace("$MESSAGE$", htmlFriendlyCurrentMessage);

    if (TIME_FORMAT_OPTION > 3)
      html.replace("$time$", "time and date");
    else
      html.replace("$time$", "time");

    if (messageboardPassword.length() == 0) {
      const String quote = String('"');
      String original = "name=" + quote + "passwordneeded" + quote;
      String revised_ = "name=" + quote + "passwordneeded" + quote + " hidden=" + quote + "hidden" + quote;
      html.replace(original, revised_);
    };

    if (htmlFriendlyFeedback.length() == 0) {
      String original = "name=" + quote + "errormessages" + quote;
      String revised_ = "name=" + quote + "errormessages" + quote + " hidden=" + quote + "hidden" + quote;
      html.replace(original, revised_);
    } else {
      html.replace("$color$", htmlFriendlyFeedbackColor);
      html.replace("$errors$", htmlFriendlyFeedback);
    };

    html.concat(htmlFooter);
    request->send(200, "text/html", html); });

  server.on("/htmlConfirmCleared", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    // present htmlConfirmCleared window

    String html = String(htmlHeader);

    html.concat(htmlConfirmCleared);

    html.concat(htmlFooter);
    request->send(200, "text/html", html);
    clearMessageRequested = true; });

  server.on("/htmlConfirmMemoryCleared", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    // present htmlConfirmCleared window

    String html = String(htmlHeader);

    html.concat(htmlConfirmMemoryCleared);

    html.concat(htmlFooter);
    request->send(200, "text/html", html);
    EEPROMClearRequested = true;
    restartRequested = true; });

  server.on("/htmlConfirmRestart", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    // present htmlConfirmCleared window

    String html = String(htmlHeader);

    html.concat(htmlConfirmRestart);

    html.concat(htmlFooter);
    request->send(200, "text/html", html);
    restartRequested = true; });

  server.on("/htmlConfirmUptime", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    // present htmlConfirmUptime window

    String html = String(htmlHeader);

    html.concat(htmlConfirmUptime);
    html.replace("$uptime$", getUpTime());

    html.concat(htmlFooter);
    request->send(200, "text/html", html); });

  server.on("/htmlConfirmUpdate", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    // present htmlConfirmUpdate window

    const String quote = String('"');
    const String quoteCode = String("&quot;");

    String html = String(htmlHeader);
    html.concat(htmlConfirmUpdate);

    String htmlFriendlyCurrentMessage = currentMessage;
    htmlFriendlyCurrentMessage.trim();
    htmlFriendlyCurrentMessage.replace(quote, quoteCode);

    htmlFriendlyCurrentMessage.concat(currentMessageTimeAndDate);

    html.replace("$MESSAGE$", htmlFriendlyCurrentMessage);

    html.concat(htmlFooter);
    request->send(200, "text/html", html); });

  server.on("/pushbullet", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    // present htmlGetPushbulletCredentials window

    const String quote = String('"');

    String html = String(htmlHeader);

    html.concat(htmlGetPushbulletCredentials);

    html.replace("$99$", String(lengthOfAValidPushbulletAccessToken));

    if (pushbulletAccessToken.length() == lengthOfAValidPushbulletAccessToken)
      html.replace("$pbaccesstoken$", String(pushbulletAccessToken));
    else
      html.replace("$pbaccesstoken$", "");

    html.replace("$color$", htmlFriendlyFeedbackColor);

    html.replace("$errors$", htmlFriendlyFeedback);

    html.concat(htmlFooter);
    request->send(200, "text/html", html); });

  server.on("/clearpushbulletaccesstoken", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    // present confirmPushbulletAccessToken window

    String html = String(htmlHeader);

    html.concat(htmlClearPushbulletAccessToken);

    html.concat(htmlFooter);
    restartRequested = true;

    request->send(200, "text/html", html); });

  server.on("/confirmpushbulletaccesstoken", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    // present confirmPushbulletAccessToken window

    String html = String(htmlHeader);

    html.concat(htmlConfirmPushbulletAccessToken);

    html.concat(htmlFooter);
    restartRequested = true;

    request->send(200, "text/html", html); });

  server.on("/password", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    // present htmlGetMessageboardPasswordCredentials window

    const String quote = String('"');

    String html = String(htmlHeader);

    html.concat(htmlGetMessageboardPasswordCredentials);

    html.replace("$old$", "");
    html.replace("$new1$", "");
    html.replace("$new2$", "");

    html.replace("$color$", htmlFriendlyFeedbackColor);

    html.replace("$errors$", htmlFriendlyFeedback);

    html.concat(htmlFooter);
    request->send(200, "text/html", html); });

  server.on("/htmlMessageboardPasswordConfirmed", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    // present htmlMessageboardPasswordConfirmed window

    String html = String(htmlHeader);

    html.concat(htmlMessageboardPasswordConfirmed);

    html.concat(htmlFooter);
    request->send(200, "text/html", html); });

  server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    //

    htmlFriendlyFeedback = "";
    htmlFriendlyFeedbackColor = "black";
    htmlFriendlyCurrentMessage = "";

    String html = String(htmlHeader);

    if (request->hasParam(PARAM_INPUT_MESSAGE)) {

      String inputMessage = request->getParam(PARAM_INPUT_MESSAGE)->value();
      inputMessage.trim();

      Serial.print("Input message: ");
      Serial.println(inputMessage);

      if (stringsAreMatchRegardlessOfCase(inputMessage, showUptime)) {
        request->redirect("/htmlConfirmUptime");
        return;
      };

      if (stringsAreMatchRegardlessOfCase(inputMessage, restartCommand)) {
        request->redirect("/htmlConfirmRestart");
        return;
      };

      if (stringsAreMatchRegardlessOfCase(inputMessage, clearTheESP32sMemory)) {
        request->redirect("/htmlConfirmMemoryCleared");
        return;
      };

      if (request->hasParam(PARAM_INPUT_DATA_ENTRY_PASSWORD)) {

        String inputPassword = request->getParam(PARAM_INPUT_DATA_ENTRY_PASSWORD)->value();
        inputPassword.trim();

        if (inputPassword != messageboardPassword) {
          htmlFriendlyFeedback = "The update password is incorrect";
          htmlFriendlyFeedbackColor = "red";
          request->redirect("/");
          return;
        };
      };

      if ((inputMessage.length() == 0) || (stringsAreMatchRegardlessOfCase(inputMessage, clearTheMessageBoardCommand)) || (request->hasParam(PARAM_INPUT_CLEAR))) {
        request->redirect("/htmlConfirmCleared");
        return;
      }

      if (request->hasParam(PARAM_INPUT_INCLUDETIME)) {

        String inputTimeCheckboxString = "";
        inputTimeCheckboxString = request->getParam(PARAM_INPUT_INCLUDETIME)->value();

        if (inputTimeCheckboxString == "on") {
          const char fieldSeparator = 254;
          inputMessage.concat(fieldSeparator);
          inputMessage.concat(" - ");
          inputMessage.concat(getFormatTimeandDate());
        };
      };

      writeMessageToEEPROM(inputMessage);  // save input message to eeprom and make it the current message

      request->redirect("/htmlConfirmUpdate");

      resetMax7219(true);
      displayTheCurrentMessage();
      return;
    };

    if (request->hasParam(PARAM_MESSAGE_CLEAR) || request->hasParam(PARAM_MESSAGE_CONFIRM) || request->hasParam(PARAM_UPTIME_OK)) {
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



    if (request->hasParam(PARAM_INPUT_MESSAGEBOARD_PASSWORD_UPDATE)) {
      String inputOldPassword = request->getParam(PARAM_INPUT_MESSAGEBOARD_PASSWORD_OLD)->value();
      String inputNewPassword1 = request->getParam(PARAM_INPUT_MESSAGEBOARD_PASSWORD_NEW_1)->value();
      String inputNewPassword2 = request->getParam(PARAM_INPUT_MESSAGEBOARD_PASSWORD_NEW_2)->value();

      inputNewPassword1.trim();
      inputNewPassword2.trim();

      if (inputOldPassword != messageboardPassword) {
        htmlFriendlyFeedbackColor = "red";
        htmlFriendlyFeedback = "incorrect current password";
        request->redirect("/password");
        return;
      };

      if (inputNewPassword1.length() > 0) {
        if (inputNewPassword1 != inputNewPassword2) {
          htmlFriendlyFeedbackColor = "red";
          htmlFriendlyFeedback = "new password and confirmed new password don't match";
          request->redirect("/password");
          return;
        };
      };

      if (inputOldPassword == inputNewPassword1) {
        htmlFriendlyFeedbackColor = "red";
        htmlFriendlyFeedback = "old and new passwords are the same";
        request->redirect("/password");
        return;
      };

      messageboardPassword = inputNewPassword1;
      writeMessageBoardPasswordToEEPROM(messageboardPassword);
      request->redirect("/htmlMessageboardPasswordConfirmed");
      return;
    };

    if (request->hasParam(PARAM_INPUT_MESSAGEBOARD_PASSWORD_UPDATE_CONFIRM)) {
      request->redirect("/");
      return;
    }; });

  events.onConnect([](AsyncEventSourceClient *client)
                   {
    // client->send("hello!", NULL, millis(), 1000);
    Serial.println("Webserver connected"); });

  server.addHandler(&events);

  server.begin();

  // request the user enter the message on the webpage
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    String html = String(htmlHeader);
    html.concat(htmlDataEntryWindow);
    html.replace("$MESSAGE$", currentMessage);
    html.concat(htmlFooter);
    request->send(200, "text/html", html); });
}

void setupOTAUpdate()
{
  // Port defaults to 3232
  // ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
  ArduinoOTA.setHostname(SECRET_OTA_HOSTNAME);

  // No authentication by default
  ArduinoOTA.setPassword(SECRET_OTA_PASSWORD);

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA
      .onStart([]()
               {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else  // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type); })
      .onEnd([]()
             { Serial.println("\nEnd"); })
      .onProgress([](unsigned int progress, unsigned int total)
                  { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); })
      .onError([](ota_error_t error)
               {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed"); });

  ArduinoOTA.begin();
}

void setupFinalDisplays()
{
  displayMessageOnMax("Setup complete", true);

  displayMessageBoardsAddressAsNeeded();

  displayTheCurrentMessage();
}

void checkWifiConnection()
{
  // Enhanced WiFi connection checking with non-blocking reconnection logic

  static unsigned long lastWiFiCheck = 0;
  static unsigned long lastReconnectAttempt = 0;
  static unsigned long reconnectStartTime = 0;
  static int reconnectAttempts = 0;
  static bool reconnectInProgress = false;

  const unsigned long wifiCheckInterval = 5000;  // Check every 5 seconds
  const unsigned long reconnectInterval = 30000; // Wait 30 seconds between reconnect attempts
  const unsigned long reconnectTimeout = 20000;  // Wait 20 seconds for each reconnect attempt
  const unsigned long tenMinutes = 600000;       // 10 minutes before restart
  const int maxReconnectAttempts = 5;            // Max attempts before restart

  // Only check WiFi status every 5 seconds to avoid constant checking
  if (millis() - lastWiFiCheck < wifiCheckInterval)
  {
    return;
  }
  lastWiFiCheck = millis();

  if (WiFi.status() == WL_CONNECTED)
  {
    lastTimeWiFiWasConnected = millis();
    reconnectAttempts = 0;       // Reset attempt counter on successful connection
    reconnectInProgress = false; // Clear reconnect flag
  }
  else
  {
    // If we're currently attempting to reconnect, check if it timed out
    if (reconnectInProgress)
    {
      if (millis() - reconnectStartTime > reconnectTimeout)
      {
        Serial.println("WiFi reconnection attempt timed out");
        reconnectInProgress = false;
      }
      else
      {
        // Still waiting for current reconnect attempt - don't start another
        return;
      }
    }

    Serial.println("WiFi connection lost, status: " + String(WiFi.status()));

    // Don't attempt reconnect too frequently
    if (millis() - lastReconnectAttempt < reconnectInterval)
    {
      return;
    }

    lastReconnectAttempt = millis();
    reconnectStartTime = millis();
    reconnectAttempts++;
    reconnectInProgress = true;

    Serial.println("Attempting WiFi reconnection, attempt: " + String(reconnectAttempts));

    // Use non-blocking reconnection approach
    WiFi.disconnect();
    delay(100); // Small delay for disconnect to complete
    WiFi.mode(WIFI_STA);
    delay(100); // Small delay for mode change
    WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());

    // Check if we should restart after prolonged disconnection
    if ((millis() - lastTimeWiFiWasConnected) > tenMinutes || reconnectAttempts > maxReconnectAttempts)
    {
      Serial.println("WiFi connection lost for too long or too many failed attempts, restarting...");
      displayMessageOnMax("WiFi connection lost - restarting", true);
      restartRequested = true;
    }
  }
}

void displayTheCurrentMessage()
{

  if (currentMessage.length() > 0)
  {

    bool showOnlyOnce = currentMessage.startsWith("[1]");

    if (showOnlyOnce)
    {
      currentMessage.remove(0, 3);
      displayTheCurrentMessageOnMax(true);
      currentMessage = "[1]" + currentMessage;
    }
    else
      displayTheCurrentMessageOnMax(false);
  };
};

void clearDisplayAsNeeded()
{
  if (clearMessageRequested)
  {

    Serial.println("Clearing the message board");
    writeMessageToEEPROM(" ");
    displayMessageOnMax(" ", true);

    clearMessageRequested = false;
    messageBoardsAddressRequested = false; // if message was cleared ignore request to display the message board's address
  };
}

void displayMessageBoardsAddressAsNeeded()
{
  if (messageBoardsAddressRequested)
  {

    String message = "To update the scrolling message please browse to http://";
    message.concat(WiFi.localIP().toString());

    displayMessageOnMax(message, true);

    messageBoardsAddressRequested = false;

    // if the message board was previously displaying a message, resume it now
    displayTheCurrentMessage();
  };
};

String getFormatTimeandDate()
{

  struct timeval tvTime;

  gettimeofday(&tvTime, NULL);

  int iTotal_seconds = tvTime.tv_sec;
  struct tm *ptm = localtime((const time_t *)&iTotal_seconds);

  int iYear = ptm->tm_year - 100;
  int iMonth = ptm->tm_mon + 1;
  int iDay = ptm->tm_mday;
  int iHour = ptm->tm_hour;
  int iMinute = ptm->tm_min;
  int iSecond = ptm->tm_sec;
  int iTenthOfASecond = tvTime.tv_usec / 100000;

  String indicatorForAmPm = "";

  bool FormatOptionIncludesTime = false;
  bool FormatOptionIncludesDate = false;

  if (SHOW_TIME_IN_12_HOUR_FORMAT)
  {

    if (iHour < 12)
      indicatorForAmPm = " a.m.";
    else
      indicatorForAmPm = " p.m.";

    if (iHour > 12)
      iHour -= 12;
  };

  char buffer1[10];
  char buffer2[10];

  switch (TIME_FORMAT_OPTION)
  {
    // 1: HH:MM
    // 2: HH:MM:SS
    // 3: HH:MM:SS.T
    // 4: MM/DD
    // 4: YY/MM/DD
    // 6: MM/DD/YY
    // 7: HH:MM & MM/DD
    // 8: HH:MM & YY/MM/DD
    // 9: HH:MM & MM/DD/YY
    // 10: HH:MM:SS & YY/MM/DD
    // 11: HH:MM:SS & MM/DD/YY

  case 1:
  {
    if (iSecond & 0x01)
    {
      sprintf(buffer1, "%02d %02d", iHour, iMinute);
    }
    else
    {
      sprintf(buffer1, "%02d:%02d", iHour, iMinute);
    };
    FormatOptionIncludesTime = true;
    break;
  }

  case 2:
  {
    sprintf(buffer1, "%02d:%02d:%02d", iHour, iMinute, iSecond);
    FormatOptionIncludesTime = true;
    break;
  }

  case 3:
  {
    sprintf(buffer1, "%02d:%02d:%02d.%01d", iHour, iMinute, iSecond, iTenthOfASecond);
    FormatOptionIncludesTime = true;
    break;
  }

  case 4:
  {
    sprintf(buffer1, "%02d/%02d", iMonth, iDay);
    FormatOptionIncludesDate = true;
    break;
  }

  case 5:
  {
    sprintf(buffer1, "%02d/%02d/%02d", iYear, iMonth, iDay);
    FormatOptionIncludesDate = true;
    break;
  }

  case 6:
  {
    sprintf(buffer1, "%02d/%02d/%02d", iMonth, iDay, iYear);
    FormatOptionIncludesDate = true;
    break;
  }

  case 7:
  {
    if (iSecond & 0x01)
    {
      sprintf(buffer1, "%02d %02d", iHour, iMinute);
    }
    else
    {
      sprintf(buffer1, "%02d:%02d", iHour, iMinute);
    };
    sprintf(buffer2, "%02d/%02d", iMonth, iDay);
    FormatOptionIncludesTime = true;
    FormatOptionIncludesDate = true;
    break;
  }

  case 8:
  {
    if (iSecond & 0x01)
    {
      sprintf(buffer1, "%02d %02d", iHour, iMinute);
    }
    else
    {
      sprintf(buffer1, "%02d:%02d", iHour, iMinute);
    };
    sprintf(buffer2, "%02d/%02d/%02d", iYear, iMonth, iDay);
    FormatOptionIncludesTime = true;
    FormatOptionIncludesDate = true;
    break;
  }

  case 9:
  {
    if (iSecond & 0x01)
    {
      sprintf(buffer1, "%02d %02d", iHour, iMinute);
    }
    else
    {
      sprintf(buffer1, "%02d:%02d", iHour, iMinute);
    };
    sprintf(buffer2, "%02d/%02d/%02d", iMonth, iDay, iYear);
    FormatOptionIncludesTime = true;
    FormatOptionIncludesDate = true;
    break;
  }

  case 10:
  {
    sprintf(buffer1, "%02d:%02d:%02d", iHour, iMinute, iSecond);
    sprintf(buffer2, "%02d/%02d/%02d", iYear, iMonth, iDay);
    FormatOptionIncludesTime = true;
    FormatOptionIncludesDate = true;
    break;
  }

  case 11:
  {
    sprintf(buffer1, "%02d:%02d:%02d", iHour, iMinute, iSecond);
    sprintf(buffer2, "%02d/%02d/%02d", iMonth, iDay, iYear);
    FormatOptionIncludesTime = true;
    FormatOptionIncludesDate = true;
    break;
  }
  }

  if (FormatOptionIncludesTime)
  {

    if (SHOW_TIME_IN_12_HOUR_FORMAT)
    {

      if ((buffer1[0] == '0') & (buffer1[1] == '0'))
      {
        buffer1[0] = '1';
        buffer1[1] = '2';
      };

      if (buffer1[0] == '0')
        buffer1[0] = ' ';
    };
  };

  String TimeString = String(buffer1) + indicatorForAmPm;
  String DateString = String(buffer2);

  String returnResult = "";
  if (FormatOptionIncludesTime)
  {
    returnResult.concat(TimeString);
    returnResult.trim();
  };

  if (FormatOptionIncludesTime && FormatOptionIncludesDate)
    returnResult.concat(" ");

  if (FormatOptionIncludesDate)
  {
    returnResult.concat(DateString);
    returnResult.trim();
  };

  return returnResult;
};

void restartAndClearAsNeeded()
{

  if (EEPROMClearRequested)
    clearTheEEPROM();

  if (restartRequested)
  {
    displayMessageOnMax("Restarting ...", true);
    ESP.restart();
  };
}

void setup()
{

  setupSerial();
  setupMax7219();
  setupButtons();
  setupFromEEPROM();
  setupOpeningDisplays();

  checkButton();

  if (manualResetRequested || (wifiSSID.length() == 0))
    setupWifiWithNewCredentials();
  else
    setupWiFiWithExistingCredentials(0);

  setupTime();
  setupPushbullet();
  setupWebServer();
  setupOTAUpdate();
  setupFinalDisplays();

  setupComplete = true;
}

void loop()
{
  static unsigned long lastMaintenanceTime = 0;
  const unsigned long maintenanceInterval = 1000; // Run maintenance tasks every second

  // Always handle immediate user interactions and display
  checkButton();
  clearDisplayAsNeeded();
  displayMessageBoardsAddressAsNeeded();
  fullyScrollMessageOnMax();

  // Run maintenance tasks less frequently to reduce blocking
  if (millis() - lastMaintenanceTime > maintenanceInterval)
  {
    checkWifiConnection(); // Now non-blocking
    checkPushbulletConnection();
    keepPushbulletAccountAlive(false);
    refreshTimeOnceAWeek();
    restartAndClearAsNeeded();

    lastMaintenanceTime = millis();
  }

  // Handle network tasks frequently to prevent timeouts
  keepPushbulletSessionAlive();
  ArduinoOTA.handle();
};