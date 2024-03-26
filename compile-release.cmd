@echo off
rem set PATH=c:\devkitPro\msys2\usr\bin;%PATH%
cls
echo ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
make -j30
    
copy output\payload.bin "C:\Users\Krono\Documents\GitHub\StarDustCFWPack\SD_card_root\payload.bin"
copy output\payload.bin "C:\Users\Krono\Documents\GitHub\StarDustCFWPack\SD_card_root\atmosphere\reboot_payload.bin"
copy output\payload.bin "E:\payload.bin"
copy output\payload.bin "E:\atmosphere\reboot_payload.bin"

    
%systemroot%\system32\timeout.exe 113 >nul



exit