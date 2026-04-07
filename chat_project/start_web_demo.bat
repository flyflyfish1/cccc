@echo off
setlocal

set "ROOT=%~dp0"
set "SERVER_EXE=%ROOT%server\release\chat_server.exe"
set "GATEWAY_EXE=%ROOT%web_gateway\release\chat_web_gateway.exe"
set "WEB_DIR=%ROOT%web_client"
set "WEB_LOG=%WEB_DIR%\http.err.log"

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

where py >nul 2>nul
if errorlevel 1 (
    echo [ERROR] Python launcher ^(py^) was not found.
    pause
    exit /b 1
)

set "PY_EXE="
for /f "delims=" %%I in ('where py') do (
    set "PY_EXE=%%I"
    goto :py_found
)

:py_found
if not defined PY_EXE (
    echo [ERROR] Could not resolve py.exe path.
    pause
    exit /b 1
)

taskkill /f /im chat_server.exe >nul 2>nul
taskkill /f /im chat_web_gateway.exe >nul 2>nul
taskkill /f /im python.exe >nul 2>nul
taskkill /f /im py.exe >nul 2>nul

echo [1/4] Starting TCP server...
start "chat_server" "%SERVER_EXE%"

timeout /t 1 /nobreak >nul

echo [2/4] Starting WebSocket gateway...
start "chat_web_gateway" "%GATEWAY_EXE%"

timeout /t 1 /nobreak >nul

echo [3/4] Starting static web server on http://127.0.0.1:8080 ...
if exist "%WEB_LOG%" del /q "%WEB_LOG%"
start "chat_web_http" /min "%PY_EXE%" -3 -m http.server 8080 --bind 127.0.0.1 --directory "%WEB_DIR%"

echo Waiting for local web server...
powershell -NoProfile -Command "$ok=$false; for($i=0;$i -lt 10;$i++){ Start-Sleep -Milliseconds 700; try { $r=Invoke-WebRequest -Uri 'http://127.0.0.1:8080/index.html' -UseBasicParsing -TimeoutSec 2; if($r.StatusCode -eq 200){$ok=$true; break} } catch {} }; if(-not $ok){ exit 1 }"
if errorlevel 1 (
    echo [ERROR] Web page server did not start correctly.
    echo Please check:
    echo %WEB_DIR%
    pause
    exit /b 1
)

echo [4/4] Opening browser...
start "" "http://127.0.0.1:8080"

echo.
echo Demo stack started:
echo - TCP server: 127.0.0.1:8888
echo - WebSocket gateway: ws://127.0.0.1:9999
echo - Web client: http://127.0.0.1:8080
echo.
echo Run stop_web_demo.bat when you want to stop everything.
exit /b 0
