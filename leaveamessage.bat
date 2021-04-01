echo off

cls
 
set /p choice= "What would you like to post to the message board? "

for /f "delims=" %%a in ('wmic OS Get localdatetime ^| find "."') do set dt=%%a

set year=%dt:~0,4%

set month=%dt:~4,2%
if %month%==01 set monthname=January
if %month%==02 set monthname=Febuary
if %month%==03 set monthname=March
if %month%==04 set monthname=April
if %month%==05 set monthname=May
if %month%==06 set monthname=June
if %month%==07 set monthname=July
if %month%==08 set monthname=August
if %month%==09 set monthname=September
if %month%==10 set monthname=October
if %month%==11 set monthname=November
if %month%==12 set monthname=December

set day=%dt:~6,2%
IF %day% LEQ 9 (SET day=%day:~1,1%)

SET hour=%dt:~8,2%
IF %hour%=="00" (SET hour=0) ELSE (IF %hour% LEQ 9 (SET hour=%hour:~1,1%))
IF %hour% GTR 11 (SET "ap=pm") ELSE (SET "ap=am")
IF %hour% GTR 12 (SET /A "hour-=12")
IF %hour%=="0" (SET /A "hour=12") 

SET "minute=%dt:~10,2%" 

set PushbulletAPI="xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"

curl -u %PushbulletAPI%:ROBSPC https://api.pushbullet.com/v2/pushes -d type=note -d title="Leave a message" -d body="%choice% - %monthname% %day%, %year% at %hour%:%minute%%ap%"
