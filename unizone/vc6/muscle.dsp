# Microsoft Developer Studio Project File - Name="muscle" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=muscle - Win32 Debug ANSI
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "muscle.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "muscle.mak" CFG="muscle - Win32 Debug ANSI"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "muscle - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "muscle - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "muscle - Win32 Debug ANSI" (based on "Win32 (x86) Static Library")
!MESSAGE "muscle - Win32 Release ANSI" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "muscle - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "muscle___Win32_Release"
# PROP Intermediate_Dir "muscle___Win32_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "..\src\muscle" /I "$(QTDIR)\include" /I "..\src\muscle\regex" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "QT_THREAD_SUPPORT" /D "QT_DLL" /D "UNICODE" /D "MUSCLE_ENABLE_ZLIB_ENCODING" /D "ZLIB_USEDLL" /D "REGEX_USEDLL" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"muscle.lib"

!ELSEIF  "$(CFG)" == "muscle - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "muscle___Win32_Debug"
# PROP BASE Intermediate_Dir "muscle___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "muscle___Win32_Debug"
# PROP Intermediate_Dir "muscle___Win32_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /GR /GX /ZI /Od /I "..\src\muscle" /I "$(QTDIR)\include" /I "..\src\muscle\regex" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "QT_THREAD_SUPPORT" /D "QT_DLL" /D "UNICODE" /D "MUSCLE_ENABLE_ZLIB_ENCODING" /D "ZLIB_USEDLL" /D "REGEX_USEDLL" /D "DISABLE_OBJECT_POOLING" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"muscle___Win32_Debug/muscled.bsc"
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"muscled.lib"

!ELSEIF  "$(CFG)" == "muscle - Win32 Debug ANSI"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "muscle___Win32_Debug_ANSI"
# PROP BASE Intermediate_Dir "muscle___Win32_Debug_ANSI"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "muscle___Win32_Debug_ANSI"
# PROP Intermediate_Dir "muscle___Win32_Debug_ANSI"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /GR /GX /ZI /Od /I "..\src\muscle" /I "$(QTDIR)\include" /I "..\src\muscle\regex" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "QT_THREAD_SUPPORT" /D "QT_DLL" /D "UNICODE" /D "MUSCLE_ENABLE_ZLIB_ENCODING" /D "ZLIB_USEDLL" /D "REGEX_USEDLL" /D "DISABLE_OBJECT_POOLING" /FR /FD /GZ /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MDd /W3 /GR /GX /ZI /Od /I "..\src\muscle" /I "$(QTDIR)\include" /I "..\src\muscle\regex" /D "_DEBUG" /D "WIN32" /D "_LIB" /D "QT_THREAD_SUPPORT" /D "QT_DLL" /D "MUSCLE_ENABLE_ZLIB_ENCODING" /D "ZLIB_USEDLL" /D "REGEX_USEDLL" /D "DISABLE_OBJECT_POOLING" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"muscle___Win32_Debug_ANSI/musclead.bsc"
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"muscled.lib"
# ADD LIB32 /nologo /out:"musclead.lib"

!ELSEIF  "$(CFG)" == "muscle - Win32 Release ANSI"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "muscle___Win32_Release_ANSI"
# PROP BASE Intermediate_Dir "muscle___Win32_Release_ANSI"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "muscle___Win32_Release_ANSI"
# PROP Intermediate_Dir "muscle___Win32_Release_ANSI"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O2 /I "..\src\muscle" /I "$(QTDIR)\include" /I "..\src\muscle\regex" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "QT_THREAD_SUPPORT" /D "QT_DLL" /D "UNICODE" /D "MUSCLE_ENABLE_ZLIB_ENCODING" /D "ZLIB_USEDLL" /D "REGEX_USEDLL" /FR /FD /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "..\src\muscle" /I "$(QTDIR)\include" /I "..\src\muscle\regex" /D "NDEBUG" /D "WIN32" /D "_LIB" /D "QT_THREAD_SUPPORT" /D "QT_DLL" /D "MUSCLE_ENABLE_ZLIB_ENCODING" /D "ZLIB_USEDLL" /D "REGEX_USEDLL" /FR /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"muscle___Win32_Release_ANSI/musclea.bsc"
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"muscle.lib"
# ADD LIB32 /nologo /out:"musclea.lib"

!ENDIF 

# Begin Target

# Name "muscle - Win32 Release"
# Name "muscle - Win32 Debug"
# Name "muscle - Win32 Debug ANSI"
# Name "muscle - Win32 Release ANSI"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\muscle\iogateway\AbstractMessageIOGateway.cpp
# End Source File
# Begin Source File

SOURCE=..\src\muscle\reflector\AbstractReflectSession.cpp
# End Source File
# Begin Source File

SOURCE=..\src\muscle\system\AcceptSocketsThread.cpp
# End Source File
# Begin Source File

SOURCE=..\src\muscle\util\ByteBuffer.cpp
# End Source File
# Begin Source File

SOURCE=..\src\muscle\reflector\DumbReflectSession.cpp
# End Source File
# Begin Source File

SOURCE=..\src\muscle\reflector\FilterSessionFactory.cpp
# End Source File
# Begin Source File

SOURCE=..\src\muscle\system\GlobalMemoryAllocator.cpp
# End Source File
# Begin Source File

SOURCE=..\src\muscle\util\MemoryAllocator.cpp
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

SOURCE=..\src\muscle\util\MiscUtilityFunctions.cpp
# End Source File
# Begin Source File

SOURCE=..\src\muscle\util\NetworkUtilityFunctions.cpp
# End Source File
# Begin Source File

SOURCE=..\src\muscle\regex\PathMatcher.cpp
# End Source File
# Begin Source File

SOURCE=..\src\muscle\iogateway\PlainTextMessageIOGateway.cpp
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

SOURCE=..\src\muscle\iogateway\RawDataMessageIOGateway.cpp
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
# Begin Source File

SOURCE=..\src\muscle\zlib\ZLibDataIO.cpp
# End Source File
# Begin Source File

SOURCE=..\src\muscle\zlib\ZLibUtilityFunctions.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
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

SOURCE=..\src\muscle\system\AcceptSocketsThread.h
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

SOURCE=..\src\muscle\dataio\FileDataIO.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\reflector\FilterSessionFactory.h
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

SOURCE=..\src\muscle\dataio\MemoryBufferDataIO.h
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

SOURCE=..\src\muscle\util\MiscUtilityFunctions.h
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

SOURCE=..\src\muscle\dataio\NullDataIO.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\util\ObjectPool.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\regex\PathMatcher.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\iogateway\PlainTextMessageIOGateway.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\support\Point.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\util\PulseNode.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\regex\QueryFilter.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\util\Queue.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\reflector\RateLimitSessionIOPolicy.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\iogateway\RawDataMessageIOGateway.h
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

SOURCE=..\src\muscle\util\String.h
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
# Begin Source File

SOURCE=..\src\muscle\zlib\ZLibDataIO.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\zlib\ZLibUtilityFunctions.h
# End Source File
# End Group
# End Target
# End Project
