# Microsoft Developer Studio Project File - Name="zlib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=zlib - Win32 Debug ANSI
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "zlib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "zlib.mak" CFG="zlib - Win32 Debug ANSI"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "zlib - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "zlib - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "zlib - Win32 Debug ANSI" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "zlib - Win32 Release ANSI" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "zlib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "zlib___Win32_Release"
# PROP BASE Intermediate_Dir "zlib___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "zlib___Win32_Release"
# PROP Intermediate_Dir "zlib___Win32_Release"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ZLIB_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ZLIB_EXPORTS" /D "ZLIB_DLL" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40b /d "NDEBUG"
# ADD RSC /l 0x40b /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /map /machine:I386

!ELSEIF  "$(CFG)" == "zlib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "zlib___Win32_Debug"
# PROP BASE Intermediate_Dir "zlib___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "zlib___Win32_Debug"
# PROP Intermediate_Dir "zlib___Win32_Debug"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ZLIB_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /GR /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ZLIB_EXPORTS" /D "ZLIB_DLL" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40b /d "_DEBUG"
# ADD RSC /l 0x40b /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"zlib___Win32_Debug/zlibd.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /map /debug /machine:I386 /out:"zlibd.dll" /pdbtype:sept

!ELSEIF  "$(CFG)" == "zlib - Win32 Debug ANSI"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "zlib___Win32_Debug_ANSI"
# PROP BASE Intermediate_Dir "zlib___Win32_Debug_ANSI"
# PROP BASE Ignore_Export_Lib 1
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "zlib___Win32_Debug_ANSI"
# PROP Intermediate_Dir "zlib___Win32_Debug_ANSI"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /GR /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ZLIB_EXPORTS" /D "ZLIB_DLL" /FR /FD /GZ /c
# SUBTRACT BASE CPP /YX
# ADD CPP /nologo /MDd /W3 /GR /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "ZLIB_EXPORTS" /D "ZLIB_DLL" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40b /d "_DEBUG"
# ADD RSC /l 0x40b /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"zlib___Win32_Debug_ANSI/zlibad.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /map /debug /machine:I386 /out:"zlibd.dll" /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /map /debug /machine:I386 /out:"zlibad.dll" /pdbtype:sept

!ELSEIF  "$(CFG)" == "zlib - Win32 Release ANSI"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "zlib___Win32_Release_ANSI"
# PROP BASE Intermediate_Dir "zlib___Win32_Release_ANSI"
# PROP BASE Ignore_Export_Lib 1
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "zlib___Win32_Release_ANSI"
# PROP Intermediate_Dir "zlib___Win32_Release_ANSI"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ZLIB_EXPORTS" /D "ZLIB_DLL" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "ZLIB_EXPORTS" /D "ZLIB_DLL" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40b /d "NDEBUG"
# ADD RSC /l 0x40b /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"zlib___Win32_Release_ANSI/zliba.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /map /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /map /machine:I386 /out:"zlib___Win32_Release_ANSI/zliba.dll"

!ENDIF 

# Begin Target

# Name "zlib - Win32 Release"
# Name "zlib - Win32 Debug"
# Name "zlib - Win32 Debug ANSI"
# Name "zlib - Win32 Release ANSI"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
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

SOURCE=..\src\muscle\zlib\zlib\gzio.c
# End Source File
# Begin Source File

SOURCE=..\src\muscle\zlib\zlib\infblock.c
# End Source File
# Begin Source File

SOURCE=..\src\muscle\zlib\zlib\infcodes.c
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

SOURCE=..\src\muscle\zlib\zlib\infutil.c
# End Source File
# Begin Source File

SOURCE=..\src\muscle\zlib\zlib\trees.c
# End Source File
# Begin Source File

SOURCE=..\src\muscle\zlib\zlib\uncompr.c
# End Source File
# Begin Source File

SOURCE=..\src\muscle\zlib\zlib\zutil.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\src\muscle\zlib\zlib\deflate.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\zlib\zlib\infblock.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\zlib\zlib\infcodes.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\zlib\zlib\inffast.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\zlib\zlib\inffixed.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\zlib\zlib\inftrees.h
# End Source File
# Begin Source File

SOURCE=..\src\muscle\zlib\zlib\infutil.h
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
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\src\muscle\zlib\zlib\nt\zlib.rc
# End Source File
# End Group
# End Target
# End Project
