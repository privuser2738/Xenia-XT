@echo off
REM ============================================================================
REM Xenia-XT Windows Build Script
REM ============================================================================
REM
REM Usage:
REM   build-windows.bat [command] [options]
REM
REM Commands:
REM   build     Build the project (default)
REM   clean     Clean build outputs
REM   setup     Setup/update submodules and run premake
REM   dist      Build and create distribution package
REM   help      Show this help message
REM
REM Options:
REM   --config=Debug|Release    Build configuration (default: Release)
REM   --verbose                 Show verbose output
REM
REM Examples:
REM   build-windows.bat                    Build Release
REM   build-windows.bat build              Build Release
REM   build-windows.bat --config=Debug     Build Debug
REM   build-windows.bat clean              Clean build outputs
REM   build-windows.bat setup              Setup project
REM   build-windows.bat dist               Build and package for distribution
REM
REM ============================================================================

SETLOCAL ENABLEDELAYEDEXPANSION

SET "ROOT_DIR=%~dp0"
cd /d "%ROOT_DIR%"

REM Default values
SET "COMMAND=build"
SET "CONFIG=Release"
SET "VERBOSE="

REM Parse arguments
:parse_args
IF "%~1"=="" GOTO :done_parsing
IF /I "%~1"=="build" (
    SET "COMMAND=build"
    SHIFT
    GOTO :parse_args
)
IF /I "%~1"=="clean" (
    SET "COMMAND=clean"
    SHIFT
    GOTO :parse_args
)
IF /I "%~1"=="setup" (
    SET "COMMAND=setup"
    SHIFT
    GOTO :parse_args
)
IF /I "%~1"=="dist" (
    SET "COMMAND=dist"
    SHIFT
    GOTO :parse_args
)
IF /I "%~1"=="help" (
    SET "COMMAND=help"
    SHIFT
    GOTO :parse_args
)
IF /I "%~1"=="--config=Debug" (
    SET "CONFIG=Debug"
    SHIFT
    GOTO :parse_args
)
IF /I "%~1"=="--config=Release" (
    SET "CONFIG=Release"
    SHIFT
    GOTO :parse_args
)
IF /I "%~1"=="--verbose" (
    SET "VERBOSE=1"
    SHIFT
    GOTO :parse_args
)
REM Unknown argument, skip
SHIFT
GOTO :parse_args

:done_parsing

REM Execute command
IF "%COMMAND%"=="help" GOTO :show_help
IF "%COMMAND%"=="clean" GOTO :do_clean
IF "%COMMAND%"=="setup" GOTO :do_setup
IF "%COMMAND%"=="dist" GOTO :do_dist
IF "%COMMAND%"=="build" GOTO :do_build
GOTO :do_build

REM ============================================================================
:show_help
REM ============================================================================
echo.
echo ============================================================================
echo Xenia-XT Windows Build Script
echo ============================================================================
echo.
echo Usage:
echo   build-windows.bat [command] [options]
echo.
echo Commands:
echo   build     Build the project (default)
echo   clean     Clean build outputs
echo   setup     Setup/update submodules and run premake
echo   dist      Build and create distribution package
echo   help      Show this help message
echo.
echo Options:
echo   --config=Debug^|Release    Build configuration (default: Release)
echo   --verbose                 Show verbose output
echo.
echo Examples:
echo   build-windows.bat                    Build Release
echo   build-windows.bat build              Build Release
echo   build-windows.bat --config=Debug     Build Debug
echo   build-windows.bat clean              Clean build outputs
echo   build-windows.bat setup              Setup project
echo   build-windows.bat dist               Build and package for distribution
echo.
exit /b 0

REM ============================================================================
:do_setup
REM ============================================================================
echo.
echo ============================================================================
echo Xenia-XT Setup
echo ============================================================================
echo.

echo [SETUP] Updating submodules...
git submodule update --init --recursive
if errorlevel 1 (
    echo [ERROR] Failed to update submodules!
    exit /b 1
)

echo [SETUP] Running premake...
call "%ROOT_DIR%xb.cmd" premake
if errorlevel 1 (
    echo [ERROR] Premake failed!
    exit /b 1
)

