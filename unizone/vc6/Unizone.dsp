# Microsoft Developer Studio Project File - Name="Unizone" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Unizone - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Unizone.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Unizone.mak" CFG="Unizone - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Unizone - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Unizone - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "Unizone - Win32 English" (based on "Win32 (x86) Application")
!MESSAGE "Unizone - Win32 Portuguese" (based on "Win32 (x86) Application")
!MESSAGE "Unizone - Win32 German" (based on "Win32 (x86) Application")
!MESSAGE "Unizone - Win32 French" (based on "Win32 (x86) Application")
!MESSAGE "Unizone - Win32 Russian" (based on "Win32 (x86) Application")
!MESSAGE "Unizone - Win32 Swedish" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "$(QTDIR)\include" /I "..\src" /I "..\src\muscle" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "QT_THREAD_SUPPORT" /D "QT_DLL" /D "BETA" /D "QT_NO_ASCII_CAST" /D "UNICODE" /YX /FD /c
# SUBTRACT CPP /X /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 unicows.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib shlwapi.lib oleaut32.lib uuid.lib wsock32.lib qt-mt230nc.lib qtmain.lib muscle.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libc" /nodefaultlib:"libcd" /out:"Unizone.exe" /libpath:"$(QTDIR)\lib"
# SUBTRACT LINK32 /pdb:none /incremental:yes

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /GR /GX /ZI /Od /I "$(QTDIR)\include" /I "..\src" /I "..\src\muscle" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "QT_THREAD_SUPPORT" /D "QT_DLL" /D "QT_NO_ASCII_CAST" /D "BETA" /D "UNICODE" /FR /YX /FD /GZ /c
# SUBTRACT CPP /X
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 unicows.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib shlwapi.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib qt-mt230nc.lib qtmain.lib muscled.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"msvcrt" /out:"Unizoned.exe" /pdbtype:sept /libpath:"$(QTDIR)\lib"
# SUBTRACT LINK32 /incremental:no

!ELSEIF  "$(CFG)" == "Unizone - Win32 English"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Unizone___Win32_English"
# PROP BASE Intermediate_Dir "Unizone___Win32_English"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Unizone___Win32_English"
# PROP Intermediate_Dir "Unizone___Win32_English"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GR /GX /O2 /I "$(QTDIR)\include" /I "..\src" /I "..\src\muscle" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "QT_THREAD_SUPPORT" /D "QT_DLL" /D "BETA" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "$(QTDIR)\include" /I "..\src" /I "..\src\muscle" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "QT_THREAD_SUPPORT" /D "QT_DLL" /D "BETA" /D MSG_LANG=1 /D "QT_NO_ASCII_CAST" /D "UNICODE" /YX /FD /c
# SUBTRACT CPP /X /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"Release/Unizone.bsc"
# ADD BSC32 /nologo /o"Release/Unizone.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib muscle.lib wsock32.lib $(QTDIR)\lib\qt-mt230nc.lib $(QTDIR)\lib\qtmain.lib /nologo /subsystem:windows /incremental:yes /machine:I386 /nodefaultlib:"libc" /nodefaultlib:"libcd" /out:"Unizone.exe"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 unicows.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib shlwapi.lib oleaut32.lib uuid.lib wsock32.lib qt-mt230nc.lib qtmain.lib muscle.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libc" /nodefaultlib:"libcd" /out:"Unizone_en.exe" /libpath:"$(QTDIR)\lib"
# SUBTRACT LINK32 /pdb:none /incremental:yes

