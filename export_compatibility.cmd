@ECHO OFF
REM Export Xenia compatibility data for sharing with the community

SETLOCAL
SET "SCRIPT_DIR=%~dp0"
SET "OUTPUT_DIR=%SCRIPT_DIR%compatibility_export"
SET "TIMESTAMP=%DATE:~-4%%DATE:~4,2%%DATE:~7,2%_%TIME:~0,2%%TIME:~3,2%%TIME:~6,2%"
SET "TIMESTAMP=%TIMESTAMP: =0%"

ECHO ========================================
ECHO Xenia Compatibility Data Exporter
ECHO ========================================
ECHO.

REM Create output directory
IF NOT EXIST "%OUTPUT_DIR%" (
    MKDIR "%OUTPUT_DIR%"
    ECHO Created output directory: %OUTPUT_DIR%
)

REM Copy crash recovery database
IF EXIST "%SCRIPT_DIR%crash_recovery.db" (
    ECHO Exporting crash recovery data...
    COPY "%SCRIPT_DIR%crash_recovery.db" "%OUTPUT_DIR%\crash_recovery_%TIMESTAMP%.db" >nul
    ECHO   - Saved: crash_recovery_%TIMESTAMP%.db
) ELSE (
    ECHO   - No crash recovery data found
)

REM Create metadata file
ECHO Creating metadata file...
(
    ECHO # Xenia Compatibility Export
    ECHO Export Date: %DATE% %TIME%
    ECHO User: %USERNAME%
    ECHO System: %COMPUTERNAME%
    ECHO OS: %OS%
    ECHO.
    ECHO ## Instructions
    ECHO 1. Share these files on GitHub:
    ECHO    https://github.com/xenia-project/game-compatibility
    ECHO.
    ECHO 2. Include information about:
    ECHO    - Your GPU model and driver version
    ECHO    - Your CPU model
    ECHO    - Which games you tested
    ECHO    - Any issues you encountered
    ECHO.
    ECHO 3. Your contribution helps everyone!
    ECHO.
    ECHO ## Files in this export
) > "%OUTPUT_DIR%\README.txt"

IF EXIST "%OUTPUT_DIR%\crash_recovery_%TIMESTAMP%.db" (
    ECHO    - crash_recovery_%TIMESTAMP%.db ^(crash patterns and workarounds^) >> "%OUTPUT_DIR%\README.txt"
)

ECHO   - Saved: README.txt
ECHO.

REM Show what was exported
ECHO ========================================
ECHO Export Complete!
ECHO ========================================
ECHO.
ECHO Files saved to: %OUTPUT_DIR%
ECHO.
DIR "%OUTPUT_DIR%" /B
ECHO.
ECHO Next steps:
ECHO 1. Open GitHub: https://github.com/xenia-project/game-compatibility
ECHO 2. Create a new issue or discussion
ECHO 3. Upload the files from: %OUTPUT_DIR%
ECHO 4. Describe your testing experience
ECHO.
ECHO Thank you for contributing to Xenia!
ECHO.

PAUSE
