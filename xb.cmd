@ECHO OFF
REM Copyright 2022 Ben Vanik. All Rights Reserved.
REM Native Windows build script - no Python required

SETLOCAL ENABLEDELAYEDEXPANSION

SET "SCRIPT_DIR=%~dp0"
SET "ROOT_DIR=%SCRIPT_DIR%"

REM ============================================================================
REM Parse Command
REM ============================================================================

SET "COMMAND=%~1"
IF "%COMMAND%"=="" (
    ECHO Usage: xb ^<command^> [options]
    ECHO.
    ECHO Commands:
    ECHO   setup       - Setup the build environment
    ECHO   build       - Build the project
    ECHO   premake     - Run premake to update projects
    ECHO   devenv      - Launch Visual Studio
    ECHO   clean       - Clean build artifacts
    ECHO   dist        - Build Release and package to dist folder
    ECHO.
    EXIT /B 1
)

SHIFT

REM ============================================================================
REM Execute Command
REM ============================================================================

IF /I "%COMMAND%"=="setup" GOTO :cmd_setup
IF /I "%COMMAND%"=="build" GOTO :cmd_build
IF /I "%COMMAND%"=="premake" GOTO :cmd_premake
IF /I "%COMMAND%"=="devenv" GOTO :cmd_devenv
IF /I "%COMMAND%"=="clean" GOTO :cmd_clean
IF /I "%COMMAND%"=="pull" GOTO :cmd_pull
IF /I "%COMMAND%"=="dist" GOTO :cmd_dist

ECHO ERROR: Unknown command "%COMMAND%"
EXIT /B 1

REM ============================================================================
REM Setup Command
REM ============================================================================
:cmd_setup
ECHO Setting up the build environment...
ECHO.

REM Setup submodules if git is available
WHERE git >nul 2>&1
IF %ERRORLEVEL% EQU 0 (
    ECHO - git submodule init / update...
    git -c fetch.recurseSubmodules=on-demand submodule update --init
    IF ERRORLEVEL 1 (
        ECHO ERROR: Git submodule update failed.
        EXIT /B 1
    )
    ECHO.
) ELSE (
    ECHO WARNING: Git not available. Dependencies may be missing.
    ECHO.
)

ECHO - running premake...
CALL :run_premake vs2022
IF ERRORLEVEL 1 (
    ECHO ERROR: Premake failed.
    EXIT /B 1
)
ECHO.
ECHO Success!
EXIT /B 0

REM ============================================================================
REM Build Command
REM ============================================================================
:cmd_build
SET "CONFIG=Debug"
SET "TARGET="
SET "FORCE="
SET "NO_PREMAKE="

:parse_build_args
IF "%~1"=="" GOTO :build_execute
IF /I "%~1"=="--config" (
    SET "CONFIG=%~2"
    SHIFT
    SHIFT
    GOTO :parse_build_args
)
IF /I "%~1"=="--target" (
    SET "TARGET=%~2"
    SHIFT
    SHIFT
    GOTO :parse_build_args
)
IF /I "%~1"=="--force" (
    SET "FORCE=1"
    SHIFT
    GOTO :parse_build_args
)
IF /I "%~1"=="--no_premake" (
    SET "NO_PREMAKE=1"
    SHIFT
    GOTO :parse_build_args
)
SHIFT
GOTO :parse_build_args

:build_execute
ECHO Building %CONFIG%...
ECHO.

IF NOT "%NO_PREMAKE%"=="1" (
    ECHO - running premake...
    CALL :run_premake vs2022
    IF ERRORLEVEL 1 EXIT /B 1
    ECHO.
)

REM Find msbuild
CALL :find_msbuild
IF ERRORLEVEL 1 (
    ECHO ERROR: Could not find msbuild.
    ECHO.
    ECHO Please either:
    ECHO   1. Set MSBUILD environment variable to msbuild.exe path
    ECHO   2. Run from a Visual Studio Developer Command Prompt
    ECHO   3. Add msbuild to your PATH
    ECHO   4. Install Visual Studio with C++ development tools
    EXIT /B 1
)

