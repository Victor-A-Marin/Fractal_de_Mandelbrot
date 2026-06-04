@echo off
SETLOCAL EnableDelayedExpansion

:: 1. Choose your MSYS2 environment (MINGW64 or UCRT64)
set MSYSTEM=MINGW64

:: 2. Convert current Windows path backslashes (\) to forward slashes (/) for Bash
set "CURRENT_DIR=%cd:\=/%"

:: 3. Grab arguments from the command line, or use defaults if empty
set THREADS=%1
if "%THREADS%"=="" set THREADS=4

set ITERATIONS=%2
if "%ITERATIONS%"=="" set ITERATIONS=20000

set TASK_SIZE=%3
if "%TASK_SIZE%"=="" set TASK_SIZE=32

echo ===================================================
echo  [MSYS2 Automatization] Environment: %MSYSTEM%
echo ===================================================
echo [1/2] Compiling source files...

:: Call MSYS2 Bash, jump to the project directory, and compile
C:\msys64\usr\bin\bash.exe --login -c "cd '%CURRENT_DIR%' && gcc main.c task_buffer.c result_buffer.c threads.c producer.c -o mandelbrot_sdl -lmingw32 -lSDL2main -lSDL2 -lpthread -lm"

:: Check if the compilation was successful (Exit Code 0)
if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] Compilation failed! Check the errors above.
    pause
    exit /b %ERRORLEVEL%
)

echo [2/2] Running Mandelbrot (Threads: %THREADS%, Iterations: %ITERATIONS%, Block Size: %TASK_SIZE%)...
echo.

:: Launch the compiled binary with the arguments
C:\msys64\usr\bin\bash.exe --login -c "cd '%CURRENT_DIR%' && ./mandelbrot_sdl %THREADS% %ITERATIONS% %TASK_SIZE%"

echo.
echo Execution finished.
pause