!ELSEIF  "$(CFG)" == "Unizone - Win32 Portuguese"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Unizone___Win32_Portuguese"
# PROP BASE Intermediate_Dir "Unizone___Win32_Portuguese"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Unizone___Win32_Portuguese"
# PROP Intermediate_Dir "Unizone___Win32_Portuguese"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GR /GX /O2 /I "$(QTDIR)\include" /I "..\src" /I "..\src\muscle" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "QT_THREAD_SUPPORT" /D "QT_DLL" /D "BETA" /D MSG_LANG=1 /D "QT_NO_ASCII_CAST" /D "UNICODE" /YX /FD /c
# SUBTRACT BASE CPP /Fr
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "$(QTDIR)\include" /I "..\src" /I "..\src\muscle" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "QT_THREAD_SUPPORT" /D "QT_DLL" /D "BETA" /D MSG_LANG=351 /D "QT_NO_ASCII_CAST" /D "UNICODE" /YX /FD /c
# SUBTRACT CPP /X /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"Release/Unizone.bsc"
# ADD BSC32 /nologo /o"Release/Unizone.bsc"
LINK32=link.exe
# ADD BASE LINK32 unicows.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib shlwapi.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib muscle.lib wsock32.lib $(QTDIR)\lib\qt-mt230nc.lib $(QTDIR)\lib\qtmain.lib /nologo /subsystem:windows /incremental:yes /machine:I386 /nodefaultlib:"libc" /nodefaultlib:"libcd" /out:"Unizone_en.exe"
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 unicows.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib shlwapi.lib oleaut32.lib uuid.lib wsock32.lib qt-mt230nc.lib qtmain.lib muscle.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libc" /nodefaultlib:"libcd" /out:"Unizone_pt.exe" /libpath:"$(QTDIR)\lib"
# SUBTRACT LINK32 /pdb:none /incremental:yes

!ELSEIF  "$(CFG)" == "Unizone - Win32 German"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Unizone___Win32_German0"
# PROP BASE Intermediate_Dir "Unizone___Win32_German0"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Unizone___Win32_German"
# PROP Intermediate_Dir "Unizone___Win32_German"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O2 /I "$(QTDIR)\include" /I "..\src" /I "..\src\muscle" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "QT_THREAD_SUPPORT" /D "QT_DLL" /D "BETA" /D MSG_LANG=1 /D "QT_NO_ASCII_CAST" /D "UNICODE" /YX /FD /c
# SUBTRACT BASE CPP /X /Fr
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "$(QTDIR)\include" /I "..\src" /I "..\src\muscle" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "QT_THREAD_SUPPORT" /D "QT_DLL" /D "BETA" /D MSG_LANG=49 /D "QT_NO_ASCII_CAST" /D "UNICODE" /YX /FD /c
# SUBTRACT CPP /X /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"Release/Unizone.bsc"
# ADD BSC32 /nologo /o"Release/Unizone.bsc"
LINK32=link.exe
# ADD BASE LINK32 unicows.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib shlwapi.lib oleaut32.lib uuid.lib wsock32.lib qt-mt230nc.lib qtmain.lib muscle.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libc" /nodefaultlib:"libcd" /out:"Unizone_en.exe" /libpath:"$(QTDIR)\lib"
# SUBTRACT BASE LINK32 /pdb:none /incremental:yes
# ADD LINK32 unicows.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib shlwapi.lib oleaut32.lib uuid.lib wsock32.lib qt-mt230nc.lib qtmain.lib muscle.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libc" /nodefaultlib:"libcd" /out:"Unizone_de.exe" /libpath:"$(QTDIR)\lib"
# SUBTRACT LINK32 /pdb:none /incremental:yes