ECHO - building...
SET "BUILD_TARGET="
IF NOT "%TARGET%"=="" (
    IF "%FORCE%"=="1" (
        SET "BUILD_TARGET=/t:%TARGET%:Rebuild"
    ) ELSE (
        SET "BUILD_TARGET=/t:%TARGET%"
    )
) ELSE (
    IF "%FORCE%"=="1" (
        SET "BUILD_TARGET=/t:Rebuild"
    )
)

"%MSBUILD%" build\xenia.sln /nologo /m /v:m /p:Configuration=%CONFIG% /p:Platform=Windows %BUILD_TARGET%
IF ERRORLEVEL 1 (
    ECHO ERROR: Build failed.
    EXIT /B 1
)
ECHO.
ECHO Success!
EXIT /B 0

REM ============================================================================
REM Premake Command
REM ============================================================================
:cmd_premake
ECHO Running premake...
ECHO.
CALL :run_premake vs2022
IF ERRORLEVEL 1 (
    ECHO ERROR: Premake failed.
    EXIT /B 1
)
ECHO Success!
EXIT /B 0

REM ============================================================================
REM Devenv Command
REM ============================================================================
:cmd_devenv
ECHO Launching Visual Studio...
ECHO.

ECHO - running premake...
CALL :run_premake vs2022
IF ERRORLEVEL 1 EXIT /B 1
ECHO.

ECHO - launching devenv...
IF EXIST "build\xenia.sln" (
    START "" devenv "build\xenia.sln"
) ELSE (
    ECHO ERROR: build\xenia.sln not found. Run 'xb setup' first.
    EXIT /B 1
)
EXIT /B 0

REM ============================================================================
REM Clean Command
REM ============================================================================
:cmd_clean
ECHO Cleaning build artifacts...
ECHO.

IF EXIST "build\" (
    ECHO - removing build/...
    RMDIR /S /Q "build\"
    ECHO.
)

ECHO Success!
EXIT /B 0

REM ============================================================================
REM Pull Command
REM ============================================================================
:cmd_pull
ECHO Pulling...
ECHO.

WHERE git >nul 2>&1
IF %ERRORLEVEL% NEQ 0 (
    ECHO ERROR: Git is not installed or not in PATH.
    EXIT /B 1
)

ECHO - switching to master...
git checkout master
IF ERRORLEVEL 1 EXIT /B 1
ECHO.

ECHO - pulling self...
git pull --rebase
IF ERRORLEVEL 1 EXIT /B 1
ECHO.

ECHO - pulling dependencies...
git -c fetch.recurseSubmodules=on-demand submodule update --init
IF ERRORLEVEL 1 EXIT /B 1
ECHO.

ECHO - running premake...
CALL :run_premake vs2022
IF ERRORLEVEL 1 EXIT /B 1
ECHO.

ECHO Success!
EXIT /B 0

REM ============================================================================
REM Helper: Run Premake
REM ============================================================================
:run_premake
SET "ACTION=%~1"
IF "%ACTION%"=="" SET "ACTION=vs2022"

REM Find premake5 executable
SET "PREMAKE5="
IF EXIST "third_party\premake-core\bin\release\premake5.exe" (
    SET "PREMAKE5=third_party\premake-core\bin\release\premake5.exe"
) ELSE IF EXIST "tools\build\bin\premake5.exe" (
    SET "PREMAKE5=tools\build\bin\premake5.exe"
) ELSE (
    ECHO ERROR: premake5.exe not found. Please ensure premake-core submodule is initialized.
    EXIT /B 1
)

REM Ensure premake scripts exist
IF NOT EXIST "third_party\premake-core\scripts\package.lua" (
    ECHO ERROR: third_party/premake-core was not present; run 'xb setup'...
    EXIT /B 1
)

REM Generate version header
CALL :generate_version_h

REM Run premake
"%PREMAKE5%" --scripts=third_party\premake-core --file=premake5.lua --os=windows --test-suite-mode=combined --verbose %ACTION%
EXIT /B %ERRORLEVEL%

REM ============================================================================
REM Helper: Generate version.h
REM ============================================================================
:generate_version_h
SET "HEADER_FILE=build\version.h"
SET "BRANCH_NAME=unknown"
SET "COMMIT=unknown"
SET "COMMIT_SHORT=unknown"

REM Create build directory if it doesn't exist
IF NOT EXIST "build\" MKDIR "build\"

