@echo off
setlocal enabledelayedexpansion

REM Git Update Script for Windows
REM Handles git initialization, gitignore setup, size check, and GitHub repo creation

set "USERNAME=privuser2738"
for %%I in (.) do set "REPO_NAME=%%~nxI"

:MAIN_MENU
cls
echo =========================================
echo     Git Repository Manager
echo =========================================
echo Repository: %REPO_NAME%
echo User: %USERNAME%
echo.
echo 1) Full Setup (init, gitignore, size check, push)
echo 2) Initialize Git Repository
echo 3) Update .gitignore
echo 4) Check Repository Size
echo 5) Create GitHub Repository
echo 6) Commit and Push Changes
echo 7) Exit
echo.

set /p choice="Select an option [1-7]: "

if "%choice%"=="1" goto FULL_SETUP
if "%choice%"=="2" goto INIT_GIT
if "%choice%"=="3" goto UPDATE_GITIGNORE
if "%choice%"=="4" goto CHECK_SIZE
if "%choice%"=="5" goto CREATE_GITHUB
if "%choice%"=="6" goto COMMIT_PUSH
if "%choice%"=="7" goto EXIT
echo Invalid option. Please try again.
pause
goto MAIN_MENU

:CHECK_GIT
where git >nul 2>&1
if errorlevel 1 (
    echo Error: git is not installed. Please install git first.
    pause
    exit /b 1
)
exit /b 0

:CHECK_GH
where gh >nul 2>&1
if errorlevel 1 (
    exit /b 1
)
exit /b 0

:INIT_GIT
call :CHECK_GIT
if errorlevel 1 goto MAIN_MENU

echo.
echo [*] Initializing git repository...
if exist .git (
    echo [!] Git repository already initialized
) else (
    git init
    if errorlevel 1 (
        echo [ERROR] Failed to initialize git repository
    ) else (
        echo [OK] Git repository initialized
    )
)
pause
goto MAIN_MENU

:UPDATE_GITIGNORE
echo.
echo [*] Updating .gitignore...

if not exist .gitignore (
    type nul > .gitignore
    echo [OK] Created new .gitignore file
)

set "entries=/deps /dist /bin /build"
for %%e in (%entries%) do (
    findstr /x /c:"%%e" .gitignore >nul 2>&1
    if errorlevel 1 (
        echo %%e >> .gitignore
        echo [OK] Added %%e to .gitignore
    ) else (
        echo [!] %%e already in .gitignore
    )
)
pause
goto MAIN_MENU

:CHECK_SIZE
echo.
echo [*] Checking repository size...

set "total_size=0"
set "file_count=0"

REM Use PowerShell to calculate size more accurately
powershell -Command "$size = 0; Get-ChildItem -Recurse -File | Where-Object { $_.FullName -notmatch '[\\/]\.git[\\/]' } | ForEach-Object { $size += $_.Length }; $sizeMB = [math]::Round($size / 1MB, 2); Write-Host \"Total size: $sizeMB MB\"; if ($sizeMB -gt 100) { exit 1 } else { exit 0 }"
set "size_check=%errorlevel%"

echo.
echo [*] Finding largest items...
echo.
echo Top 10 largest directories:
powershell -Command "Get-ChildItem -Directory | Where-Object { $_.Name -ne '.git' } | ForEach-Object { $size = (Get-ChildItem $_.FullName -Recurse -File -ErrorAction SilentlyContinue | Measure-Object -Property Length -Sum).Sum; [PSCustomObject]@{Name=$_.Name; SizeMB=[math]::Round($size/1MB, 2)} } | Sort-Object SizeMB -Descending | Select-Object -First 10 | Format-Table -AutoSize"

echo.
echo Top 10 largest files:
powershell -Command "Get-ChildItem -Recurse -File | Where-Object { $_.FullName -notmatch '[\\/]\.git[\\/]' } | Sort-Object Length -Descending | Select-Object -First 10 | ForEach-Object { [PSCustomObject]@{Name=$_.Name; Path=$_.DirectoryName; SizeMB=[math]::Round($_.Length/1MB, 2)} } | Format-Table -AutoSize"

if "%size_check%"=="1" (
    echo.
    echo [!] Repository size exceeds 100MB!
    call :HANDLE_LARGE_REPO
)

pause
goto MAIN_MENU

:HANDLE_LARGE_REPO
echo.
set /p "add_ignore=Do you want to add items to .gitignore? (y/n): "

REM Trim whitespace and convert to lowercase for comparison
set "add_ignore=%add_ignore: =%"

if /i "%add_ignore%"=="y" (
    echo Enter paths to add to .gitignore (type 'done' when finished):
    goto ADD_IGNORE_LOOP
)

echo [!] Skipping .gitignore updates. Proceeding anyway...
exit /b 0

:ADD_IGNORE_LOOP
set /p "path=Path: "
if /i "%path%"=="done" goto ADD_IGNORE_DONE
if not "%path%"=="" (
    echo %path% >> .gitignore
    echo [OK] Added %path% to .gitignore
)
goto ADD_IGNORE_LOOP