!ELSEIF  "$(CFG)" == "Unizone - Win32 French"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Unizone___Win32_French"
# PROP BASE Intermediate_Dir "Unizone___Win32_French"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Unizone___Win32_French"
# PROP Intermediate_Dir "Unizone___Win32_French"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O2 /I "$(QTDIR)\include" /I "..\src" /I "..\src\muscle" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "QT_THREAD_SUPPORT" /D "QT_DLL" /D "BETA" /D MSG_LANG=49 /D "QT_NO_ASCII_CAST" /D "UNICODE" /YX /FD /c
# SUBTRACT BASE CPP /X /Fr
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "$(QTDIR)\include" /I "..\src" /I "..\src\muscle" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "QT_THREAD_SUPPORT" /D "QT_DLL" /D "BETA" /D MSG_LANG=33 /D "QT_NO_ASCII_CAST" /D "UNICODE" /YX /FD /c
# SUBTRACT CPP /X /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"Release/Unizone.bsc"
# ADD BSC32 /nologo /o"Release/Unizone.bsc"
LINK32=link.exe
# ADD BASE LINK32 unicows.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib shlwapi.lib oleaut32.lib uuid.lib wsock32.lib qt-mt230nc.lib qtmain.lib muscle.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libc" /nodefaultlib:"libcd" /out:"Unizone_de.exe" /libpath:"$(QTDIR)\lib"
# SUBTRACT BASE LINK32 /pdb:none /incremental:yes
# ADD LINK32 unicows.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib shlwapi.lib oleaut32.lib uuid.lib wsock32.lib qt-mt230nc.lib qtmain.lib muscle.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libc" /nodefaultlib:"libcd" /out:"Unizone_fr.exe" /libpath:"$(QTDIR)\lib"
# SUBTRACT LINK32 /pdb:none /incremental:yes

!ELSEIF  "$(CFG)" == "Unizone - Win32 Russian"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Unizone___Win32_Russian"
# PROP BASE Intermediate_Dir "Unizone___Win32_Russian"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Unizone___Win32_Russian"
# PROP Intermediate_Dir "Unizone___Win32_Russian"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O2 /I "$(QTDIR)\include" /I "..\src" /I "..\src\muscle" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "QT_THREAD_SUPPORT" /D "QT_DLL" /D "BETA" /D MSG_LANG=1 /D "QT_NO_ASCII_CAST" /D "UNICODE" /YX /FD /c
# SUBTRACT BASE CPP /X /Fr
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "$(QTDIR)\include" /I "..\src" /I "..\src\muscle" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "QT_THREAD_SUPPORT" /D "QT_DLL" /D "BETA" /D MSG_LANG=7 /D "QT_NO_ASCII_CAST" /D "UNICODE" /Fr /YX /FD /c
# SUBTRACT CPP /X
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"Release/Unizone.bsc"
# ADD BSC32 /nologo /o"Release/Unizone.bsc"
LINK32=link.exe
# ADD BASE LINK32 unicows.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib shlwapi.lib oleaut32.lib uuid.lib wsock32.lib qt-mt230nc.lib qtmain.lib muscle.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libc" /nodefaultlib:"libcd" /out:"Unizone_en.exe" /libpath:"$(QTDIR)\lib"
# SUBTRACT BASE LINK32 /pdb:none /incremental:yes
# ADD LINK32 unicows.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib shlwapi.lib oleaut32.lib uuid.lib wsock32.lib qt-mt230nc.lib qtmain.lib muscle.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libc" /nodefaultlib:"libcd" /out:"Unizone_ru.exe" /libpath:"$(QTDIR)\lib"
# SUBTRACT LINK32 /pdb:none /incremental:yes

!ELSEIF  "$(CFG)" == "Unizone - Win32 Swedish"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Unizone___Win32_Swedish"
# PROP BASE Intermediate_Dir "Unizone___Win32_Swedish"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Unizone___Win32_Swedish"
# PROP Intermediate_Dir "Unizone___Win32_Swedish"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O2 /I "$(QTDIR)\include" /I "..\src" /I "..\src\muscle" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "QT_THREAD_SUPPORT" /D "QT_DLL" /D "BETA" /D MSG_LANG=1 /D "QT_NO_ASCII_CAST" /D "UNICODE" /YX /FD /c
# SUBTRACT BASE CPP /X /Fr
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "$(QTDIR)\include" /I "..\src" /I "..\src\muscle" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "QT_THREAD_SUPPORT" /D "QT_DLL" /D "BETA" /D MSG_LANG=46 /D "QT_NO_ASCII_CAST" /D "UNICODE" /YX /FD /c
# SUBTRACT CPP /X /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"Release/Unizone.bsc"
# ADD BSC32 /nologo /o"Release/Unizone.bsc"
LINK32=link.exe
# ADD BASE LINK32 unicows.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib shlwapi.lib oleaut32.lib uuid.lib wsock32.lib qt-mt230nc.lib qtmain.lib muscle.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libc" /nodefaultlib:"libcd" /out:"Unizone_en.exe" /libpath:"$(QTDIR)\lib"
# SUBTRACT BASE LINK32 /pdb:none /incremental:yes
# ADD LINK32 unicows.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib shlwapi.lib oleaut32.lib uuid.lib wsock32.lib qt-mt230nc.lib qtmain.lib muscle.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libc" /nodefaultlib:"libcd" /out:"Unizone_se.exe" /libpath:"$(QTDIR)\lib"
# SUBTRACT LINK32 /pdb:none /incremental:yes

