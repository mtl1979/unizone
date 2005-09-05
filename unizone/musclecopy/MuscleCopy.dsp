# Microsoft Developer Studio Project File - Name="MuscleCopy" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=MuscleCopy - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "MuscleCopy.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MuscleCopy.mak" CFG="MuscleCopy - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MuscleCopy - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "MuscleCopy - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "MuscleCopy - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "$(QTDIR)\include" /I "..\src\muscle" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "QT_THREAD_SUPPORT" /D "QT_DLL" /D "QT_NO_ASCII_CAST" /D "BETA" /D "UNICODE" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40b /d "NDEBUG"
# ADD RSC /l 0x40b /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib shlwapi.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib winmm.lib qt-mt3.lib qtmain.lib /nologo /subsystem:windows /pdb:none /machine:I386 /nodefaultlib:"msvcrtd" /libpath:"$(QTDIR)\lib"

!ELSEIF  "$(CFG)" == "MuscleCopy - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "$(QTDIR)\include" /I "..\src" /I "..\src\muscle" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "QT_THREAD_SUPPORT" /D "QT_DLL" /D "QT_NO_ASCII_CAST" /D "BETA" /D "UNICODE" /D "MUSCLE_USE_X86_INLINE_ASSEMBLY" /D "MUSCLE_ENABLE_ZLIB_ENCODING" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40b /d "_DEBUG"
# ADD RSC /l 0x40b /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib shlwapi.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib winmm.lib qt-mt3.lib qtmain.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"msvcrt" /pdbtype:sept /libpath:"$(QTDIR)\lib"

!ENDIF 

# Begin Target

# Name "MuscleCopy - Win32 Release"
# Name "MuscleCopy - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\downloadthread.cpp
# End Source File
# Begin Source File

SOURCE=.\genericthread.cpp
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE=.\md5.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_genericthread.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_status.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_statusimpl.cpp
# End Source File
# Begin Source File

SOURCE=.\status.cpp
# End Source File
# Begin Source File

SOURCE=.\statusimpl.cpp
# End Source File
# Begin Source File

SOURCE=..\src\windows\wfile.cpp
# End Source File
# Begin Source File

SOURCE=..\src\windows\unicode\wfile_win.cpp
# End Source File
# Begin Source File

SOURCE=..\src\wstring.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\downloadthread.h
# End Source File
# Begin Source File

SOURCE=.\genericthread.h

!IF  "$(CFG)" == "MuscleCopy - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.
InputPath=.\genericthread.h
InputName=genericthread

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "MuscleCopy - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.
InputPath=.\genericthread.h
InputName=genericthread

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\md5.h
# End Source File
# Begin Source File

SOURCE=.\status.h

!IF  "$(CFG)" == "MuscleCopy - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.
InputPath=.\status.h
InputName=status

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "MuscleCopy - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.
InputPath=.\status.h
InputName=status

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\statusimpl.h

!IF  "$(CFG)" == "MuscleCopy - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.
InputPath=.\statusimpl.h
InputName=statusimpl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "MuscleCopy - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.
InputPath=.\statusimpl.h
InputName=statusimpl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\wfile.h
# End Source File
# Begin Source File

SOURCE=.\wgenericevent.h
# End Source File
# Begin Source File

SOURCE=..\src\wstring.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\status.ui

!IF  "$(CFG)" == "MuscleCopy - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.
InputPath=.\status.ui
InputName=status

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(InputDir)\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\$(InputName).cpp \
	

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "MuscleCopy - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.
InputPath=.\status.ui
InputName=status

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
# Begin Group "MUSCLE Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\muscle\iogateway\AbstractMessageIOGateway.cpp
# End Source File
# Begin Source File

SOURCE=..\src\muscle\reflector\AbstractReflectSession.cpp
# End Source File
# Begin Source File

SOURCE=..\src\muscle\util\ByteBuffer.cpp
# End Source File
# Begin Source File

SOURCE=..\src\muscle\reflector\DumbReflectSession.cpp
# End Source File
# Begin Source File

SOURCE=..\src\muscle\message\Message.cpp
# End Source File
# Begin Source File

SOURCE=..\src\muscle\iogateway\MessageIOGateway.cpp
# End Source File
# Begin Source File

SOURCE=..\src\muscle\system\MessageTransceiverThread.cpp
# End Source File
# Begin Source File

SOURCE=..\src\muscle\util\NetworkUtilityFunctions.cpp
# End Source File
# Begin Source File

SOURCE=..\src\muscle\regex\PathMatcher.cpp
# End Source File
# Begin Source File

SOURCE=..\src\muscle\util\PulseNode.cpp
# End Source File
# Begin Source File

SOURCE=..\src\muscle\regex\QueryFilter.cpp
# End Source File
# Begin Source File

