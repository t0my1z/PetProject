@echo off
REM Script to run chat_creator and chat_joiner in separate terminals

setlocal enabledelayedexpansion

REM Set the paths
set EXE_PATH=C:\Users\trodrigoc\Documents\Personal\PetProject\install\windows\bin\comms_manager.exe
set CONFIG_CREATOR=C:\Users\trodrigoc\Documents\Personal\PetProject\install\windows\config
set CONFIG_JOINER=C:\Users\trodrigoc\Documents\Personal\PetProject\install\windows\config_joiner

REM Verify files exist
if not exist "%EXE_PATH%" (
    echo Error: comms_manager.exe not found at %EXE_PATH%
    pause
    exit /b 1
)

if not exist "%CONFIG_CREATOR%\config.yaml" (
    echo Error: config.yaml not found at %CONFIG_CREATOR%\config.yaml
    pause
    exit /b 1
)

if not exist "%CONFIG_JOINER%\config.yaml" (
    echo Error: config.yaml not found at %CONFIG_JOINER%\config.yaml
    pause
    exit /b 1
)

echo Starting Chat Creator (Server)...
start "Chat Creator" "%EXE_PATH%" --config_folder "%CONFIG_CREATOR%"

REM Wait a moment for the first instance to start
timeout /t 2 /nobreak

echo Starting Chat Joiner (Client)...
start "Chat Joiner" "%EXE_PATH%" --config_folder "%CONFIG_JOINER%"

echo.
echo Both instances started in separate windows!
echo.