!ENDIF 

# Begin Target

# Name "Unizone - Win32 Release"
# Name "Unizone - Win32 Debug"
# Name "Unizone - Win32 English"
# Name "Unizone - Win32 Portuguese"
# Name "Unizone - Win32 German"
# Name "Unizone - Win32 French"
# Name "Unizone - Win32 Russian"
# Name "Unizone - Win32 Swedish"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\aboutdlg.cpp
# End Source File
# Begin Source File

SOURCE=..\src\aboutdlgimpl.cpp
# End Source File
# Begin Source File

SOURCE=..\src\accept.cpp
# End Source File
# Begin Source File

SOURCE=..\src\botitem.cpp
# End Source File
# Begin Source File

SOURCE=..\src\chattext.cpp
# End Source File
# Begin Source File

SOURCE=..\src\checkgroup.cpp
# End Source File
# Begin Source File

SOURCE=..\src\colors.cpp
# End Source File
# Begin Source File

SOURCE=..\src\combo.cpp
# End Source File
# Begin Source File

SOURCE=..\src\downloadimpl.cpp
# End Source File
# Begin Source File

SOURCE=..\src\downloadthread.cpp
# End Source File
# Begin Source File

SOURCE=..\src\fileinfo.cpp
# End Source File
# Begin Source File

SOURCE=..\src\filethread.cpp
# End Source File
# Begin Source File

SOURCE=..\src\formatting.cpp
# End Source File
# Begin Source File

SOURCE=..\src\genericthread.cpp
# End Source File
# Begin Source File

SOURCE=..\src\gotourl.cpp
# End Source File
# Begin Source File

SOURCE=..\src\icon.rc
# End Source File
# Begin Source File

SOURCE=..\src\Log.cpp
# End Source File
# Begin Source File

SOURCE=..\src\looper.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main.cpp
# End Source File
# Begin Source File

SOURCE=..\src\md5.cpp
# End Source File
# Begin Source File

SOURCE=..\src\menubar.cpp
# End Source File
# Begin Source File

SOURCE=..\src\moc_aboutdlg.cpp
# End Source File
# Begin Source File

SOURCE=..\src\moc_aboutdlgimpl.cpp
# End Source File
# Begin Source File

SOURCE=..\src\moc_chattext.cpp
# End Source File
# Begin Source File

SOURCE=..\src\moc_combo.cpp
# End Source File
# Begin Source File

SOURCE=..\src\moc_downloadimpl.cpp
# End Source File
# Begin Source File

SOURCE=..\src\moc_genericthread.cpp
# End Source File
# Begin Source File

SOURCE=..\src\moc_htmlview.cpp
# End Source File
# Begin Source File

SOURCE=..\src\moc_netclient.cpp
# End Source File
# Begin Source File

SOURCE=..\src\moc_prefs.cpp
# End Source File
# Begin Source File

SOURCE=..\src\moc_prefsimpl.cpp
# End Source File
# Begin Source File

SOURCE=..\src\moc_privatewindow.cpp
# End Source File
# Begin Source File

SOURCE=..\src\moc_privatewindowimpl.cpp
# End Source File
# Begin Source File

