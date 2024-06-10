# esp32max7219
Voice controlled scrolling message board
by Rob Latour https://rlatour.com

Notes:

   Before compiling and uploading to your ESP32, please review the key_settings.h file and make updates as you may like.

   The user's WiFi Network name (SSID), Wifi password, and Pushbullet Access Token are not stored in this sketch nor the key_settings.h file.
   Rather they are stored in non-volatile memory after being entered by the user via a web browser (see below).

To operate:

  1. Press and hold the external button for more than 10 seconds when the esp32 is powered on to trigger the process to reset your Wifi Credentials
     This should be done for initial setup, but may be done anytime afterwards if your Wifi Network name (SSID) or Wifi password change

     1.1 The message board will prompt you to take three steps (which you can do this from your computer or cell phone):
         Step 1:  connect to Wifi network ScrollingMessageBoard
	     Step 2:  open your browser in Incognito mode 
         Step 3:  browse to http://123.123.123.123
         Step 4:  enter your Wifi network information, and click 'OK'

  2. Once connected to your Wifi network:

     2.1 Press and hold the external button for 1 second to have the scrolling message board display the web address at which you can change/clear the text on the scrolling message board
         for example:  http://192.168.1.100

     2.3 (Optional) To enter a password for updating the message on the Scrolling Message Board, browse to the address identified in step 2.1 with "/password" (without the quotes) added after it,
         for example:  http://192.168.1.100/password
         To start, the current password is blank, that is to say nothing is entered in the current password box
         If the current password is changed from being blank to something else, it can later be changed back to being blank by leaving the new and confirmed new passwords as blank when updating the password
         If the current password is not blank, the user will need to enter it to to change the text of the scrolling message board via the web interface (see 2.4 below)

     2.3 (Optional) To enter your Pushbullet Access Token, browse to the address identified in step 2.1 with "/pushbullet" (without the quotes) added after it,
         for example:  http://192.168.1.100/pushbullet
         For more information on Pushbullet setup please see: https://hackaday.io/project/170281-voice-controlled-scrolling-message-board

     2.4 To change the text on the scrolling message board

         Method 1: browse to the address identified in step 2.1 above, enter the new text, and click 'OK'

         Method 2: send a Pushbullet push with the text you would like displayed

         For both methods, here are some special messages (without the quotes) you can use to have special functions performed, 
		 these are:

                   "Clear the message board"                  (clears the message board's display)
                   "Restart the message board"                (restarts the message board)
                   "Clear the memory of the message board"    (clears the non-volatile memory in which the Wifi SSID, 
				                                              Wifi Password, Pushbullet Access Token, and current
 															  message values are stored)
															  
         For Method 1, an additional method exists, it is:
                   "Show uptime"                              (this will show in the browser the amount of time since the Scrolling Message Board was last started)


         Notes: 

            the above special commands are not case sensitive; i.e.  "Show uptime" (without the quotes) and "show uptime" (without the quotes) will work equally as well

            the maximum text message length is about 4,000 characters (which should be more that enough for most use cases)

     2.5 To clear the text on the message board:

         method 1:  Press and hold the external button for more than 5 seconds

         method 2:  Click the clear button on the web page identified in 2.1 above

         method 3:  as described above, send a Pushbullet push with the message "Clear the memory of the message board" (without the quotes)


 Final Notes:

     If you are using Pushbullet, please note the two certificates stored in the pushbulletCertificates.h file
	 both have expiry dates (as shown in the file)

     I will endeavor to update this file on Github in the future but there is more information on how you can
	 do this yourself in the pushbulletCertificates.h file
	 
     When the certificates expire they will need to be updated and this sketch recompiled and reloaded

     I am already thinking of ways to make this more seamless in the future ... perhaps a future update?

     Donations welcome at https://rlatour.com/


For more information please see:
https://hackaday.io/project/170281-voice-controlled-scrolling-message-board

## Support

[<img alt="buy me  a coffee" width="200px" src="https://cdn.buymeacoffee.com/buttons/v2/default-blue.png" />](https://www.buymeacoffee.com/roblatour)
