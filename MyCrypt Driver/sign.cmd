signtool sign /v /s "Personal" /n "Shelekhov-Balchunas Artur (for drivers)" "../x64/Debug/MyCrypt Driver.sys"
signtool.exe verify /pa /v "../x64/Debug/MyCrypt Driver.sys"
pause
