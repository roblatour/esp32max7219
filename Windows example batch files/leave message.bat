echo off

cls
 
set /p choice= "What would you like to post to the message board? "

set PushbulletAPI="xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"

curl -u %PushbulletAPI%:ROBSOFFICE https://api.pushbullet.com/v2/pushes -d type=note -d title="Leave a message" -d body="%choice%"


 

