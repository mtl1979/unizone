# Microsoft Developer Studio Project File - Name="Unizone" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Unizone - Win32 Debug ANSI
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Unizone.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Unizone.mak" CFG="Unizone - Win32 Debug ANSI"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Unizone - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Unizone - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "Unizone - Win32 Debug ANSI" (based on "Win32 (x86) Application")
!MESSAGE "Unizone - Win32 Release ANSI" (based on "Win32 (x86) Application")
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
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "$(QTDIR)\include" /I "..\..\src" /I "..\..\src\qt2" /I "..\..\src\muscle" /I "..\..\src\muscle\regex\regex" /I "..\..\libjpeg" /D "NDEBUG" /D "_MBCS" /D "UNICODE" /D "MUSCLE_USE_X86_INLINE_ASSEMBLY" /D "WIN32" /D "_WINDOWS" /D "QT_THREAD_SUPPORT" /D "QT_DLL" /D "BETA" /D "QT_NO_ASCII_CAST" /D "MUSCLE_ENABLE_ZLIB_ENCODING" /D "QJPEG_STATIC" /Fr /YX /FD /c
# SUBTRACT CPP /X
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib shlwapi.lib oleaut32.lib uuid.lib wsock32.lib winmm.lib qt-mt230nc.lib qtmain.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libc" /nodefaultlib:"libcd" /out:"Unizone.exe" /libpath:"$(QTDIR)\lib" /libpath:"..\..\libjpeg\Release" /libpath:"muscle___Win32_Release" /libpath:"regex___Win32_Release" /libpath:"zlib___Win32_Release"
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
# ADD CPP /nologo /MDd /W3 /GR /GX /ZI /Od /I "$(QTDIR)\include" /I "..\..\src" /I "..\..\src\qt2" /I "..\..\src\muscle" /I "..\..\src\muscle\regex\regex" /I "..\..\libjpeg" /D "_DEBUG" /D "_MBCS" /D "UNICODE" /D "DISABLE_OBJECT_POOLING" /D "WIN32" /D "_WINDOWS" /D "QT_THREAD_SUPPORT" /D "QT_DLL" /D "BETA" /D "QT_NO_ASCII_CAST" /D "MUSCLE_ENABLE_ZLIB_ENCODING" /D "QJPEG_STATIC" /FR /FD /GZ /c
# SUBTRACT CPP /X /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"Debug/Unizoned.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 winspool.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib shlwapi.lib oleaut32.lib uuid.lib wsock32.lib winmm.lib qt-mt230nc.lib qtmain.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"msvcrt" /out:"Unizoned.exe" /pdbtype:sept /libpath:"$(QTDIR)\lib" /libpath:"..\..\libjpeg\Debug" /libpath:"muscle___Win32_Debug" /libpath:"regex___Win32_Debug" /libpath:"zlib___Win32_Debug"
# SUBTRACT LINK32 /incremental:no

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Unizone___Win32_Debug_ANSI"
# PROP BASE Intermediate_Dir "Unizone___Win32_Debug_ANSI"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_ANSI"
# PROP Intermediate_Dir "Debug_ANSI"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /GR /GX /ZI /Od /I "$(QTDIR)\include" /I "..\..\src" /I "..\..\src\muscle" /I "..\..\libjpeg" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "QT_THREAD_SUPPORT" /D "QT_DLL" /D "QT_NO_ASCII_CAST" /D "BETA" /D "UNICODE" /D "MUSCLE_ENABLE_ZLIB_ENCODING" /D "ZLIB_USEDLL" /D "REGEX_USEDLL" /D "DISABLE_OBJECT_POOLING" /FR /FD /GZ /c
# SUBTRACT BASE CPP /X /YX
# ADD CPP /nologo /MDd /W3 /GR /GX /ZI /Od /I "$(QTDIR)\include" /I "..\..\src" /I "..\..\src\qt2" /I "..\..\src\muscle" /I "..\..\src\muscle\regex\regex" /I "..\..\libjpeg" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "QT_THREAD_SUPPORT" /D "QT_DLL" /D "BETA" /D "QT_NO_ASCII_CAST" /D "MUSCLE_ENABLE_ZLIB_ENCODING" /D "QJPEG_STATIC" /FR /FD /GZ /c
# SUBTRACT CPP /X /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo /o"Debug/Unizoned.bsc"
# ADD BSC32 /nologo /o"Debug_ANSI/Unizonead.bsc"
LINK32=link.exe
# ADD BASE LINK32 winspool.lib odbc32.lib odbccp32.lib muscled.lib zlibd.lib regexd.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib shlwapi.lib oleaut32.lib uuid.lib wsock32.lib winmm.lib qt-mt230nc.lib qtmain.lib qjpegd.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"msvcrt" /out:"Unizoned.exe" /pdbtype:sept /libpath:"$(QTDIR)\lib" /libpath:"..\..\libjpeg\Debug"
# SUBTRACT BASE LINK32 /incremental:no
# ADD LINK32 winspool.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib shlwapi.lib oleaut32.lib uuid.lib wsock32.lib winmm.lib qt-mt230nc.lib qtmain.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"msvcrt" /out:"Unizonead.exe" /pdbtype:sept /libpath:"$(QTDIR)\lib" /libpath:"..\..\libjpeg\Debug_ANSI" /libpath:"muscle___Win32_Debug_ANSI" /libpath:"regex___Win32_Debug_ANSI" /libpath:"zlib___Win32_Debug_ANSI"
# SUBTRACT LINK32 /incremental:no

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Unizone___Win32_Release_ANSI"
# PROP BASE Intermediate_Dir "Unizone___Win32_Release_ANSI"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_ANSI"
# PROP Intermediate_Dir "Release_ANSI"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O2 /I "$(QTDIR)\include" /I "..\..\src" /I "..\..\src\muscle" /I "..\..\libjpeg" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "QT_THREAD_SUPPORT" /D "QT_DLL" /D "BETA" /D "QT_NO_ASCII_CAST" /D "UNICODE" /D "MUSCLE_ENABLE_ZLIB_ENCODING" /D "ZLIB_USEDLL" /D "REGEX_USEDLL" /D "DISABLE_TUNNELING" /Fr /YX /FD /c
# SUBTRACT BASE CPP /X
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "$(QTDIR)\include" /I "..\..\src" /I "..\..\src\qt2" /I "..\..\src\muscle" /I "..\..\src\muscle\regex\regex" /I "..\..\libjpeg" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "QT_THREAD_SUPPORT" /D "QT_DLL" /D "BETA" /D "QT_NO_ASCII_CAST" /D "MUSCLE_ENABLE_ZLIB_ENCODING" /D "QJPEG_STATIC" /Fr /YX /FD /c
# SUBTRACT CPP /X
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"Release_ANSI/Unizonea.bsc"
LINK32=link.exe
# ADD BASE LINK32 muscle.lib zlib.lib regex.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib shlwapi.lib oleaut32.lib uuid.lib wsock32.lib winmm.lib qt-mt230nc.lib qtmain.lib qjpeg.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libc" /nodefaultlib:"libcd" /out:"Unizone.exe" /libpath:"$(QTDIR)\lib" /libpath:"..\libjpeg\Release"
# SUBTRACT BASE LINK32 /pdb:none /incremental:yes
# ADD LINK32 kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib shlwapi.lib oleaut32.lib uuid.lib wsock32.lib winmm.lib qt-mt230nc.lib qtmain.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"libc" /nodefaultlib:"libcd" /out:"Unizonea.exe" /libpath:"$(QTDIR)\lib" /libpath:"..\libjpeg\Release_ANSI" /libpath:"muscle___Win32_Release_ANSI" /libpath:"regex___Win32_Release_ANSI" /libpath:"zlib___Win32_Release_ANSI"
# SUBTRACT LINK32 /pdb:none /incremental:yes