REM Try to get git info
WHERE git >nul 2>&1
IF %ERRORLEVEL% EQU 0 (
    REM Check if we're in a git repository
    git rev-parse --is-inside-work-tree >nul 2>&1
    IF !ERRORLEVEL! EQU 0 (
        REM Get branch name
        FOR /F "tokens=*" %%i IN ('git symbolic-ref --short -q HEAD 2^>nul') DO SET "BRANCH_NAME=%%i"
        IF "!BRANCH_NAME!"=="" SET "BRANCH_NAME=detached"

        REM Get commit hash
        FOR /F "tokens=*" %%i IN ('git rev-parse HEAD 2^>nul') DO SET "COMMIT=%%i"

        REM Get short commit hash
        FOR /F "tokens=*" %%i IN ('git rev-parse --short HEAD 2^>nul') DO SET "COMMIT_SHORT=%%i"
    )
)

REM Write header file
(
    ECHO // Autogenerated by `xb premake`.
    ECHO #ifndef GENERATED_VERSION_H_
    ECHO #define GENERATED_VERSION_H_
    ECHO #define XE_BUILD_BRANCH "!BRANCH_NAME!"
    ECHO #define XE_BUILD_COMMIT "!COMMIT!"
    ECHO #define XE_BUILD_COMMIT_SHORT "!COMMIT_SHORT!"
    ECHO #define XE_BUILD_DATE __DATE__
    ECHO #endif  // GENERATED_VERSION_H_
) > "%HEADER_FILE%"

EXIT /B 0

REM ============================================================================
REM Helper: Find MSBuild
REM ============================================================================
:find_msbuild
REM IMPORTANT: Prefer Visual Studio's msbuild over standalone versions
REM because it comes with C++ build tools pre-configured

REM First, try to find Visual Studio's msbuild (has C++ tools built-in)
FOR %%D IN (
    "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
    "C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe"
    "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe"
    "C:\Program Files\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe"
    "C:\Program Files\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe"
    "C:\Program Files\Microsoft Visual Studio\2019\Professional\MSBuild\Current\Bin\MSBuild.exe"
    "C:\Program Files\Microsoft Visual Studio\2019\Enterprise\MSBuild\Current\Bin\MSBuild.exe"
    "C:\Program Files\Microsoft Visual Studio\2019\BuildTools\MSBuild\Current\Bin\MSBuild.exe"
    "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\MSBuild\15.0\Bin\MSBuild.exe"
    "C:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\MSBuild\15.0\Bin\MSBuild.exe"
    "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\MSBuild\15.0\Bin\MSBuild.exe"
) DO (
    IF EXIST %%D (
        REM Remove quotes from %%D when assigning
        SET "MSBUILD=%%~D"
        ECHO Found Visual Studio msbuild at: !MSBUILD!
        REM Set up VCTargetsPath for C++ builds
        CALL :setup_vctargets_path "!MSBUILD!"
        EXIT /B 0
    )
)

REM Check if MSBUILD environment variable is set
IF DEFINED MSBUILD (
    IF EXIST "%MSBUILD%" (
        ECHO Found msbuild at: %MSBUILD% (from MSBUILD env var)
        CALL :setup_vctargets_path "%MSBUILD%"
        EXIT /B 0
    )
)

REM Check if msbuild is in PATH
WHERE msbuild >nul 2>&1
IF NOT ERRORLEVEL 1 (
    FOR /F "tokens=*" %%i IN ('where msbuild') DO SET "MSBUILD=%%i"
    ECHO Found msbuild in PATH: !MSBUILD!
    CALL :setup_vctargets_path "!MSBUILD!"
    EXIT /B 0
)

REM Try to set up VS environment and check again
ECHO msbuild not found. Attempting to locate Visual Studio...
CALL :setup_vs_env
IF NOT ERRORLEVEL 1 (
    WHERE msbuild >nul 2>&1
    IF NOT ERRORLEVEL 1 (
        FOR /F "tokens=*" %%i IN ('where msbuild') DO SET "MSBUILD=%%i"
        ECHO Found msbuild after VS setup: !MSBUILD!
        CALL :setup_vctargets_path "!MSBUILD!"
        EXIT /B 0
    )
)

