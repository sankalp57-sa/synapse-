@echo off
TITLE Synapse Demo Launcher
echo ==============================================
echo       SYNAPSE - DEMO PRESENTATION
echo ==============================================
echo.
echo [1/3] Starting C++ Backend Server...
start "" main.exe
echo.
echo [2/3] Waiting for server to initialize...
timeout /t 3 /nobreak > nul
echo.
echo [3/3] Opening Synapse Web Portal...
start http://localhost:8080
echo.
echo ==============================================
echo       PRO-TIP: Leave this window open!
echo       Press any key to stop server and exit.
echo ==============================================
pause > nul
taskkill /IM main.exe /F
exit
