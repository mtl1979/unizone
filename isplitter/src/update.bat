@echo off
if not exist %QTDIR%\bin\uic3.exe goto FINISH
echo Qt 4.x or newer detected.
lupdate isplitter.pro
:FINISH
