@echo off
rem set PATH=c:\devkitPro\msys2\usr\bin;%PATH%
:1
cls
echo ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	make -j30
%systemroot%\system32\timeout.exe 113 >nul
goto:1
exit