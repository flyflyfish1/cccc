@echo off
setlocal

taskkill /f /im chat_server.exe >nul 2>nul
taskkill /f /im chat_web_gateway.exe >nul 2>nul

echo Services stopped.
pause
