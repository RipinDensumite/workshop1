@echo off

g++ main.cpp login.cpp inventory.cpp StockMonitor.cpp ExpiryTrack.cpp Report.cpp db.cpp mainmenu.cpp -o app.exe

if %errorlevel% neq 0 (
    echo Compile failed.
    pause
    exit /b %errorlevel%
)

app.exe
pause