SOURCE=..\src\moc_search.cpp
# End Source File
# Begin Source File

SOURCE=..\src\moc_winsharewindow.cpp
# End Source File
# Begin Source File

SOURCE=..\src\netclient.cpp
# End Source File
# Begin Source File

SOURCE=..\src\nicklist.cpp
# End Source File
# Begin Source File

SOURCE=..\src\platform.cpp
# End Source File
# Begin Source File

SOURCE=..\src\prefs.cpp
# End Source File
# Begin Source File

SOURCE=..\src\prefsimpl.cpp
# End Source File
# Begin Source File

SOURCE=..\src\privatewindow.cpp
# End Source File
# Begin Source File

SOURCE=..\src\privatewindowimpl.cpp
# End Source File
# Begin Source File

SOURCE=..\src\search.cpp
# End Source File
# Begin Source File

SOURCE=..\src\searchitem.cpp
# End Source File
# Begin Source File

SOURCE=..\src\settings.cpp
# End Source File
# Begin Source File

SOURCE=..\src\textevent.cpp
# End Source File
# Begin Source File

SOURCE=..\src\transferitem.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ulistview.cpp
# End Source File
# Begin Source File

SOURCE=..\src\uploadthread.cpp
# End Source File
# Begin Source File

SOURCE=..\src\user.cpp
# End Source File
# Begin Source File

SOURCE=..\src\userlistitem.cpp
# End Source File
# Begin Source File

SOURCE=..\src\winshare_network.cpp
# End Source File
# Begin Source File

SOURCE=..\src\winshare_parsing.cpp
# End Source File
# Begin Source File

SOURCE=..\src\winshare_slots.cpp
# End Source File
# Begin Source File

SOURCE=..\src\winsharewindow.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\src\aboutdlg.h

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\aboutdlg.h
InputName=aboutdlg

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\aboutdlg.h
InputName=aboutdlg

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 English"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\aboutdlg.h
InputName=aboutdlg

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Portuguese"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\aboutdlg.h
InputName=aboutdlg

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 German"

# PROP BASE Ignore_Default_Tool 1
# PROP Intermediate_Dir "Unizone___Win32_German"
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\aboutdlg.h
InputName=aboutdlg

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 French"

# PROP BASE Intermediate_Dir "Unizone___Win32_German"
# PROP BASE Ignore_Default_Tool 1
# PROP Intermediate_Dir "Unizone___Win32_German"
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\aboutdlg.h
InputName=aboutdlg

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Russian"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\aboutdlg.h
InputName=aboutdlg

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Swedish"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\aboutdlg.h
InputName=aboutdlg

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\aboutdlgimpl.h

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\aboutdlgimpl.h
InputName=aboutdlgimpl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\aboutdlgimpl.h
InputName=aboutdlgimpl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 English"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\aboutdlgimpl.h
InputName=aboutdlgimpl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Portuguese"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\aboutdlgimpl.h
InputName=aboutdlgimpl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 German"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\aboutdlgimpl.h
InputName=aboutdlgimpl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 French"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\aboutdlgimpl.h
InputName=aboutdlgimpl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Russian"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\aboutdlgimpl.h
InputName=aboutdlgimpl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Swedish"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\aboutdlgimpl.h
InputName=aboutdlgimpl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\accept.h
# End Source File
# Begin Source File

SOURCE=..\src\botitem.h
# End Source File
# Begin Source File

SOURCE=..\src\build.h
# End Source File
# Begin Source File

SOURCE=..\src\chattext.h

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\chattext.h
InputName=chattext

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\chattext.h
InputName=chattext

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 English"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\chattext.h
InputName=chattext

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Portuguese"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\chattext.h
InputName=chattext

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 German"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\chattext.h
InputName=chattext

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 French"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\chattext.h
InputName=chattext

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Russian"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\chattext.h
InputName=chattext

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Swedish"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\chattext.h
InputName=chattext

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\checkgroup.h
# End Source File
# Begin Source File