!ENDIF 

# Begin Target

# Name "Unizone - Win32 Release"
# Name "Unizone - Win32 Debug"
# Name "Unizone - Win32 Debug ANSI"
# Name "Unizone - Win32 Release ANSI"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\src\aboutdlgimpl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\acronymclient.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\botitem.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\channelimpl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\channelinfo.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\channels.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\chatevent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\chattext.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\chatwindow.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\combo.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\debugimpl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\downloadimpl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\downloadqueue.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\downloadthread.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\downloadworker.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\events.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\fileinfo.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\filethread.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\formatting.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\htmlview.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\listthread.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\Log.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\main.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\md5.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\menubar.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\netclient.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\nicklist.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\parser.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\picviewerimpl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\platform.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\prefsimpl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\privatewindowimpl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\resolver.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\resolverthread.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\scanprogressimpl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\search.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\searchitem.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\serverclient.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\settings.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\textevent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\titanic.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\transferitem.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\uenv.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\ulistview.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\updateclient.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\uploadimpl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\uploadthread.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\user.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\userlistitem.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\util.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\version.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\wcrypt.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\werrorevent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\windows\wfile.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\winshare_lists.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\winshare_network.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\winshare_parsing.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\winshare_slots.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\winsharewindow.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\wmessageevent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\wpwevent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\wstatusbar.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\wstring.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\wsystemevent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\windows\wutil_msvc.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\wwarningevent.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\src\aboutdlgimpl.h

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\aboutdlgimpl.h
InputName=aboutdlgimpl

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\aboutdlgimpl.h
InputName=aboutdlgimpl

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\aboutdlgimpl.h
InputName=aboutdlgimpl

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\aboutdlgimpl.h
InputName=aboutdlgimpl

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\acronymclient.h

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\acronymclient.h
InputName=acronymclient

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\acronymclient.h
InputName=acronymclient

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\acronymclient.h
InputName=acronymclient

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\acronymclient.h
InputName=acronymclient

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\botitem.h
# End Source File
# Begin Source File

SOURCE=..\..\src\channelimpl.h

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\channelimpl.h
InputName=channelimpl

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\channelimpl.h
InputName=channelimpl

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\channelimpl.h
InputName=channelimpl

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\channelimpl.h
InputName=channelimpl

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\channelinfo.h
# End Source File
# Begin Source File

SOURCE=..\..\src\channels.h

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\channels.h
InputName=channels

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\channels.h
InputName=channels

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\channels.h
InputName=channels

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\channels.h
InputName=channels

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\chatevent.h
# End Source File
# Begin Source File

SOURCE=..\..\src\chattext.h

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\chattext.h
InputName=chattext

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\chattext.h
InputName=chattext

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\chattext.h
InputName=chattext

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\chattext.h
InputName=chattext

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\chatwindow.h
# End Source File
# Begin Source File

SOURCE=..\..\src\colors.h
# End Source File
# Begin Source File

SOURCE=..\..\src\combo.h

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\combo.h
InputName=combo

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\combo.h
InputName=combo

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\combo.h
InputName=combo

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\combo.h
InputName=combo

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\debugimpl.h
# End Source File
# Begin Source File

SOURCE=..\..\src\downloadimpl.h

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\downloadimpl.h
InputName=downloadimpl

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\downloadimpl.h
InputName=downloadimpl

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\downloadimpl.h
InputName=downloadimpl

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\downloadimpl.h
InputName=downloadimpl

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\downloadqueue.h
# End Source File
# Begin Source File

SOURCE=..\..\src\downloadthread.h

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\downloadthread.h
InputName=downloadthread

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\downloadthread.h
InputName=downloadthread

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\downloadthread.h
InputName=downloadthread

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\downloadthread.h
InputName=downloadthread

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\downloadworker.h
# End Source File
# Begin Source File

SOURCE=..\..\src\events.h
# End Source File
# Begin Source File

SOURCE=..\..\src\fileinfo.h
# End Source File
# Begin Source File

