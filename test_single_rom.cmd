@echo off
REM Test a single ROM with verbose output to console and log file

setlocal

set "XENIA=%~dp0build\bin\Windows\Debug\xenia.exe"
set "LOGS_DIR=%~dp0rom_logs"
set "ISO=%~1"

if not exist "%XENIA%" (
    echo ERROR: Xenia not found at %XENIA%
    echo Please build Xenia first with: xb.cmd build --config Debug
    pause
    exit /b 1
)

if "%ISO%"=="" (
    echo Usage: test_single_rom.cmd "path\to\game.iso"
    echo.
    echo Example:
    echo   test_single_rom.cmd "c:\Games\sfxt.iso"
    pause
    exit /b 1
)

if not exist "%ISO%" (
    echo ERROR: ISO file not found: %ISO%
    pause
    exit /b 1
)

if not exist "%LOGS_DIR%" mkdir "%LOGS_DIR%"

REM Extract filename without extension
for %%F in ("%ISO%") do set "name=%%~nF"

set "LOGFILE=%LOGS_DIR%\%name%_detailed.log"

echo ========================================
echo Testing: %name%
echo ISO: %ISO%
echo Log: %LOGFILE%
echo ========================================
echo.
echo Starting Xenia... (output will be logged)
echo Press Ctrl+C to stop
echo.

REM Run and display output while also saving to log
"%XENIA%" "%ISO%" 2>&1 | tee "%LOGFILE%"

echo.
echo ========================================
echo Test complete!
echo Full log saved to: %LOGFILE%
echo ========================================
pause
