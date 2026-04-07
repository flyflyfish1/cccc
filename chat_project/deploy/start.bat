@echo off
setlocal

set "ROOT=%~dp0"
set "BIN_DIR=%ROOT%bin"
set "SERVER_EXE=%BIN_DIR%\chat_server.exe"
set "GATEWAY_EXE=%BIN_DIR%\chat_web_gateway.exe"

if not exist "%SERVER_EXE%" (
    echo [ERROR] Missing server executable:
    echo %SERVER_EXE%
    pause
    exit /b 1
)

if not exist "%GATEWAY_EXE%" (
    echo [ERROR] Missing web gateway executable:
    echo %GATEWAY_EXE%
    pause
    exit /b 1
)

taskkill /f /im chat_server.exe >nul 2>nul
taskkill /f /im chat_web_gateway.exe >nul 2>nul

echo [1/3] Starting chat server...
start "chat_server" "%SERVER_EXE%"

timeout /t 1 /nobreak >nul

echo [2/3] Starting web gateway...
start "chat_web_gateway" "%GATEWAY_EXE%"

echo [3/3] Waiting for web page...
powershell -NoProfile -Command "$ok=$false; for($i=0;$i -lt 12;$i++){ Start-Sleep -Milliseconds 700; try { $r=Invoke-WebRequest -Uri 'http://127.0.0.1:8080/index.html' -UseBasicParsing -TimeoutSec 2; if($r.StatusCode -eq 200){$ok=$true; break} } catch {} }; if(-not $ok){ exit 1 }"
if errorlevel 1 (
    echo [ERROR] Web page did not start correctly.
    pause
    exit /b 1
)

start "" "http://127.0.0.1:8080"

echo.
echo Deployment started successfully.
echo Web address: http://127.0.0.1:8080
echo.
echo Use stop.bat to stop all services.
exit /b 0
