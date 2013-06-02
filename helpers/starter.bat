@echo off

set APPNAME=emptyExample
REM set APPNAME=emptyExample_DEBUG

:start
echo Trying to start application...
cmd /C %APPNAME%

if %ERRORLEVEL% EQU 6 (
	echo failed, retrying...
	goto start
)

echo Success!

pause

REM exit 0
