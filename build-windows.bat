@echo off
REM ============================================================================
REM Xenia Windows Build Script - Outputs to dist folder
REM ============================================================================

SETLOCAL ENABLEDELAYEDEXPANSION

SET "ROOT_DIR=%~dp0"
cd /d "%ROOT_DIR%"

echo ============================================================================
echo Xenia Windows Build - Release
echo ============================================================================
echo.

REM ============================================================================
REM Build Release configuration
REM ============================================================================

echo [BUILD] Building Release configuration...
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

REM ============================================================================
REM Create dist folder and copy files
REM ============================================================================

echo [DIST] Creating distribution folder...
echo.

SET "DIST_DIR=%ROOT_DIR%dist"
SET "BUILD_DIR=%ROOT_DIR%build\bin\Windows\Release"

REM Create dist folder if it doesn't exist
if not exist "%DIST_DIR%" mkdir "%DIST_DIR%"

REM Clean old files
echo [DIST] Cleaning old distribution files...
if exist "%DIST_DIR%\xenia.exe" del /q "%DIST_DIR%\xenia.exe"
if exist "%DIST_DIR%\xenia.pdb" del /q "%DIST_DIR%\xenia.pdb"
if exist "%DIST_DIR%\LICENSE" del /q "%DIST_DIR%\LICENSE"

REM Copy main executable
echo [DIST] Copying xenia.exe...
if exist "%BUILD_DIR%\xenia.exe" (
    copy /y "%BUILD_DIR%\xenia.exe" "%DIST_DIR%\" >nul
    if errorlevel 1 (
        echo [ERROR] Failed to copy xenia.exe
        exit /b 1
    )
) else (
    echo [ERROR] xenia.exe not found in build output!
    exit /b 1
)

REM Copy PDB for debugging (optional but useful)
echo [DIST] Copying xenia.pdb...
if exist "%BUILD_DIR%\xenia.pdb" (
    copy /y "%BUILD_DIR%\xenia.pdb" "%DIST_DIR%\" >nul
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
for %%F in ("%DIST_DIR%\xenia.exe") do (
    echo Executable: %%~nxF
    echo Size:       %%~zF bytes
    echo Built:      %%~tF
)

echo.
exit /b 0
