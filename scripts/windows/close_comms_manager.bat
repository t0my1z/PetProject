@echo off
REM Close all running comms_manager.exe processes

echo Checking for running comms_manager.exe processes...

tasklist | find /i "comms_manager.exe" >nul
if %errorlevel% equ 0 (
    echo Found comms_manager.exe processes. Terminating...
    taskkill /im comms_manager.exe /f >nul 2>&1
    timeout /t 1 /nobreak >nul
    
    REM Verify all processes are closed
    tasklist | find /i "comms_manager.exe" >nul
    if %errorlevel% equ 0 (
        echo Some processes still running. Retrying...
        taskkill /im comms_manager.exe /f >nul 2>&1
    )
    
    echo.
    echo [OK] All comms_manager.exe processes have been closed.
) else (
    echo No comms_manager.exe processes found.
)

pause
