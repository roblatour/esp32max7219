// the following options can also be changed via the program's settings window:

#define SHOW_TIME_IN_TWELVE_HOUR_FORMAT      true                           // if set to true for 24 hour time format, set to false for 12 hour time format

#define DEFAULT_DISPLAY_OPTION               2                              // Display options:
                                                                            // 1: HH:MM
                                                                            // 2: HH:MM:SS
                                                                            // 3: HH:MM:SS.T
                                                                            // 4: MM/DD
                                                                            // 5: YY/MM/DD
                                                                            // 6: MM/DD/YY
                                                                            // 7: HH:MM & MM/DD
                                                                            // 8: HH:MM & YY/MM/DD
                                                                            // 9: HH:MM & MM/DD/YY
                                                                            // 10: HH:MM:SS & YY/MM/DD
                                                                            // 11: HH:MM:SS & MM/DD/YY

#define USE_TIME_SENSITIVE_BACKGROUND        true                           // change the background colour every second

// the following options cannot be changed via the program's settings window

#define WIFI_CONNECTING_STATUS_COLOUR        TFT_GREEN                      // the text colour of the display during WIFI connection process
 

#define TIME_SENSITIVE_COLOURIZATION_METHOD  1                              // 1: start at a random colour and rotate through all feasable colours
                                                                            // 2: use a unique colour based on time

#define BACKGROUND_COLOUR_DEFAULT            TFT_RED                        // if USE_TIME_SENSITIVE_BACKGROUND is false, this is used as default background colour

#define TIME_ZONE                             -5                            // hour offset from GMT
#define DAYLIGHT_SAVINGS_TIME                  1                            // hour offset for Daylight Savings time (0, .5, 1)

#define PRIMARY_TIME_SERVER                  "time.nrc.ca"                  // primary ntp time server
#define SECONDARY_TIME_SERVER                "ca.pool.ntp.org"              // secondary ntp time server
#define TERTIARY_TIME_SERVER                 "north-america.pool.ntp.or"    // tertiary ntp time server 
                                                                            // alternative ntp time servers / server pools:                                                                          
                                                                            //    "time.nrc.ca"                        // Ottawa, Ontario, Canada
                                                                            //    "ca.pool.ntp.org"                    // Canada
                                                                            //    "asia.pool.ntp.org"
                                                                            //    "europe.pool.ntp.org"
                                                                            //    "north-america.pool.ntp.org" 
                                                                            //    "oceania.pool.ntp.org"
                                                                            //    "south-america.pool.ntp.org" 
                                                                            //    "pool.ntp.org"                       // World wide      

#define SERIAL_MONITOR_SPEED                  115200                        // serial monitor speed

#define DEBUG_IS_ON                           true                          // show debug info to Serial monitor windows

#define ALLOW_ACCESS_TO_NETWORK               false                         // used for testing, set to false to disallow access to your Wifi network

#define ALLOW_ACCESS_TO_THE_TIME_SERVER       true                          // used for testing, set to false to disallow access to the time server


