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

#define TIME_SERVER                                                    "pool.ntp.org"                          // ntp time server
#define TIME_ZONE                                                      -5                                      // hour offset from GMT
#define DAYLIGHT_SAVINGS_TIME                                          1                                       // hour offset for Daylight Savings time (0, .5, 1)
#define SHOW_TIME_IN_12_HOUR_FORMAT                                    true                                    // if true show time in 12 hour format, if false show time in 24 hour format
#define TIME_FORMAT_OPTION                                             2                                       // Options are:
                                                                                                               //   1: HH:MM
                                                                                                               //   2: HH:MM:SS
                                                                                                               //   3: HH:MM:SS.T
                                                                                                               //   4: MM/DD
                                                                                                               //   5: YY/MM/DD
                                                                                                               //   6: MM/DD/YY
                                                                                                               //   7: HH:MM MM/DD
                                                                                                               //   8: HH:MM YY/MM/DD
                                                                                                               //   9: HH:MM MM/DD/YY
                                                                                                               //   10: HH:MM:SS YY/MM/DD
                                                                                                               //   11: HH:MM:SS & MM/DD/YY



#define SECRET_OTA_HostName                                            "ESP32MsgBoard"                         // Over The Air (OTA) update host name
#define SECRET_OTA_PASSWORD                                            "MsgBoard"                              // OTA password