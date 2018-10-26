set ERRORLEVEL=0
cd bin
if %ERRORLEVEL% NEQ 0 (
goto FAIL
)
copy SubAuth.dll %SYSTEMROOT%\System32
if %ERRORLEVEL% NEQ 0 (
goto FAIL
)
reg import install.reg
if %ERRORLEVEL% NEQ 0 (
goto FAIL
)
echo Installed.Press Any Key To Configure.
pause
Config
exit
:FAIL
echo Fail.