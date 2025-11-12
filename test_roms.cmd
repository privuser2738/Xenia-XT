@echo off
REM Test multiple Xbox 360 ROMs and generate detailed logs

setlocal enabledelayedexpansion

set "ROMS_DIR=c:\Games"
set "LOGS_DIR=%~dp0rom_logs"
set "XENIA=%~dp0build\bin\Windows\Debug\xenia.exe"

if not exist "%XENIA%" (
    echo ERROR: Xenia not found at %XENIA%
    echo Please build Xenia first with: xb.cmd build --config Debug
    pause
    exit /b 1
)

if not exist "%LOGS_DIR%" mkdir "%LOGS_DIR%"

echo ========================================
echo Xbox 360 ROM Testing Suite
echo ========================================
echo.
echo Xenia: %XENIA%
echo ROMs:  %ROMS_DIR%
echo Logs:  %LOGS_DIR%
echo.

set "count=0"
for %%F in ("%ROMS_DIR%\*.iso") do (
    set /a count+=1
    set "name=%%~nF"
    echo [!count!] Testing: !name!

    REM Run with 30 second timeout
    timeout /t 1 /nobreak >nul
    start /wait "" "%XENIA%" "%%F" >nul 2>&1

    REM Kill after 30 seconds if still running
    timeout /t 30 /nobreak >nul
    taskkill /IM xenia.exe /F >nul 2>&1

    echo     Log: %LOGS_DIR%\!name!_load.log
)

if !count!==0 (
    echo No .iso files found in %ROMS_DIR%
) else (
    echo.
    echo ========================================
    echo Tested !count! ROM(s)
    echo Logs saved to: %LOGS_DIR%
    echo ========================================
)

echo.
echo Opening log directory...
start "" "%LOGS_DIR%"

pause
