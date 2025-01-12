// this is a consolidation of key user settings

#define NUMBER_OF_MAX7219_MODULES                                     13                                       // Number of connected Max7219 modules

#define EXTERNAL_BUTTON_PIN                                           19                                       // External button pin used to clear message and reset the wifi credentials

#define CLK_PIN                                                       18                                       // ESP32 CLK pin
#define CS_PIN                                                        5                                        // ESP32 CS pin
#define DATA_PIN                                                      23                                       // ESP32 Data pin

#define ACCESS_POINT_SSID                                             "ScrollingMessageBoard"                  // Access Point network name

#define PUSHBULLET_TITLE_TO_REACT_TO                                  "Leave a message"
                                                                                                               // Special commands:
#define CLEAR_ESP32S_MEMORY_COMMAND                                   "Clear the memory of the message board"  // clear all non-volatile memory (i.e. current wifi SSID, wifi Password, Pushbullet Access Token, and the current message)
#define CLEAR_THE_MESSAGE_BOARD_COMMAND                               "Clear the message board"                // clear the current message 
#define RESTART_COMMAND                                               "Restart the message board"              // restart the message board
#define SHOW_UPTIME_COMMAND                                           "Show uptime"                           // show uptime in the web browser

#define MAXIMUM_SIZE_OF_NON_VOLATILE_MEMORY                            4096                                    // 4096 (4K) is the maximum supported

#define SEND_PUSHBULLET_KEEP_ALIVE_REQUEST                             true                                    // normally set to true; set to false when testing
#define REBOOT_AFTER_THIS_MANY_MINUTES_WITHOUT_HEARING_FROM_PUSHBULLET 15


#define MY_TIME_ZONE                                                   "EST5EDT,M3.2.0,M11.1.0"                // Time Zone for America/Toronto   
                                                                                                               // Supported Timezones are listed here: https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
                                                                                                               //
#define PRIMARY_TIME_SERVER                                            "time.nrc.ca"                           // primary ntp time server
#define SECONDARY_TIME_SERVER                                          "ca.pool.ntp.org"                       // secondary ntp time server
#define TERTIARY_TIME_SERVER                                           "north-america.pool.ntp.org"            // tertiary ntp time server 
                                                                                                               // alternative ntp time servers/time server pools; 
                                                                                                               // best to use a more local server / server pool as the primary and spread out from there
                                                                                                               //    "time.nrc.ca"                  time server in Ottawa, On, Canada
                                                                                                               //    "ca.pool.ntp.org"              time server pool for Canada                                                                                                                                                                                                                                            
                                                                                                               //    "asia.pool.ntp.org"               
                                                                                                               //    "europe.pool.ntp.org"
                                                                                                               //    "north-america.pool.ntp.org" 
                                                                                                               //    "oceania.pool.ntp.org"
                                                                                                               //    "south-america.pool.ntp.org" 
                                                                                                               //    "pool.ntp.org"                  world wide time server pool
                                                                                                              
                                                                                                                
									                             											
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
																											   

#define SECRET_OTA_HOSTNAME "ESP32MsgBoard"                                                                    // Over The Air (OTA) update host name
#define SECRET_OTA_PASSWORD "MsgBoard"                                                                         // OTA password