@echo off
REM Test Marvel vs Capcom 3 and capture output

setlocal

set "XENIA=%~dp0build\bin\Windows\Release\xenia.exe"
set "ISO=c:\Games\Marvel vs. Capcom 3 - Fate of Two Worlds (World) (En,Ja,Fr,De,Es,It).iso"
set "LOGFILE=%~dp0mvc3_test.log"

echo Testing Marvel vs Capcom 3...
echo ISO: %ISO%
echo Log: %LOGFILE%
echo.

"%XENIA%" "%ISO%" > "%LOGFILE%" 2>&1

echo.
echo Test complete! Check %LOGFILE% for details.
pause
