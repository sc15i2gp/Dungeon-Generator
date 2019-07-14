setlocal
set PATH=%PATH%;C:\MinGW\bin;%~dp0;%~dp0\..\bin;%~dp0\..\build;
set INCLUDE=C:\VulkanSDK\1.1.106.0\Include;%INCLUDE%
cd ..\src
start cmd /k echo Opened cmd