:ADD_IGNORE_DONE
exit /b 0

:CREATE_GITHUB
call :CHECK_GIT
if errorlevel 1 goto MAIN_MENU

call :CHECK_GH
if errorlevel 1 (
    echo [ERROR] gh CLI is not installed. Cannot create GitHub repository.
    echo Please install GitHub CLI from: https://cli.github.com/
    pause
    goto MAIN_MENU
)

echo.
echo [*] Checking GitHub authentication...
gh auth status >nul 2>&1
if errorlevel 1 (
    echo [!] Please authenticate with GitHub:
    gh auth login
)

echo.
echo [*] Creating private GitHub repository: %REPO_NAME%
gh repo create "%REPO_NAME%" --private --source=. --remote=origin --push
if errorlevel 1 (
    echo [ERROR] Failed to create GitHub repository
) else (
    echo [OK] GitHub repository created successfully!
)

pause
goto MAIN_MENU

:COMMIT_PUSH
call :CHECK_GIT
if errorlevel 1 goto MAIN_MENU

echo.
echo [*] Staging files...
git add .

echo [*] Committing changes...
git commit -m "%REPO_NAME%"
if errorlevel 1 (
    echo [!] Nothing to commit or commit failed
)

echo [*] Checking for remote...
git remote get-url origin >nul 2>&1
if errorlevel 1 (
    echo [!] No remote 'origin' configured. Skipping push.
    echo [!] Use option 5 to create a GitHub repository first.
) else (
    echo [*] Pushing to origin...
    for /f "tokens=*" %%b in ('git branch --show-current') do set "current_branch=%%b"
    git push -u origin !current_branch!
    if errorlevel 1 (
        echo [ERROR] Push failed. You may need to set up the remote first.
    ) else (
        echo [OK] Successfully pushed to GitHub!
    )
)

pause
goto MAIN_MENU

:FULL_SETUP
call :CHECK_GIT
if errorlevel 1 goto MAIN_MENU

echo.
echo [*] Starting full setup...
echo.

REM Initialize git
if exist .git (
    echo [!] Git repository already initialized
) else (
    echo [*] Initializing git repository...
    git init
    echo [OK] Git repository initialized
)

REM Update gitignore
echo.
echo [*] Updating .gitignore...
if not exist .gitignore (
    type nul > .gitignore
    echo [OK] Created new .gitignore file
)

set "entries=/deps /dist /bin /build"
for %%e in (%entries%) do (
    findstr /x /c:"%%e" .gitignore >nul 2>&1
    if errorlevel 1 (
        echo %%e >> .gitignore
        echo [OK] Added %%e to .gitignore
    ) else (
        echo [!] %%e already in .gitignore
    )
)

REM Check size
echo.
echo [*] Checking repository size...
powershell -Command "$size = 0; Get-ChildItem -Recurse -File | Where-Object { $_.FullName -notmatch '[\\/]\.git[\\/]' } | ForEach-Object { $size += $_.Length }; $sizeMB = [math]::Round($size / 1MB, 2); Write-Host \"Total size: $sizeMB MB\"; if ($sizeMB -gt 100) { exit 1 } else { exit 0 }"

if errorlevel 1 (
    call :HANDLE_LARGE_REPO
)

REM Ask about GitHub repo
echo.
set /p "create_gh=Do you want to create a GitHub repository? (y/n): "

REM Trim whitespace
set "create_gh=%create_gh: =%"

if /i "%create_gh%"=="y" goto SETUP_CREATE_GH

goto SETUP_NO_GH

:SETUP_CREATE_GH
call :CHECK_GH
if errorlevel 1 goto SETUP_GH_NOT_INSTALLED

gh auth status >nul 2>&1
if errorlevel 1 (
    echo [!] Please authenticate with GitHub:
    gh auth login
)
echo [*] Creating private GitHub repository: %REPO_NAME%
gh repo create "%REPO_NAME%" --private --source=. --remote=origin --push
if errorlevel 1 (
    echo [ERROR] Failed to create GitHub repository
) else (
    echo [OK] GitHub repository created and pushed successfully!
)
goto SETUP_COMPLETE

:SETUP_GH_NOT_INSTALLED
echo [ERROR] gh CLI is not installed. Skipping GitHub creation.
echo [*] Committing and pushing manually...
git add .
git commit -m "%REPO_NAME%"
echo [!] GitHub repository not created. Please set up remote manually.
goto SETUP_COMPLETE

:SETUP_NO_GH
echo [*] Staging files...
git add .
echo [*] Committing changes...
git commit -m "%REPO_NAME%"

git remote get-url origin >nul 2>&1
if not errorlevel 1 (
    echo [*] Pushing to origin...
    for /f "tokens=*" %%b in ('git branch --show-current') do set "current_branch=%%b"
    git push -u origin !current_branch!
) else (
    echo [!] No remote configured. Committed locally only.
)
goto SETUP_COMPLETE

:SETUP_COMPLETE

echo.
echo [OK] Full setup complete!
pause
goto MAIN_MENU

:EXIT
echo.
echo Goodbye!
timeout /t 2 >nul
exit /b 0