SOURCE=..\..\src\filethread.h

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\filethread.h
InputName=filethread

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\filethread.h
InputName=filethread

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\filethread.h
InputName=filethread

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\filethread.h
InputName=filethread

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\formatting.h

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\formatting.h
InputName=formatting

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\formatting.h
InputName=formatting

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\formatting.h
InputName=formatting

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\formatting.h
InputName=formatting

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\global.h
# End Source File
# Begin Source File

SOURCE=..\..\src\gotourl.h
# End Source File
# Begin Source File

SOURCE=..\..\src\htmlview.h

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\htmlview.h
InputName=htmlview

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\htmlview.h
InputName=htmlview

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\htmlview.h
InputName=htmlview

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\htmlview.h
InputName=htmlview

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\listthread.h

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\listthread.h
InputName=listthread

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\listthread.h
InputName=listthread

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\listthread.h
InputName=listthread

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\listthread.h
InputName=listthread

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\Log.h
# End Source File
# Begin Source File

SOURCE=..\..\src\md5.h
# End Source File
# Begin Source File

SOURCE=..\..\src\menubar.h

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\menubar.h
InputName=menubar

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\menubar.h
InputName=menubar

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\menubar.h
InputName=menubar

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\menubar.h
InputName=menubar

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\netclient.h

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\netclient.h
InputName=netclient

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\netclient.h
InputName=netclient

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\netclient.h
InputName=netclient

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\netclient.h
InputName=netclient

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\nicklist.h
# End Source File
# Begin Source File

SOURCE=..\..\src\parser.h
# End Source File
# Begin Source File

SOURCE=..\..\src\picviewerimpl.h

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\picviewerimpl.h
InputName=picviewerimpl

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\picviewerimpl.h
InputName=picviewerimpl

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\picviewerimpl.h
InputName=picviewerimpl

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\picviewerimpl.h
InputName=picviewerimpl

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\platform.h
# End Source File
# Begin Source File

SOURCE=..\..\src\prefsimpl.h

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\prefsimpl.h
InputName=prefsimpl

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\prefsimpl.h
InputName=prefsimpl

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\prefsimpl.h
InputName=prefsimpl

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\prefsimpl.h
InputName=prefsimpl

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\privatewindowimpl.h

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\privatewindowimpl.h
InputName=privatewindowimpl

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\privatewindowimpl.h
InputName=privatewindowimpl

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\privatewindowimpl.h
InputName=privatewindowimpl

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\privatewindowimpl.h
InputName=privatewindowimpl

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\resolver.h
# End Source File
# Begin Source File

SOURCE=..\..\src\resolverthread.h

!IF  "$(CFG)" == "Unizone - Win32 Release"

# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\resolverthread.h
InputName=resolverthread

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\resolverthread.h
InputName=resolverthread

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\resolverthread.h
InputName=resolverthread

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\resolverthread.h
InputName=resolverthread

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\resource.h
# End Source File
# Begin Source File

SOURCE=..\..\src\scanevent.h
# End Source File
# Begin Source File

SOURCE=..\..\src\scanprogressimpl.h
# End Source File
# Begin Source File

SOURCE=..\..\src\search.h

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\search.h
InputName=search

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\search.h
InputName=search

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\search.h
InputName=search

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\search.h
InputName=search

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\searchitem.h
# End Source File
# Begin Source File

SOURCE=..\..\src\serverclient.h

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\serverclient.h
InputName=serverclient

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\serverclient.h
InputName=serverclient

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\serverclient.h
InputName=serverclient

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\serverclient.h
InputName=serverclient

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\settings.h
# End Source File
# Begin Source File

SOURCE=..\..\src\textevent.h
# End Source File
# Begin Source File

SOURCE=..\..\src\titanic.h
# End Source File
# Begin Source File

SOURCE=..\..\src\tokenizer.h
# End Source File
# Begin Source File

SOURCE=..\..\src\transferitem.h
# End Source File
# Begin Source File

SOURCE=..\..\src\transferlist.h
# End Source File
# Begin Source File

SOURCE=..\..\src\uenv.h
# End Source File
# Begin Source File

SOURCE=..\..\src\ulistview.h

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\ulistview.h
InputName=ulistview

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\ulistview.h
InputName=ulistview

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\ulistview.h
InputName=ulistview

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\ulistview.h
InputName=ulistview

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\updateclient.h

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\updateclient.h
InputName=updateclient

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\updateclient.h
InputName=updateclient

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\updateclient.h
InputName=updateclient

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\updateclient.h
InputName=updateclient

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\uploadimpl.h

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\uploadimpl.h
InputName=uploadimpl

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\uploadimpl.h
InputName=uploadimpl

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\uploadimpl.h
InputName=uploadimpl

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\uploadimpl.h
InputName=uploadimpl

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\uploadthread.h

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\uploadthread.h
InputName=uploadthread

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\uploadthread.h
InputName=uploadthread

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\uploadthread.h
InputName=uploadthread

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\uploadthread.h
InputName=uploadthread

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\user.h
# End Source File
# Begin Source File

SOURCE=..\..\src\userlistitem.h
# End Source File
# Begin Source File

SOURCE=..\..\src\util.h
# End Source File
# Begin Source File

SOURCE=..\..\src\version.h
# End Source File
# Begin Source File

SOURCE=..\..\src\wcrypt.h
# End Source File
# Begin Source File

SOURCE=..\..\src\wdownloadevent.h
# End Source File
# Begin Source File

SOURCE=..\..\src\werrorevent.h
# End Source File
# Begin Source File

SOURCE=..\..\src\wfile.h
# End Source File
# Begin Source File

SOURCE=..\..\src\winsharewindow.h

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\winsharewindow.h
InputName=winsharewindow

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\winsharewindow.h
InputName=winsharewindow

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\winsharewindow.h
InputName=winsharewindow

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\winsharewindow.h
InputName=winsharewindow

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\windows\wlaunchthread_win.h
# End Source File
# Begin Source File

SOURCE=..\..\src\wmessageevent.h
# End Source File
# Begin Source File