REM Check standalone locations (last resort - may not have C++ tools)
IF EXIST "C:\c\msbuild.exe" (
    SET "MSBUILD=C:\c\msbuild.exe"
    ECHO WARNING: Using standalone msbuild at: !MSBUILD!
    ECHO This may not have C++ build tools configured.
    CALL :setup_vctargets_path "!MSBUILD!"
    EXIT /B 0
)

ECHO ERROR: msbuild not found anywhere
EXIT /B 1

REM ============================================================================
REM Helper: Setup VCTargets Path for C++ Builds
REM ============================================================================
:setup_vctargets_path
SET "MSBUILD_PATH=%~1"

REM Try to find VS installation from msbuild path
REM Expected path: ...\Microsoft Visual Studio\YEAR\EDITION\MSBuild\...
FOR %%P IN ("%MSBUILD_PATH%") DO SET "MSBUILD_DIR=%%~dpP"

REM Look for VS installation root
SET "VS_ROOT="
IF "!MSBUILD_DIR:~0,40!"=="C:\Program Files\Microsoft Visual Studio\" (
    REM Extract VS root from path
    FOR /F "tokens=1-5 delims=\" %%a IN ("!MSBUILD_DIR!") DO (
        IF EXIST "%%a\%%b\%%c\%%d\%%e\MSBuild\Microsoft\VC" (
            SET "VS_ROOT=%%a\%%b\%%c\%%d\%%e"
        )
    )
)

IF "!MSBUILD_DIR:~0,48!"=="C:\Program Files (x86)\Microsoft Visual Studio\" (
    REM Extract VS root from path
    FOR /F "tokens=1-6 delims=\" %%a IN ("!MSBUILD_DIR!") DO (
        IF EXIST "%%a\%%b\%%c\%%d\%%e\%%f\MSBuild\Microsoft\VC" (
            SET "VS_ROOT=%%a\%%b\%%c\%%d\%%e\%%f"
        )
    )
)

IF DEFINED VS_ROOT (
    REM Find the VC version directory (v170 for VS2022, v160 for VS2019, etc.)
    FOR /D %%V IN ("!VS_ROOT!\MSBuild\Microsoft\VC\v*") DO (
        IF EXIST "%%V\Microsoft.Cpp.Default.props" (
            SET "VCTargetsPath=%%V\"
            ECHO Set VCTargetsPath=!VCTargetsPath!
        )
    )
)

REM If still not found, try to find it by searching common locations
IF NOT DEFINED VCTargetsPath (
    FOR %%D IN (
        "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Microsoft\VC\v170\"
        "C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Microsoft\VC\v170\"
        "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\MSBuild\Microsoft\VC\v170\"
        "C:\Program Files\Microsoft Visual Studio\2019\Community\MSBuild\Microsoft\VC\v160\"
        "C:\Program Files\Microsoft Visual Studio\2019\Professional\MSBuild\Microsoft\VC\v160\"
        "C:\Program Files\Microsoft Visual Studio\2019\Enterprise\MSBuild\Microsoft\VC\v160\"
        "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\MSBuild\Microsoft\VC\v150\"
        "C:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\MSBuild\Microsoft\VC\v150\"
        "C:\Program Files (x86)\Microsoft Visual Studio\2017\Enterprise\MSBuild\Microsoft\VC\v150\"
    ) DO (
        IF EXIST %%D (
            REM Remove quotes when assigning
            SET "VCTargetsPath=%%~D"
            ECHO Set VCTargetsPath=!VCTargetsPath!
            GOTO :vctargets_done
        )
    )
)

:vctargets_done
IF NOT DEFINED VCTargetsPath (
    ECHO WARNING: Could not set VCTargetsPath. C++ builds may fail.
)
EXIT /B 0

REM ============================================================================
REM Helper: Setup Visual Studio Environment
REM ============================================================================
:setup_vs_env
ECHO Attempting to configure Visual Studio environment...

REM Use vswhere to find Visual Studio
SET "VSWHERE=%ROOT_DIR%tools\vswhere\vswhere.exe"
IF NOT EXIST "%VSWHERE%" (
    ECHO ERROR: vswhere.exe not found at %VSWHERE%
    EXIT /B 1
)

