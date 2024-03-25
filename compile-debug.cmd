@echo off
rem set PATH=c:\devkitPro\msys2\usr\bin;%PATH%
:1
cls
echo ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	make -j20
%systemroot%\system32\timeout.exe 13 >nul
goto:1
exit