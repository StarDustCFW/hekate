@echo off
set PATH=c:\devkitPro\msys2\usr\bin;%PATH%
:1
echo ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	make -j7
%systemroot%\system32\timeout.exe 13 >nul
goto:1
exit