SOURCE=..\..\src\wpwevent.h
# End Source File
# Begin Source File

SOURCE=..\..\src\wstatusbar.h
# End Source File
# Begin Source File

SOURCE=..\..\src\wstring.h
# End Source File
# Begin Source File

SOURCE=..\..\src\wsystemevent.h
# End Source File
# Begin Source File

SOURCE=..\..\src\wuploadevent.h
# End Source File
# Begin Source File

SOURCE=..\..\src\wutil.h
# End Source File
# Begin Source File

SOURCE=..\..\src\wwarningevent.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\..\src\icon.rc
# End Source File
# Begin Source File

SOURCE=..\..\src\icon2.ico
# End Source File
# Begin Source File

SOURCE=..\..\src\unizone.ico
# End Source File
# Begin Source File

SOURCE=..\..\src\qt2\unizone.pro
# End Source File
# End Group
# Begin Group "UI files"

# PROP Default_Filter "ui"
# Begin Source File

SOURCE=..\..\src\aboutdlg.ui

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\aboutdlg.ui
InputName=aboutdlg

BuildCmds= \
	$(QTDIR)\bin\uic.exe $(InputPath) -o $(InputDir)\qt2\$(InputName).h \
	$(QTDIR)\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\qt2\$(InputName).cpp \
	

"$(InputDir)\qt2\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\qt2\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\aboutdlg.ui
InputName=aboutdlg

BuildCmds= \
	$(QTDIR)\bin\uic.exe $(InputPath) -o $(InputDir)\qt2\$(InputName).h \
	$(QTDIR)\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\qt2\$(InputName).cpp \
	

"$(InputDir)\qt2\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\qt2\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\aboutdlg.ui
InputName=aboutdlg

BuildCmds= \
	$(QTDIR)\bin\uic.exe $(InputPath) -o $(InputDir)\qt2\$(InputName).h \
	$(QTDIR)\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\qt2\$(InputName).cpp \
	

"$(InputDir)\qt2\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\qt2\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\aboutdlg.ui
InputName=aboutdlg

BuildCmds= \
	$(QTDIR)\bin\uic.exe $(InputPath) -o $(InputDir)\qt2\$(InputName).h \
	$(QTDIR)\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\qt2\$(InputName).cpp \
	

"$(InputDir)\qt2\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\qt2\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\channel.ui

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\channel.ui
InputName=channel

BuildCmds= \
	$(QTDIR)\bin\uic.exe $(InputPath) -o $(InputDir)\qt2\$(InputName).h \
	$(QTDIR)\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\qt2\$(InputName).cpp \
	

"$(InputDir)\qt2\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\qt2\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\channel.ui
InputName=channel

BuildCmds= \
	$(QTDIR)\bin\uic.exe $(InputPath) -o $(InputDir)\qt2\$(InputName).h \
	$(QTDIR)\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\qt2\$(InputName).cpp \
	

"$(InputDir)\qt2\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\qt2\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\channel.ui
InputName=channel

BuildCmds= \
	$(QTDIR)\bin\uic.exe $(InputPath) -o $(InputDir)\qt2\$(InputName).h \
	$(QTDIR)\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\qt2\$(InputName).cpp \
	

"$(InputDir)\qt2\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\qt2\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\channel.ui
InputName=channel

BuildCmds= \
	$(QTDIR)\bin\uic.exe $(InputPath) -o $(InputDir)\qt2\$(InputName).h \
	$(QTDIR)\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\qt2\$(InputName).cpp \
	

"$(InputDir)\qt2\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\qt2\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\picviewer.ui

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\picviewer.ui
InputName=picviewer

BuildCmds= \
	$(QTDIR)\bin\uic.exe $(InputPath) -o $(InputDir)\qt2\$(InputName).h \
	$(QTDIR)\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\qt2\$(InputName).cpp \
	

"$(InputDir)\qt2\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\qt2\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\picviewer.ui
InputName=picviewer

BuildCmds= \
	$(QTDIR)\bin\uic.exe $(InputPath) -o $(InputDir)\qt2\$(InputName).h \
	$(QTDIR)\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\qt2\$(InputName).cpp \
	

"$(InputDir)\qt2\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\qt2\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\picviewer.ui
InputName=picviewer

BuildCmds= \
	$(QTDIR)\bin\uic.exe $(InputPath) -o $(InputDir)\qt2\$(InputName).h \
	$(QTDIR)\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\qt2\$(InputName).cpp \
	

"$(InputDir)\qt2\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\qt2\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\picviewer.ui
InputName=picviewer

BuildCmds= \
	$(QTDIR)\bin\uic.exe $(InputPath) -o $(InputDir)\qt2\$(InputName).h \
	$(QTDIR)\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\qt2\$(InputName).cpp \
	

"$(InputDir)\qt2\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\qt2\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\prefs.ui

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\prefs.ui
InputName=prefs

BuildCmds= \
	$(QTDIR)\bin\uic.exe $(InputPath) -o $(InputDir)\qt2\$(InputName).h \
	$(QTDIR)\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\qt2\$(InputName).cpp \
	

"$(InputDir)\qt2\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\qt2\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\prefs.ui
InputName=prefs

BuildCmds= \
	$(QTDIR)\bin\uic.exe $(InputPath) -o $(InputDir)\qt2\$(InputName).h \
	$(QTDIR)\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\qt2\$(InputName).cpp \
	

"$(InputDir)\qt2\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\qt2\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\prefs.ui
InputName=prefs

BuildCmds= \
	$(QTDIR)\bin\uic.exe $(InputPath) -o $(InputDir)\qt2\$(InputName).h \
	$(QTDIR)\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\qt2\$(InputName).cpp \
	

"$(InputDir)\qt2\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\qt2\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\prefs.ui
InputName=prefs

BuildCmds= \
	$(QTDIR)\bin\uic.exe $(InputPath) -o $(InputDir)\qt2\$(InputName).h \
	$(QTDIR)\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\qt2\$(InputName).cpp \
	

