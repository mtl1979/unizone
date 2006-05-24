@echo off
if exist %QTDIR%\bin\qt20fix.exe goto QT2
echo Qt 3.x or newer detected.
lupdate unizone.pro
goto FINISH
:QT2
echo Qt 2.x detected.
lupdate qt2\unizone.pro
:FINISH