SOURCE=..\src\colors.h
# End Source File
# Begin Source File

SOURCE=..\src\combo.h

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\combo.h
InputName=combo

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\combo.h
InputName=combo

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 English"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\combo.h
InputName=combo

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Portuguese"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\combo.h
InputName=combo

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 German"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\combo.h
InputName=combo

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 French"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\combo.h
InputName=combo

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Russian"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\combo.h
InputName=combo

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Swedish"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\combo.h
InputName=combo

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\debugimpl.h
# End Source File
# Begin Source File

SOURCE=..\src\downloadimpl.h

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\downloadimpl.h
InputName=downloadimpl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\downloadimpl.h
InputName=downloadimpl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 English"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\downloadimpl.h
InputName=downloadimpl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Portuguese"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\downloadimpl.h
InputName=downloadimpl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 German"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\downloadimpl.h
InputName=downloadimpl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 French"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\downloadimpl.h
InputName=downloadimpl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Russian"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\downloadimpl.h
InputName=downloadimpl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Swedish"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\downloadimpl.h
InputName=downloadimpl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\downloadthread.h
# End Source File
# Begin Source File

SOURCE=..\src\lang\english.h
# End Source File
# Begin Source File

SOURCE=..\src\fileinfo.h
# End Source File
# Begin Source File

SOURCE=..\src\filethread.h
# End Source File
# Begin Source File

SOURCE=..\src\lang\finnish.h
# End Source File
# Begin Source File

SOURCE=..\src\formatting.h
# End Source File
# Begin Source File

SOURCE=..\src\lang\french.h
# End Source File
# Begin Source File

SOURCE=..\src\genericthread.h

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\genericthread.h
InputName=genericthread

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\genericthread.h
InputName=genericthread

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 English"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\genericthread.h
InputName=genericthread

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Portuguese"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\genericthread.h
InputName=genericthread

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 German"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\genericthread.h
InputName=genericthread

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 French"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\genericthread.h
InputName=genericthread

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Russian"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\genericthread.h
InputName=genericthread

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Swedish"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\genericthread.h
InputName=genericthread

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\lang\german.h
# End Source File
# Begin Source File

SOURCE=..\src\global.h
# End Source File
# Begin Source File

SOURCE=..\src\gotourl.h
# End Source File
# Begin Source File

SOURCE=..\src\htmlview.h

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\htmlview.h
InputName=htmlview

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\htmlview.h
InputName=htmlview

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 English"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\htmlview.h
InputName=htmlview

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Portuguese"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\htmlview.h
InputName=htmlview

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 German"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\htmlview.h
InputName=htmlview

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 French"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\htmlview.h
InputName=htmlview

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Russian"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\htmlview.h
InputName=htmlview

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Swedish"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\htmlview.h
InputName=htmlview

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\lang.h
# End Source File
# Begin Source File

SOURCE=..\src\Log.h
# End Source File
# Begin Source File

SOURCE=..\src\looper.h
# End Source File
# Begin Source File

SOURCE=..\src\md5.h
# End Source File
# Begin Source File

SOURCE=..\src\menubar.h
# End Source File
# Begin Source File

SOURCE=..\src\netclient.h

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\netclient.h
InputName=netclient

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\netclient.h
InputName=netclient

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 English"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\netclient.h
InputName=netclient

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Portuguese"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\netclient.h
InputName=netclient

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 German"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\netclient.h
InputName=netclient

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 French"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\netclient.h
InputName=netclient

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Russian"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\netclient.h
InputName=netclient

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Swedish"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\netclient.h
InputName=netclient

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\nicklist.h
# End Source File
# Begin Source File

SOURCE=..\src\platform.h
# End Source File
# Begin Source File

SOURCE=..\src\lang\portuguese.h
# End Source File
# Begin Source File