"$(InputDir)\qt2\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\qt2\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\privatewindow.ui

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\privatewindow.ui
InputName=privatewindow

BuildCmds= \
	$(QTDIR)\bin\uic.exe $(InputPath) -o $(InputDir)\qt2\$(InputName).h \
	$(QTDIR)\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\qt2\$(InputName).cpp \
	

"$(InputDir)\qt2\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\qt2\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\privatewindow.ui
InputName=privatewindow

BuildCmds= \
	$(QTDIR)\bin\uic.exe $(InputPath) -o $(InputDir)\qt2\$(InputName).h \
	$(QTDIR)\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\qt2\$(InputName).cpp \
	

"$(InputDir)\qt2\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\qt2\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\privatewindow.ui
InputName=privatewindow

BuildCmds= \
	$(QTDIR)\bin\uic.exe $(InputPath) -o $(InputDir)\qt2\$(InputName).h \
	$(QTDIR)\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\qt2\$(InputName).cpp \
	

"$(InputDir)\qt2\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\qt2\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP BASE Ignore_Default_Tool 1
# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\privatewindow.ui
InputName=privatewindow

BuildCmds= \
	$(QTDIR)\bin\uic.exe $(InputPath) -o $(InputDir)\qt2\$(InputName).h \
	$(QTDIR)\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\qt2\$(InputName).cpp \
	

"$(InputDir)\qt2\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\qt2\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\scanprogress.ui

!IF  "$(CFG)" == "Unizone - Win32 Release"

# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\scanprogress.ui
InputName=scanprogress

BuildCmds= \
	$(QTDIR)\bin\uic.exe $(InputPath) -o $(InputDir)\qt2\$(InputName).h \
	$(QTDIR)\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\qt2\$(InputName).cpp \
	

"$(InputDir)\qt2\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\qt2\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\scanprogress.ui
InputName=scanprogress

BuildCmds= \
	$(QTDIR)\bin\uic.exe $(InputPath) -o $(InputDir)\qt2\$(InputName).h \
	$(QTDIR)\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\qt2\$(InputName).cpp \
	

"$(InputDir)\qt2\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\qt2\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\scanprogress.ui
InputName=scanprogress

BuildCmds= \
	$(QTDIR)\bin\uic.exe $(InputPath) -o $(InputDir)\qt2\$(InputName).h \
	$(QTDIR)\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\qt2\$(InputName).cpp \
	

"$(InputDir)\qt2\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\qt2\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\unizone\trunk\unizone\src
InputPath=..\..\src\scanprogress.ui
InputName=scanprogress

BuildCmds= \
	$(QTDIR)\bin\uic.exe $(InputPath) -o $(InputDir)\qt2\$(InputName).h \
	$(QTDIR)\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\qt2\$(InputName).cpp \
	

"$(InputDir)\qt2\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\qt2\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "ANSI"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\windows\ansi\fileinfo_win.cpp

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP BASE Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\windows\ansi\filethread_win.cpp

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP BASE Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\windows\ansi\gotourl_win.cpp

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP BASE Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\windows\ansi\wfile_win.cpp

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\windows\ansi\wlaunchthread_win.cpp

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP BASE Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "Unicode"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\windows\unicode\fileinfo_win.cpp

!IF  "$(CFG)" == "Unizone - Win32 Release"

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\windows\unicode\filethread_win.cpp

!IF  "$(CFG)" == "Unizone - Win32 Release"

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\windows\unicode\gotourl_win.cpp

!IF  "$(CFG)" == "Unizone - Win32 Release"

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\windows\unicode\wfile_win.cpp

!IF  "$(CFG)" == "Unizone - Win32 Release"

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\windows\unicode\wlaunchthread_win.cpp