REM Find VS installation path
FOR /F "usebackq tokens=*" %%i IN (`"%VSWHERE%" -version "[15,)" -latest -prerelease -property installationPath -products Microsoft.VisualStudio.Product.Enterprise Microsoft.VisualStudio.Product.Professional Microsoft.VisualStudio.Product.Community Microsoft.VisualStudio.Product.BuildTools`) DO (
    SET "VS_INSTALL_PATH=%%i"
)

IF NOT DEFINED VS_INSTALL_PATH (
    ECHO ERROR: Visual Studio 2017 or later not found.
    EXIT /B 1
)

ECHO Found Visual Studio at: !VS_INSTALL_PATH!

REM Find and call VsDevCmd.bat to set up environment
SET "VSDEVCMD=!VS_INSTALL_PATH!\Common7\Tools\VsDevCmd.bat"
IF EXIST "!VSDEVCMD!" (
    ECHO Setting up VS environment via VsDevCmd.bat...
    CALL "!VSDEVCMD!" -arch=amd64 -host_arch=amd64 -no_logo
    EXIT /B 0
)

REM Fallback to vcvarsall.bat
SET "VCVARSALL=!VS_INSTALL_PATH!\VC\Auxiliary\Build\vcvarsall.bat"
IF EXIST "!VCVARSALL!" (
    ECHO Setting up VS environment via vcvarsall.bat...
    CALL "!VCVARSALL!" x64
    EXIT /B 0
)

ECHO ERROR: Could not find VsDevCmd.bat or vcvarsall.bat in VS installation
EXIT /B 1

REM ============================================================================
REM Dist Command - Build Release and Package
REM ============================================================================
:cmd_dist
ECHO Creating distribution package...
ECHO.

REM Build Release first
ECHO - building Release configuration...
CALL :cmd_build_internal Release
IF ERRORLEVEL 1 (
    ECHO ERROR: Release build failed.
    EXIT /B 1
)
ECHO.

REM Create dist directory
SET "DIST_DIR=%ROOT_DIR%dist"
IF EXIST "%DIST_DIR%" (
    ECHO - cleaning existing dist folder...
    RMDIR /S /Q "%DIST_DIR%"
)
MKDIR "%DIST_DIR%"

REM Copy main executables
ECHO - copying executables...
SET "BIN_DIR=%ROOT_DIR%build\bin\Windows\Release"

IF EXIST "%BIN_DIR%\xenia.exe" (
    COPY "%BIN_DIR%\xenia.exe" "%DIST_DIR%\" >nul
    ECHO   + xenia.exe
)
IF EXIST "%BIN_DIR%\xenia-vfs-dump.exe" (
    COPY "%BIN_DIR%\xenia-vfs-dump.exe" "%DIST_DIR%\" >nul
    ECHO   + xenia-vfs-dump.exe
)
IF EXIST "%BIN_DIR%\xenia-gpu-shader-compiler.exe" (
    COPY "%BIN_DIR%\xenia-gpu-shader-compiler.exe" "%DIST_DIR%\" >nul
    ECHO   + xenia-gpu-shader-compiler.exe
)

REM Copy any DLLs that might be needed
ECHO - copying any required DLLs...
FOR %%F IN ("%BIN_DIR%\*.dll") DO (
    COPY "%%F" "%DIST_DIR%\" >nul
    ECHO   + %%~nxF
)

REM Copy license
IF EXIST "%ROOT_DIR%LICENSE" (
    COPY "%ROOT_DIR%LICENSE" "%DIST_DIR%\" >nul
    ECHO   + LICENSE
)

ECHO.
ECHO Distribution package created at: %DIST_DIR%
ECHO.
DIR "%DIST_DIR%"
ECHO.
ECHO Success!
EXIT /B 0

:cmd_build_internal
SET "INT_CONFIG=%~1"
IF "%INT_CONFIG%"=="" SET "INT_CONFIG=Release"

REM Run premake
CALL :run_premake vs2022
IF ERRORLEVEL 1 EXIT /B 1

REM Find msbuild
CALL :find_msbuild
IF ERRORLEVEL 1 EXIT /B 1

REM Build
"%MSBUILD%" build\xenia.sln /nologo /m /v:m /p:Configuration=%INT_CONFIG% /p:Platform=Windows
EXIT /B %ERRORLEVEL%
