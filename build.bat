@echo off
REM ============================================================================
REM Intelligent Build Script for Xenia
REM Analyzes source changes and rebuilds only when necessary
REM ============================================================================

SETLOCAL ENABLEDELAYEDEXPANSION

SET "ROOT_DIR=%~dp0"
cd /d "%ROOT_DIR%"

echo ============================================================================
echo Xenia Intelligent Build System
echo ============================================================================
echo.

REM ============================================================================
REM Check if initial build is needed
REM ============================================================================

if not exist "build\xenia.sln" (
    echo [ANALYSIS] No solution file found. Running initial setup...
    echo.
    call "%ROOT_DIR%xb.cmd" setup
    if errorlevel 1 (
        echo [ERROR] Setup failed!
        exit /b 1
    )
    echo.
    echo [BUILD] Running initial full build...
    call "%ROOT_DIR%xb.cmd" build
    if errorlevel 1 (
        echo [ERROR] Initial build failed!
        exit /b 1
    )
    echo.
    echo [SUCCESS] Initial build completed!
    exit /b 0
)

REM ============================================================================
REM Analyze what needs rebuilding
REM ============================================================================

echo [ANALYSIS] Checking for source code changes...
echo.

SET "NEEDS_REBUILD=0"
SET "CHANGED_FILES="
SET "REBUILD_REASON="

REM Check if executable exists
if not exist "build\bin\Windows\Debug\xenia.exe" (
    SET "NEEDS_REBUILD=1"
    SET "REBUILD_REASON=Executable not found"
    echo [!] Executable not found - full rebuild needed
    goto :do_build
)

REM Get executable timestamp
for %%F in ("build\bin\Windows\Debug\xenia.exe") do SET "EXE_TIME=%%~tF"
echo [INFO] Current executable: %EXE_TIME%
echo.

REM Check critical source files that were modified
echo [ANALYSIS] Checking modified source files...

SET "FILES_TO_CHECK=src\xenia\app\xenia_main.cc src\xenia\kernel\xboxkrnl\xboxkrnl_memory.cc src\xenia\base\crash_recovery.cc src\xenia\base\crash_recovery.h"

for %%F in (%FILES_TO_CHECK%) do (
    if exist "%%F" (
        for %%E in ("build\bin\Windows\Debug\xenia.exe") do SET "EXE_DATE=%%~tE"
        for %%S in ("%%F") do SET "SRC_DATE=%%~tS"

        REM Compare file times (simple newer check)
        for %%E in ("build\bin\Windows\Debug\xenia.exe") do for %%S in ("%%F") do (
            if "%%~tS" GTR "%%~tE" (
                echo [!] Changed: %%F
                SET "NEEDS_REBUILD=1"
                SET "CHANGED_FILES=!CHANGED_FILES! %%F"
                if "!REBUILD_REASON!"=="" SET "REBUILD_REASON=Source file(s) modified"
            )
        )
    )
)

REM Check if any .cc files in key directories are newer
echo.
echo [ANALYSIS] Scanning source directories for changes...
SET "CHECK_DIRS=src\xenia\app src\xenia\kernel\xboxkrnl src\xenia\base"

for %%D in (%CHECK_DIRS%) do (
    if exist "%%D" (
        for /R "%%D" %%F in (*.cc *.h) do (
            for %%E in ("build\bin\Windows\Debug\xenia.exe") do (
                if "%%~tF" GTR "%%~tE" (
                    if "!NEEDS_REBUILD!"=="0" (
                        echo [!] Newer files detected in %%D
                    )
                    SET "NEEDS_REBUILD=1"
                    if "!REBUILD_REASON!"=="" SET "REBUILD_REASON=Source changes detected"
                )
            )
        )
    )
)

echo.

REM ============================================================================
REM Decide whether to rebuild
REM ============================================================================

if "%NEEDS_REBUILD%"=="0" (
    echo ============================================================================
    echo [SUCCESS] No rebuild needed - all sources are up to date!
    echo ============================================================================
    echo.
    echo Executable: build\bin\Windows\Debug\xenia.exe
    echo Last built: %EXE_TIME%
    echo.
    exit /b 0
)

:do_build
echo ============================================================================
echo [REBUILD] Rebuild required
echo Reason: %REBUILD_REASON%
echo ============================================================================
echo.

if not "%CHANGED_FILES%"=="" (
    echo Changed files:%CHANGED_FILES%
    echo.
)

REM Check if only incremental build is needed
SET "BUILD_TYPE=incremental"
if "%REBUILD_REASON%"=="Executable not found" SET "BUILD_TYPE=full"

echo [BUILD] Starting %BUILD_TYPE% build...
echo.

if "%BUILD_TYPE%"=="full" (
    call "%ROOT_DIR%xb.cmd" build
) else (
    call "%ROOT_DIR%xb.cmd" build --no_premake
)

if errorlevel 1 (
    echo.
    echo ============================================================================
    echo [ERROR] Build failed!
    echo ============================================================================
    echo.
    echo Check the error messages above for details.
    exit /b 1
)

echo.
echo ============================================================================
echo [SUCCESS] Build completed successfully!
echo ============================================================================
echo.

REM Show new executable info
for %%F in ("build\bin\Windows\Debug\xenia.exe") do (
    echo Executable: %%F
    echo Size:       %%~zF bytes
    echo Built:      %%~tF
)

echo.
exit /b 0
