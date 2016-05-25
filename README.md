Travis CI: [![build status](https://api.travis-ci.org/mtl1979/unizone.svg)](https://travis-ci.org/mtl1979/unizone/)

Unizone - Unicode Chat and File Sharing Client

Requirements
------------

* Windows or Linux with Qt 4.8.7 development tools
* gcc or Microsoft Visual Studio 2015

Build
-----
1. run "qmake -r" in root directory for gcc or nmake, "qmake -r -tp vc" for Visual Studio
2. run make under Linux, nmake under Windows using command prompt, or open "unizone.sln" inside Visual Studio

Running
-------

Unizone should start without extra steps on Linux.

Under Windows you need to make sure Qt libraries are inside same directory as Unizone.exe and all plugins are copied inside "plugins\imageformats" subdirectory under the same directory as Unizone.exe.

Languages
---------

Supported languages are defined inside src/src.pro, each language uses two character code. Once you add new language, you need to run "lupdate src.pro" inside "src" directory so new template file(s) will be created. When you add translations to the template file, you need to rebuild Unizone so template file is compiled as language file.