SOURCE=..\src\muscle\reflector\RateLimitSessionIOPolicy.cpp
# End Source File
# Begin Source File

SOURCE=..\src\muscle\reflector\ReflectServer.cpp
# End Source File
# Begin Source File

SOURCE=..\src\muscle\reflector\ServerComponent.cpp
# End Source File
# Begin Source File

SOURCE=..\src\muscle\system\SetupSystem.cpp
# End Source File
# Begin Source File

SOURCE=..\src\muscle\reflector\StorageReflectSession.cpp
# End Source File
# Begin Source File

SOURCE=..\src\muscle\util\String.cpp
# End Source File
# Begin Source File

SOURCE=..\src\muscle\regex\StringMatcher.cpp
# End Source File
# Begin Source File

SOURCE=..\src\muscle\syslog\SysLog.cpp
# End Source File
# Begin Source File

SOURCE=..\src\muscle\system\Thread.cpp
# End Source File
# Begin Source File

SOURCE=..\src\muscle\zlib\ZLibCodec.cpp
# End Source File
# End Group
# Begin Group "regex Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\muscle\regex\regex\regcomp.c
# End Source File
# Begin Source File

SOURCE=..\src\muscle\regex\regex\regexec.c
# End Source File
# Begin Source File

SOURCE=..\src\muscle\regex\regex\regfree.c
# End Source File
# End Group
# Begin Group "zlib Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\muscle\zlib\zlib\adler32.c
# End Source File
# Begin Source File

SOURCE=..\src\muscle\zlib\zlib\compress.c
# End Source File
# Begin Source File

SOURCE=..\src\muscle\zlib\zlib\crc32.c
# End Source File
# Begin Source File

SOURCE=..\src\muscle\zlib\zlib\deflate.c
# End Source File
# Begin Source File

SOURCE=..\src\muscle\zlib\zlib\inffast.c
# End Source File
# Begin Source File

SOURCE=..\src\muscle\zlib\zlib\inflate.c
# End Source File
# Begin Source File

SOURCE=..\src\muscle\zlib\zlib\inftrees.c
# End Source File
# Begin Source File

SOURCE=..\src\muscle\zlib\zlib\trees.c
# End Source File
# Begin Source File

SOURCE=..\src\muscle\zlib\zlib\zutil.c
# End Source File
# End Group
# Begin Group "MUSCLE Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\muscle\iogateway\AbstractMessageIOGateway.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\reflector\AbstractReflectSession.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\reflector\AbstractSessionIOPolicy.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\system\AtomicCounter.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\util\ByteBuffer.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\dataio\DataIO.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\reflector\DumbReflectSession.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\util\FlatCountable.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\support\Flattenable.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\system\GlobalMemoryAllocator.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\util\Hashtable.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\syslog\LogCallback.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\util\MemoryAllocator.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\message\Message.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\iogateway\MessageIOGateway.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\system\MessageTransceiverThread.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\support\MuscleSupport.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\system\Mutex.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\util\NetworkUtilityFunctions.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\util\ObjectPool.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\regex\PathMatcher.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\support\Point.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\util\PulseNode.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\reflector\RateLimitSessionIOPolicy.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\support\Rect.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\util\RefCount.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\reflector\ReflectServer.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\reflector\ServerComponent.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\system\SetupSystem.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\iogateway\SignalMessageIOGateway.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\util\SocketHolder.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\reflector\StorageReflectConstants.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\reflector\StorageReflectSession.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\regex\StringMatcher.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\util\StringTokenizer.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\syslog\SysLog.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\dataio\TCPSocketDataIO.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\system\Thread.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\util\TimeUtilityFunctions.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\support\Tuple.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\zlib\ZLibCodec.h
# End Source File
# End Group
# Begin Group "regex Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\muscle\regex\regex\engine.c

!IF  "$(CFG)" == "MuscleCopy - Win32 Release"

!ELSEIF  "$(CFG)" == "MuscleCopy - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\muscle\regex\regex\engine.ih
# End Source File
# Begin Source File

SOURCE=..\src\muscle\regex\regex\regcomp.ih
# End Source File
# Begin Source File

SOURCE=..\src\muscle\regex\regex\regex.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\regex\regex\regex2.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\regex\regex\utils.h
# End Source File
# End Group
# Begin Group "zlib Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\muscle\zlib\zlib\crc32.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\zlib\zlib\deflate.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\zlib\zlib\inffast.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\zlib\zlib\inffixed.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\zlib\zlib\inflate.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\zlib\zlib\inftrees.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\zlib\zlib\trees.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\zlib\zlib\zconf.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\zlib\zlib\zlib.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\zlib\zlib\zutil.h
# End Source File
# End Group
# End Target
# End Project
