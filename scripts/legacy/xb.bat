@ECHO OFF
REM Copyright 2022 Ben Vanik. All Rights Reserved.

SET "DIR=%~dp0"

REM ============================================================================
REM Environment Validation
REM ============================================================================

SET "PYTHON_MINIMUM_VERSION[0]=3"
SET "PYTHON_MINIMUM_VERSION[1]=6"
CALL :check_python
IF %_RESULT% NEQ 0 (
  ECHO.
  ECHO Python %PYTHON_MINIMUM_VERSION[0]%.%PYTHON_MINIMUM_VERSION[1]%+ must be installed and on PATH:
  ECHO https://www.python.org/
  GOTO :eof
)


REM ============================================================================
REM Trampoline into xenia-build
REM ============================================================================

"%PYTHON_EXE%" "%DIR%\xenia-build" %*
EXIT /b %ERRORLEVEL%


REM ============================================================================
REM Utilities
REM ============================================================================

:check_python
SETLOCAL ENABLEDELAYEDEXPANSION

SET "FOUND_PATH="

REM Try to find Python from PATH first (most reliable for modern setups)
FOR /F "usebackq tokens=*" %%L IN (`where python 2^>nul`) DO (
  IF NOT DEFINED FOUND_PATH (
    SET "FOUND_PATH=%%L"
  )
)

REM If not found on PATH, check common installation directories
IF NOT DEFINED FOUND_PATH (
  FOR %%D IN (
    "C:\python313\python.exe"
    "C:\python312\python.exe"
    "C:\python311\python.exe"
    "C:\python310\python.exe"
    "C:\python39\python.exe"
    "C:\python38\python.exe"
    "C:\python37\python.exe"
    "C:\python36\python.exe"
    "%LOCALAPPDATA%\Programs\Python\Python313\python.exe"
    "%LOCALAPPDATA%\Programs\Python\Python312\python.exe"
    "%LOCALAPPDATA%\Programs\Python\Python311\python.exe"
    "%LOCALAPPDATA%\Programs\Python\Python310\python.exe"
    "%LOCALAPPDATA%\Programs\Python\Python39\python.exe"
  ) DO (
    IF NOT DEFINED FOUND_PATH (
      IF EXIST %%D (
        SET "FOUND_PATH=%%~D"
      )
    )
  )
)

IF NOT DEFINED FOUND_PATH (
  ECHO ERROR: no Python executable found on PATH.
  ECHO Make sure you can run 'python' or 'python3' in a Command Prompt.
  ENDLOCAL & SET _RESULT=1
  GOTO :eof
)

CMD /C ""%FOUND_PATH%" -c "import sys; sys.exit(1 if not sys.version_info[:2] ^>= (%PYTHON_MINIMUM_VERSION[0]%, %PYTHON_MINIMUM_VERSION[1]%) else 0)"
IF %ERRORLEVEL% NEQ 0 (
  ECHO ERROR: Python version mismatch, not at least %PYTHON_MINIMUM_VERSION[0]%.%PYTHON_MINIMUM_VERSION[1]%.
  ECHO Found Python executable was "%FOUND_PATH%".
  ENDLOCAL & SET _RESULT=1
  GOTO :eof
)

ENDLOCAL & (
  SET _RESULT=0
  SET "PYTHON_EXE=%FOUND_PATH%"
)
GOTO :eof
