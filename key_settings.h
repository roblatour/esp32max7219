// this is a consolidation of key user settings

#define NUMBER_OF_MAX7219_MODULES                                     13                                       // Number of connected Max7219 modules

#define EXTERNAL_BUTTON_PIN                                           19                                       // External button pin used to clear message and reset the wifi credentials

#define CLK_PIN                                                       18                                       // ESP32 CLK pin
#define CS_PIN                                                        5                                        // ESP32 CS pin
#define DATA_PIN                                                      23                                       // ESP32 Data pin

#define ACCESS_POINT_SSID                                             "ScrollingMessageBoard"                  // Access Point network name

#define PUSHBULLET_TITLE_TO_REACT_TO                                  "Leave a message"
#define CLEAR_THE_MESSAGE_BOARD_COMMAND                               "Clear the message board"
#define RESTART_COMMAND                                               "Restart the message board"
#define CLEAR_ESP32S_MEMORY_COMMAND                                   "Clear the memory of the message board"  // clear all non-volatile memory (i.e. current wifi SSID, wifi Password, Pushbullet Access Token, and the current message)

#define MAXIMUM_SIZE_OF_NON_VOLATILE_MEMORY                            4096                                    // 4096 (4K) is the maximum supported

#define SEND_PUSHBULLET_KEEP_ALIVE_REQUEST                             true                                    // normally set to true; set to false when testing
#define REBOOT_AFTER_THIS_MANY_MINUTES_WITHOUT_HEARING_FROM_PUSHBULLET 15

#define SECRET_OTA_HostName                                            "ESP32MsgBoard"                         // Over The Air (OTA) update host name
#define SECRET_OTA_PASSWORD                                            "MsgBoard"                              // OTA password