echo.
echo [SUCCESS] Setup complete!
exit /b 0

REM ============================================================================
:do_clean
REM ============================================================================
echo.
echo ============================================================================
echo Xenia-XT Clean
echo ============================================================================
echo.

echo [CLEAN] Cleaning build outputs...
call "%ROOT_DIR%xb.cmd" clean
if errorlevel 1 (
    echo [WARNING] Clean may have partially failed
)

echo.
echo [SUCCESS] Clean complete!
exit /b 0

REM ============================================================================
:do_build
REM ============================================================================
echo.
echo ============================================================================
echo Xenia-XT Build - %CONFIG%
echo ============================================================================
echo.

echo [BUILD] Building %CONFIG% configuration...
echo.

call "%ROOT_DIR%xb.cmd" build --config %CONFIG%
if errorlevel 1 (
    echo.
    echo [ERROR] Build failed!
    exit /b 1
)

echo.
echo [SUCCESS] Build completed successfully!
echo.
echo Output: build\bin\Windows\%CONFIG%\xenia-xt.exe
exit /b 0

REM ============================================================================
:do_dist
REM ============================================================================
echo.
echo ============================================================================
echo Xenia-XT Distribution Build
echo ============================================================================
echo.

REM First do a release build
echo [DIST] Building Release configuration...
echo.

call "%ROOT_DIR%xb.cmd" build --config Release
if errorlevel 1 (
    echo.
    echo [ERROR] Build failed!
    exit /b 1
)

echo.
echo [BUILD] Build completed successfully!
echo.

REM Create dist folder and copy files
echo [DIST] Creating distribution folder...
echo.

SET "DIST_DIR=%ROOT_DIR%dist"
SET "BUILD_DIR=%ROOT_DIR%build\bin\Windows\Release"

REM Create dist folder if it doesn't exist
if not exist "%DIST_DIR%" mkdir "%DIST_DIR%"

REM Clean old files
echo [DIST] Cleaning old distribution files...
if exist "%DIST_DIR%\xenia-xt.exe" del /q "%DIST_DIR%\xenia-xt.exe"
if exist "%DIST_DIR%\xenia-xt.pdb" del /q "%DIST_DIR%\xenia-xt.pdb"
if exist "%DIST_DIR%\LICENSE" del /q "%DIST_DIR%\LICENSE"

REM Copy main executable
echo [DIST] Copying xenia-xt.exe...
if exist "%BUILD_DIR%\xenia-xt.exe" (
    copy /y "%BUILD_DIR%\xenia-xt.exe" "%DIST_DIR%\" >nul
    if errorlevel 1 (
        echo [ERROR] Failed to copy xenia-xt.exe
        exit /b 1
    )
) else (
    echo [ERROR] xenia-xt.exe not found in build output!
    exit /b 1
)

REM Copy PDB for debugging (optional but useful)
echo [DIST] Copying xenia-xt.pdb...
if exist "%BUILD_DIR%\xenia-xt.pdb" (
    copy /y "%BUILD_DIR%\xenia-xt.pdb" "%DIST_DIR%\" >nul
)

REM Copy LICENSE file
echo [DIST] Copying LICENSE...
if exist "%ROOT_DIR%LICENSE" (
    copy /y "%ROOT_DIR%LICENSE" "%DIST_DIR%\" >nul
)

REM Copy any required DLLs (if any exist in build folder)
echo [DIST] Copying any required DLLs...
if exist "%BUILD_DIR%\*.dll" (
    copy /y "%BUILD_DIR%\*.dll" "%DIST_DIR%\" >nul 2>nul
)

echo.
echo ============================================================================
echo [SUCCESS] Distribution created successfully!
echo ============================================================================
echo.
echo Output folder: %DIST_DIR%
echo.

REM Show what was copied
echo Contents:
dir /b "%DIST_DIR%"

echo.

REM Show executable info
for %%F in ("%DIST_DIR%\xenia-xt.exe") do (
    echo Executable: %%~nxF
    echo Size:       %%~zF bytes
    echo Built:      %%~tF
)

echo.
exit /b 0
