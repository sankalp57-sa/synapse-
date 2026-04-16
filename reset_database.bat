@echo off
TITLE Synapse Database Reset
echo ==============================================
echo       SYNAPSE - DATABASE RESET
echo ==============================================
echo.
echo WARNING: This will delete all current student records.
echo.
set /p choice=Are you sure you want to proceed? (Y/N): 
if /I "%choice%" EQU "Y" (
    del students.txt
    echo. > students.txt
    echo [SUCCESS] Database cleared.
) else (
    echo [CANCELLED] Reset aborted.
)
echo.
pause
exit
