set PushbulletAPI="xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"

curl -u %PushbulletAPI%:ROBSOFFICE https://api.pushbullet.com/v2/pushes -d type=note -d title="Leave a message" -d body="Restart the message board"