SOURCE=..\src\prefs.h

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\prefs.h
InputName=prefs

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\prefs.h
InputName=prefs

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 English"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\prefs.h
InputName=prefs

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Portuguese"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\prefs.h
InputName=prefs

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 German"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\prefs.h
InputName=prefs

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 French"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\prefs.h
InputName=prefs

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Russian"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\prefs.h
InputName=prefs

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Swedish"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\prefs.h
InputName=prefs

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\prefsimpl.h

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\prefsimpl.h
InputName=prefsimpl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\prefsimpl.h
InputName=prefsimpl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 English"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\prefsimpl.h
InputName=prefsimpl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Portuguese"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\prefsimpl.h
InputName=prefsimpl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 German"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\prefsimpl.h
InputName=prefsimpl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 French"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\prefsimpl.h
InputName=prefsimpl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Russian"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\prefsimpl.h
InputName=prefsimpl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Swedish"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\prefsimpl.h
InputName=prefsimpl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\privatewindow.h

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\privatewindow.h
InputName=privatewindow

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\privatewindow.h
InputName=privatewindow

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 English"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\privatewindow.h
InputName=privatewindow

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Portuguese"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\privatewindow.h
InputName=privatewindow

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 German"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\privatewindow.h
InputName=privatewindow

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 French"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\privatewindow.h
InputName=privatewindow

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Russian"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\privatewindow.h
InputName=privatewindow

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Swedish"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\privatewindow.h
InputName=privatewindow

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\privatewindowimpl.h

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\privatewindowimpl.h
InputName=privatewindowimpl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\privatewindowimpl.h
InputName=privatewindowimpl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 English"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\privatewindowimpl.h
InputName=privatewindowimpl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Portuguese"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\privatewindowimpl.h
InputName=privatewindowimpl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 German"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\privatewindowimpl.h
InputName=privatewindowimpl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 French"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\privatewindowimpl.h
InputName=privatewindowimpl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Russian"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\privatewindowimpl.h
InputName=privatewindowimpl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Swedish"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\privatewindowimpl.h
InputName=privatewindowimpl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\resource.h
# End Source File
# Begin Source File

SOURCE=..\src\lang\russian.h
# End Source File
# Begin Source File

SOURCE=..\src\search.h

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\search.h
InputName=search

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\search.h
InputName=search

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 English"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\search.h
InputName=search

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Portuguese"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\search.h
InputName=search

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 German"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\search.h
InputName=search

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 French"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\search.h
InputName=search

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Russian"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\search.h
InputName=search

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Swedish"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\search.h
InputName=search

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\searchitem.h
# End Source File
# Begin Source File

SOURCE=..\src\settings.h
# End Source File
# Begin Source File

SOURCE=..\src\lang\swedish.h
# End Source File
# Begin Source File

SOURCE=..\src\textevent.h
# End Source File
# Begin Source File

SOURCE=..\src\tokenizer.h
# End Source File
# Begin Source File

SOURCE=..\src\transferitem.h
# End Source File
# Begin Source File

SOURCE=..\src\ulistview.h
# End Source File
# Begin Source File

SOURCE=..\src\uploadthread.h
# End Source File
# Begin Source File

SOURCE=..\src\user.h
# End Source File
# Begin Source File

SOURCE=..\src\userlistitem.h
# End Source File
# Begin Source File

SOURCE=..\src\version.h
# End Source File
# Begin Source File

SOURCE=..\src\wgenericevent.h
# End Source File
# Begin Source File

SOURCE=..\src\winsharewindow.h

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\winsharewindow.h
InputName=winsharewindow

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\winsharewindow.h
InputName=winsharewindow

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 English"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\winsharewindow.h
InputName=winsharewindow

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Portuguese"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\winsharewindow.h
InputName=winsharewindow

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 German"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\winsharewindow.h
InputName=winsharewindow

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 French"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\winsharewindow.h
InputName=winsharewindow

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Russian"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\winsharewindow.h
InputName=winsharewindow

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Swedish"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\Unizone\src
InputPath=..\src\winsharewindow.h
InputName=winsharewindow

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\wpwevent.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\src\unizone.ico
# End Source File
# End Group
# Begin Group "UI files"

