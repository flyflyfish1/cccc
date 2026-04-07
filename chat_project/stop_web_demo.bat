@echo off
setlocal

echo Stopping demo processes...
taskkill /f /im chat_server.exe >nul 2>nul
taskkill /f /im chat_web_gateway.exe >nul 2>nul

echo Done.
pause