!IF  "$(CFG)" == "Unizone - Win32 Release"

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "MUSCLE Headers"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\src\muscle\iogateway\AbstractMessageIOGateway.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\reflector\AbstractReflectSession.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\reflector\AbstractSessionIOPolicy.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\system\AcceptSocketsThread.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\system\AtomicCounter.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\util\ByteBuffer.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\dataio\DataIO.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\reflector\DataNode.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\reflector\DumbReflectSession.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\dataio\FileDataIO.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\reflector\FilterSessionFactory.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\util\FlatCountable.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\support\Flattenable.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\system\GlobalMemoryAllocator.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\util\Hashtable.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\syslog\LogCallback.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\util\MemoryAllocator.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\dataio\MemoryBufferDataIO.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\message\Message.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\iogateway\MessageIOGateway.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\system\MessageTransceiverThread.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\util\MiscUtilityFunctions.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\support\MuscleSupport.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\system\Mutex.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\util\NetworkUtilityFunctions.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\dataio\NullDataIO.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\util\ObjectPool.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\regex\PathMatcher.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\iogateway\PlainTextMessageIOGateway.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\support\Point.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\util\PulseNode.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\qtsupport\QAcceptSocketsThread.h

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src\muscle\qtsupport
InputPath=..\..\src\muscle\qtsupport\QAcceptSocketsThread.h
InputName=QAcceptSocketsThread

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src\muscle\qtsupport
InputPath=..\..\src\muscle\qtsupport\QAcceptSocketsThread.h
InputName=QAcceptSocketsThread

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src\muscle\qtsupport
InputPath=..\..\src\muscle\qtsupport\QAcceptSocketsThread.h
InputName=QAcceptSocketsThread

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src\muscle\qtsupport
InputPath=..\..\src\muscle\qtsupport\QAcceptSocketsThread.h
InputName=QAcceptSocketsThread

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\qtsupport\QMessageTransceiverThread.h

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src\muscle\qtsupport
InputPath=..\..\src\muscle\qtsupport\QMessageTransceiverThread.h
InputName=QMessageTransceiverThread

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src\muscle\qtsupport
InputPath=..\..\src\muscle\qtsupport\QMessageTransceiverThread.h
InputName=QMessageTransceiverThread

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src\muscle\qtsupport
InputPath=..\..\src\muscle\qtsupport\QMessageTransceiverThread.h
InputName=QMessageTransceiverThread

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src\muscle\qtsupport
InputPath=..\..\src\muscle\qtsupport\QMessageTransceiverThread.h
InputName=QMessageTransceiverThread

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\regex\QueryFilter.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\util\Queue.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\reflector\RateLimitSessionIOPolicy.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\iogateway\RawDataMessageIOGateway.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\support\Rect.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\util\RefCount.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\reflector\ReflectServer.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\reflector\ServerComponent.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\system\SetupSystem.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\iogateway\SignalMessageIOGateway.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\util\SocketHolder.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\reflector\StorageReflectConstants.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\reflector\StorageReflectSession.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\util\String.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\regex\StringMatcher.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\util\StringTokenizer.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\syslog\SysLog.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\system\SystemInfo.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\dataio\TCPSocketDataIO.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\system\Thread.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\util\TimeUtilityFunctions.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\support\Tuple.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\zlib\ZLibCodec.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\zlib\ZLibDataIO.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\zlib\ZLibUtilityFunctions.h
# End Source File
# End Group
# Begin Group "MUSCLE Sources"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\src\muscle\iogateway\AbstractMessageIOGateway.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\reflector\AbstractReflectSession.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\system\AcceptSocketsThread.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\util\ByteBuffer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\reflector\DataNode.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\reflector\DumbReflectSession.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\reflector\FilterSessionFactory.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\system\GlobalMemoryAllocator.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\util\MemoryAllocator.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\message\Message.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\iogateway\MessageIOGateway.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\system\MessageTransceiverThread.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\util\MiscUtilityFunctions.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\util\NetworkUtilityFunctions.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\regex\PathMatcher.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\iogateway\PlainTextMessageIOGateway.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\util\PulseNode.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\qtsupport\QAcceptSocketsThread.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\qtsupport\QMessageTransceiverThread.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\regex\QueryFilter.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\reflector\RateLimitSessionIOPolicy.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\iogateway\RawDataMessageIOGateway.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\reflector\ReflectServer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\reflector\ServerComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\system\SetupSystem.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\reflector\StorageReflectSession.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\util\String.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\regex\StringMatcher.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\syslog\SysLog.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\system\SystemInfo.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\system\Thread.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\zlib\ZLibCodec.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\zlib\ZLibDataIO.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\zlib\ZLibUtilityFunctions.cpp
# End Source File
# End Group
# Begin Group "MUSCLE MOC Sources"

# PROP Default_Filter "cpp"
# Begin Source File

SOURCE=..\..\src\muscle\qtsupport\qt2\moc_QAcceptSocketsThread.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\qtsupport\qt2\moc_QMessageTransceiverThread.cpp
# End Source File
# End Group
# Begin Group "regex Sources"

# PROP Default_Filter ".c"
# Begin Source File

SOURCE=..\..\src\muscle\regex\regex\debug.c
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\regex\regex\regcomp.c
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\regex\regex\regerror.c
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\regex\regex\regexec.c
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\regex\regex\regfree.c
# End Source File
# End Group
# Begin Group "regex Headers"

# PROP Default_Filter ".h"
# Begin Source File

SOURCE=..\..\src\muscle\regex\regex\regex.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\regex\regex\regex2.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\regex\regex\utils.h
# End Source File
# End Group
# Begin Group "zlib Sources"

# PROP Default_Filter ".c"
# Begin Source File

SOURCE=..\..\src\muscle\zlib\zlib\adler32.c

!IF  "$(CFG)" == "Unizone - Win32 Release"

# ADD CPP /I "..\..\src\muscle\zlib\zlib"

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# ADD CPP /I "..\..\src\muscle\zlib\zlib" /D "ASMV" /D "ASMINF"

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# ADD CPP /I "..\..\src\muscle\zlib\zlib"

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# ADD CPP /I "..\..\src\muscle\zlib\zlib"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\zlib\zlib\compress.c

!IF  "$(CFG)" == "Unizone - Win32 Release"

# ADD CPP /I "..\..\src\muscle\zlib\zlib"

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# ADD CPP /I "..\..\src\muscle\zlib\zlib" /D "ASMV" /D "ASMINF"

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# ADD CPP /I "..\..\src\muscle\zlib\zlib"

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# ADD CPP /I "..\..\src\muscle\zlib\zlib"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\zlib\zlib\crc32.c

!IF  "$(CFG)" == "Unizone - Win32 Release"

# ADD CPP /I "..\..\src\muscle\zlib\zlib"

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# ADD CPP /I "..\..\src\muscle\zlib\zlib" /D "ASMV" /D "ASMINF"

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# ADD CPP /I "..\..\src\muscle\zlib\zlib"

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# ADD CPP /I "..\..\src\muscle\zlib\zlib"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\zlib\zlib\deflate.c

!IF  "$(CFG)" == "Unizone - Win32 Release"

# ADD CPP /I "..\..\src\muscle\zlib\zlib" /D "ASMV"

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# ADD CPP /I "..\..\src\muscle\zlib\zlib" /D "ASMV" /D "ASMINF"

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# ADD CPP /I "..\..\src\muscle\zlib\zlib" /D "ASMV"

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# ADD CPP /I "..\..\src\muscle\zlib\zlib" /D "ASMV"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\zlib\zlib\contrib\masmx86\gvmat32.asm

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Assembling...
IntDir=.\Release
InputPath=..\..\src\muscle\zlib\zlib\contrib\masmx86\gvmat32.asm
InputName=gvmat32

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml.exe /nologo /c /coff /Cx /Fo"$(IntDir)\$(InputName).obj" "$(InputPath)"

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Assembling...
IntDir=.\Debug
InputPath=..\..\src\muscle\zlib\zlib\contrib\masmx86\gvmat32.asm
InputName=gvmat32

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml.exe /nologo /c /coff /Cx /Zi /Fo"$(IntDir)\$(InputName).obj" "$(InputPath)"

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Assembling...
IntDir=.\Debug_ANSI
InputPath=..\..\src\muscle\zlib\zlib\contrib\masmx86\gvmat32.asm
InputName=gvmat32

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml.exe /nologo /c /coff /Cx /Zi /Fo"$(IntDir)\$(InputName).obj" "$(InputPath)"

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Assembling...
IntDir=.\Release_ANSI
InputPath=..\..\src\muscle\zlib\zlib\contrib\masmx86\gvmat32.asm
InputName=gvmat32

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml.exe /nologo /c /coff /Cx /Fo"$(IntDir)\$(InputName).obj" "$(InputPath)"

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\zlib\zlib\contrib\masmx86\gvmat32c.c
# ADD CPP /I "..\..\src\muscle\zlib\zlib" /D "ASMV"
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\zlib\zlib\gzio.c