# PROP Default_Filter "ui"
# Begin Source File

SOURCE=..\src\aboutdlg.ui

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\Unizone\src
InputPath=..\src\aboutdlg.ui
InputName=aboutdlg

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(InputDir)\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\$(InputName).cpp \
	

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\Unizone\src
InputPath=..\src\aboutdlg.ui
InputName=aboutdlg

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(InputDir)\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\$(InputName).cpp \
	

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 English"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\Unizone\src
InputPath=..\src\aboutdlg.ui
InputName=aboutdlg

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(InputDir)\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\$(InputName).cpp \
	

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Portuguese"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\Unizone\src
InputPath=..\src\aboutdlg.ui
InputName=aboutdlg

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(InputDir)\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\$(InputName).cpp \
	

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 German"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\Unizone\src
InputPath=..\src\aboutdlg.ui
InputName=aboutdlg

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(InputDir)\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\$(InputName).cpp \
	

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 French"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\Unizone\src
InputPath=..\src\aboutdlg.ui
InputName=aboutdlg

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(InputDir)\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\$(InputName).cpp \
	

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Russian"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\Unizone\src
InputPath=..\src\aboutdlg.ui
InputName=aboutdlg

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(InputDir)\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\$(InputName).cpp \
	

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Swedish"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\Unizone\src
InputPath=..\src\aboutdlg.ui
InputName=aboutdlg

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(InputDir)\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\$(InputName).cpp \
	

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\prefs.ui

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\Unizone\src
InputPath=..\src\prefs.ui
InputName=prefs

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(InputDir)\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\$(InputName).cpp \
	

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\Unizone\src
InputPath=..\src\prefs.ui
InputName=prefs

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(InputDir)\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\$(InputName).cpp \
	

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 English"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\Unizone\src
InputPath=..\src\prefs.ui
InputName=prefs

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(InputDir)\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\$(InputName).cpp \
	

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Portuguese"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\Unizone\src
InputPath=..\src\prefs.ui
InputName=prefs

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(InputDir)\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\$(InputName).cpp \
	

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 German"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\Unizone\src
InputPath=..\src\prefs.ui
InputName=prefs

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(InputDir)\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\$(InputName).cpp \
	

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 French"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\Unizone\src
InputPath=..\src\prefs.ui
InputName=prefs

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(InputDir)\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\$(InputName).cpp \
	

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Russian"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\Unizone\src
InputPath=..\src\prefs.ui
InputName=prefs

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(InputDir)\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\$(InputName).cpp \
	

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Swedish"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\Unizone\src
InputPath=..\src\prefs.ui
InputName=prefs

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(InputDir)\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\$(InputName).cpp \
	

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\privatewindow.ui

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\Unizone\src
InputPath=..\src\privatewindow.ui
InputName=privatewindow

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(InputDir)\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\$(InputName).cpp \
	

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\Unizone\src
InputPath=..\src\privatewindow.ui
InputName=privatewindow

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(InputDir)\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\$(InputName).cpp \
	

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 English"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\Unizone\src
InputPath=..\src\privatewindow.ui
InputName=privatewindow

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(InputDir)\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\$(InputName).cpp \
	

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Portuguese"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\Unizone\src
InputPath=..\src\privatewindow.ui
InputName=privatewindow

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(InputDir)\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\$(InputName).cpp \
	

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 German"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\Unizone\src
InputPath=..\src\privatewindow.ui
InputName=privatewindow

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(InputDir)\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\$(InputName).cpp \
	

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 French"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\Unizone\src
InputPath=..\src\privatewindow.ui
InputName=privatewindow

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(InputDir)\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\$(InputName).cpp \
	

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Russian"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\Unizone\src
InputPath=..\src\privatewindow.ui
InputName=privatewindow

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(InputDir)\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\$(InputName).cpp \
	

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Swedish"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\Unizone\src
InputPath=..\src\privatewindow.ui
InputName=privatewindow

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(InputDir)\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\$(InputName).cpp \
	

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
