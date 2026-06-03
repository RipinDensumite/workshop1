@echo off

g++ *.cpp -o app.exe

if %errorlevel% neq 0 (
    echo Compile failed.
    pause
    exit /b %errorlevel%
)

app.exe
pause