!IF  "$(CFG)" == "Unizone - Win32 Release"

# ADD CPP /I "..\..\src\muscle\zlib\zlib"

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# ADD CPP /I "..\..\src\muscle\zlib\zlib" /D "ASMV" /D "ASMINF"

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# ADD CPP /I "..\..\src\muscle\zlib\zlib"

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# ADD CPP /I "..\..\src\muscle\zlib\zlib"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\zlib\zlib\contrib\masmx86\inffas32.asm

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Assembling...
IntDir=.\Release
InputPath=..\..\src\muscle\zlib\zlib\contrib\masmx86\inffas32.asm
InputName=inffas32

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml.exe /nologo /c /coff /Cx /Fo"$(IntDir)\$(InputName).obj" "$(InputPath)"

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Assembling...
IntDir=.\Debug
InputPath=..\..\src\muscle\zlib\zlib\contrib\masmx86\inffas32.asm
InputName=inffas32

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml.exe /nologo /c /coff /Cx /Zi /Fo"$(IntDir)\$(InputName).obj" "$(InputPath)"

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Assembling...
IntDir=.\Debug_ANSI
InputPath=..\..\src\muscle\zlib\zlib\contrib\masmx86\inffas32.asm
InputName=inffas32

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml.exe /nologo /c /coff /Cx /Zi /Fo"$(IntDir)\$(InputName).obj" "$(InputPath)"

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Assembling...
IntDir=.\Release_ANSI
InputPath=..\..\src\muscle\zlib\zlib\contrib\masmx86\inffas32.asm
InputName=inffas32

"$(IntDir)\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	ml.exe /nologo /c /coff /Cx /Fo"$(IntDir)\$(InputName).obj" "$(InputPath)"

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\zlib\zlib\inflate.c

!IF  "$(CFG)" == "Unizone - Win32 Release"

# ADD CPP /I "..\..\src\muscle\zlib\zlib"

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# ADD CPP /I "..\..\src\muscle\zlib\zlib" /D "ASMV" /D "ASMINF"

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# ADD CPP /I "..\..\src\muscle\zlib\zlib"

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# ADD CPP /I "..\..\src\muscle\zlib\zlib"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\zlib\zlib\inftrees.c

!IF  "$(CFG)" == "Unizone - Win32 Release"

# ADD CPP /I "..\..\src\muscle\zlib\zlib"

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# ADD CPP /I "..\..\src\muscle\zlib\zlib" /D "ASMV" /D "ASMINF"

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# ADD CPP /I "..\..\src\muscle\zlib\zlib"

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# ADD CPP /I "..\..\src\muscle\zlib\zlib"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\zlib\zlib\trees.c

!IF  "$(CFG)" == "Unizone - Win32 Release"

# ADD CPP /I "..\..\src\muscle\zlib\zlib"

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# ADD CPP /I "..\..\src\muscle\zlib\zlib" /D "ASMV" /D "ASMINF"

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# ADD CPP /I "..\..\src\muscle\zlib\zlib"

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# ADD CPP /I "..\..\src\muscle\zlib\zlib"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\zlib\zlib\uncompr.c

!IF  "$(CFG)" == "Unizone - Win32 Release"

# ADD CPP /I "..\..\src\muscle\zlib\zlib"

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# ADD CPP /I "..\..\src\muscle\zlib\zlib" /D "ASMV" /D "ASMINF"

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# ADD CPP /I "..\..\src\muscle\zlib\zlib"

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# ADD CPP /I "..\..\src\muscle\zlib\zlib"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\zlib\zlib\zutil.c

!IF  "$(CFG)" == "Unizone - Win32 Release"

# ADD CPP /I "..\..\src\muscle\zlib\zlib"

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# ADD CPP /I "..\..\src\muscle\zlib\zlib" /D "ASMV" /D "ASMINF"

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# ADD CPP /I "..\..\src\muscle\zlib\zlib"

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# ADD CPP /I "..\..\src\muscle\zlib\zlib"

!ENDIF 

# End Source File
# End Group
# Begin Group "zlib Headers"

# PROP Default_Filter ".h"
# Begin Source File

SOURCE=..\..\src\muscle\zlib\zlib\crc32.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\zlib\zlib\deflate.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\zlib\zlib\inffast.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\zlib\zlib\inffixed.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\zlib\zlib\inflate.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\zlib\zlib\inftrees.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\zlib\zlib\trees.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\zlib\zlib\zconf.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\zlib\zlib\zlib.h
# End Source File
# Begin Source File

SOURCE=..\..\src\muscle\zlib\zlib\zutil.h
# End Source File
# End Group
# Begin Group "qjpeg Sources"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\libjpeg\jcapimin.c
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jcapistd.c
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jccoefct.c
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jccolor.c
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jcdctmgr.c
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jchuff.c
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jcinit.c
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jcmainct.c
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jcmarker.c
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jcmaster.c
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jcomapi.c
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jcparam.c
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jcphuff.c
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jcprepct.c
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jcsample.c
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jctrans.c
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jdapimin.c
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jdapistd.c
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jdatadst.c
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jdatasrc.c
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jdcoefct.c
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jdcolor.c
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jddctmgr.c
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jdhuff.c
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jdinput.c
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jdmainct.c
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jdmarker.c
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jdmaster.c
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jdmerge.c
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jdphuff.c
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jdpostct.c
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jdsample.c
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jdtrans.c
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jerror.c
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jfdctflt.c
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jfdctfst.c
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jfdctint.c
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jidctflt.c
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jidctfst.c
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jidctint.c
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jidctred.c
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jmemmgr.c
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jmemnobs.c
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jpegio.cpp
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jquant1.c
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jquant2.c
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jutils.c
# End Source File
# End Group
# Begin Group "qjpeg Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\libjpeg\jchuff.h
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jconfig.h
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jdct.h
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jdhuff.h
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jerror.h
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jinclude.h
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jmemsys.h
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jmorecfg.h
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jpegint.h
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jpegio.h
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jpeglib.h
# End Source File
# Begin Source File

SOURCE=..\..\libjpeg\jversion.h
# End Source File
# End Group
# Begin Group "UIC Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\qt2\aboutdlg.h

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src\qt2
InputPath=..\..\src\qt2\aboutdlg.h
InputName=aboutdlg

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src\qt2
InputPath=..\..\src\qt2\aboutdlg.h
InputName=aboutdlg

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src\qt2
InputPath=..\..\src\qt2\aboutdlg.h
InputName=aboutdlg

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src\qt2
InputPath=..\..\src\qt2\aboutdlg.h
InputName=aboutdlg

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\qt2\channel.h

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src\qt2
InputPath=..\..\src\qt2\channel.h
InputName=channel

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src\qt2
InputPath=..\..\src\qt2\channel.h
InputName=channel

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src\qt2
InputPath=..\..\src\qt2\channel.h
InputName=channel

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src\qt2
InputPath=..\..\src\qt2\channel.h
InputName=channel

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\qt2\picviewer.h

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src\qt2
InputPath=..\..\src\qt2\picviewer.h
InputName=picviewer

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src\qt2
InputPath=..\..\src\qt2\picviewer.h
InputName=picviewer

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src\qt2
InputPath=..\..\src\qt2\picviewer.h
InputName=picviewer

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src\qt2
InputPath=..\..\src\qt2\picviewer.h
InputName=picviewer

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\qt2\prefs.h

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src\qt2
InputPath=..\..\src\qt2\prefs.h
InputName=prefs

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src\qt2
InputPath=..\..\src\qt2\prefs.h
InputName=prefs

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src\qt2
InputPath=..\..\src\qt2\prefs.h
InputName=prefs

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src\qt2
InputPath=..\..\src\qt2\prefs.h
InputName=prefs

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\qt2\privatewindow.h

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src\qt2
InputPath=..\..\src\qt2\privatewindow.h
InputName=privatewindow

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src\qt2
InputPath=..\..\src\qt2\privatewindow.h
InputName=privatewindow

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src\qt2
InputPath=..\..\src\qt2\privatewindow.h
InputName=privatewindow

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src\qt2
InputPath=..\..\src\qt2\privatewindow.h
InputName=privatewindow

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\qt2\scanprogress.h

!IF  "$(CFG)" == "Unizone - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src\qt2
InputPath=..\..\src\qt2\scanprogress.h
InputName=scanprogress

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src\qt2
InputPath=..\..\src\qt2\scanprogress.h
InputName=scanprogress

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Debug ANSI"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src\qt2
InputPath=..\..\src\qt2\scanprogress.h
InputName=scanprogress

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Unizone - Win32 Release ANSI"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\trunk\unizone\src\qt2
InputPath=..\..\src\qt2\scanprogress.h
InputName=scanprogress

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	$(QTDIR)\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "MOC Sources"

# PROP Default_Filter "cpp"
# Begin Source File

SOURCE=..\..\src\qt2\moc_aboutdlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\qt2\moc_aboutdlgimpl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\qt2\moc_acronymclient.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\qt2\moc_channel.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\qt2\moc_channelimpl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\qt2\moc_channels.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\qt2\moc_chattext.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\qt2\moc_combo.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\qt2\moc_downloadimpl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\qt2\moc_downloadthread.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\qt2\moc_filethread.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\qt2\moc_formatting.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\qt2\moc_htmlview.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\qt2\moc_listthread.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\qt2\moc_menubar.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\qt2\moc_netclient.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\qt2\moc_picviewer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\qt2\moc_picviewerimpl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\qt2\moc_prefs.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\qt2\moc_prefsimpl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\qt2\moc_privatewindow.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\qt2\moc_privatewindowimpl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\qt2\moc_resolverthread.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\qt2\moc_scanprogress.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\qt2\moc_search.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\qt2\moc_serverclient.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\qt2\moc_ulistview.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\qt2\moc_updateclient.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\qt2\moc_uploadimpl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\qt2\moc_uploadthread.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\qt2\moc_winsharewindow.cpp
# End Source File
# End Group
# Begin Group "UIC sources"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\qt2\aboutdlg.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\qt2\channel.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\qt2\picviewer.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\qt2\prefs.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\qt2\privatewindow.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\qt2\scanprogress.cpp
# End Source File
# End Group
# Begin Group "CRT Sources"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\windows\_filwbuf.c
# End Source File
# Begin Source File

SOURCE=..\..\src\windows\_getbuf.c
# End Source File
# Begin Source File

SOURCE=..\..\src\windows\imports.c
# End Source File
# Begin Source File

SOURCE=..\..\src\windows\vsscanf.c
# End Source File
# Begin Source File

SOURCE=..\..\src\windows\vwsscanf.c
# End Source File
# End Group
# Begin Group "CRT Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\windows\imports.h
# End Source File
# Begin Source File

SOURCE=..\..\src\windows\vsscanf.h
# End Source File
# End Group
# End Target
# End Project
