# Microsoft Developer Studio Project File - Name="Image Splitter" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=Image Splitter - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Image Splitter.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Image Splitter.mak" CFG="Image Splitter - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Image Splitter - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "Image Splitter - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Image Splitter - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "$(QTDIR)\include" /I "..\..\src" /I "..\..\src\qt2" /I "..\..\..\unizone\libjpeg" /I "..\..\..\unizone\src" /I "..\..\..\unizone\src\muscle" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "UNICODE" /D "QT_DLL" /D "QJPEG_STATIC" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40b /d "NDEBUG"
# ADD RSC /l 0x40b /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib shlwapi.lib qt-mt230nc.lib qtmain.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"msvcrtd.lib" /libpath:"$(QTDIR)\lib" /libpath:"..\..\libjpeg\Release"

!ELSEIF  "$(CFG)" == "Image Splitter - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "$(QTDIR)\include" /I "..\..\src" /I "..\..\src\qt2" /I "..\..\..\unizone\libjpeg" /I "..\..\..\unizone\src" /I "..\..\..\unizone\src\muscle" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "UNICODE" /D "QT_DLL" /D "QJPEG_STATIC" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40b /d "_DEBUG"
# ADD RSC /l 0x40b /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib shlwapi.lib qt-mt230nc.lib qtmain.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"msvcrt.lib" /pdbtype:sept /libpath:"$(QTDIR)\lib" /libpath:"..\..\libjpeg\Debug"

!ENDIF 

# Begin Target

# Name "Image Splitter - Win32 Release"
# Name "Image Splitter - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\src\main.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\mainwindowimpl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\menubar.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\qt2\moc_mainwindow.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\qt2\moc_mainwindowimpl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\qt2\moc_menubar.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\qt2\moc_previewimpl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\previewimpl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\src\uenv.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\src\wstring.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\src\windows\wutil_msvc.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\src\mainwindowimpl.h

!IF  "$(CFG)" == "Image Splitter - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\branches\1.2\isplitter\src
InputPath=..\..\src\mainwindowimpl.h
InputName=mainwindowimpl

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Image Splitter - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\branches\1.2\isplitter\src
InputPath=..\..\src\mainwindowimpl.h
InputName=mainwindowimpl

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\menubar.h

!IF  "$(CFG)" == "Image Splitter - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\branches\1.2\isplitter\src
InputPath=..\..\src\menubar.h
InputName=menubar

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Image Splitter - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\branches\1.2\isplitter\src
InputPath=..\..\src\menubar.h
InputName=menubar

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\src\platform.h
# End Source File
# Begin Source File

SOURCE=..\..\src\previewimpl.h

!IF  "$(CFG)" == "Image Splitter - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\branches\1.2\isplitter\src
InputPath=..\..\src\previewimpl.h
InputName=previewimpl

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Image Splitter - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\branches\1.2\isplitter\src
InputPath=..\..\src\previewimpl.h
InputName=previewimpl

"$(InputDir)\qt2\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\qt2\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\src\uenv.h
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\src\wstring.h
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\src\wutil.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\..\src\isplitter.ico
# End Source File
# Begin Source File

SOURCE=..\..\src\qt2\isplitter.pro
# End Source File
# Begin Source File

SOURCE=..\..\src\isplitter.rc
# End Source File
# Begin Source File

SOURCE=..\..\src\mainwindow.ui

!IF  "$(CFG)" == "Image Splitter - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\unizone\branches\1.2\isplitter\src
InputPath=..\..\src\mainwindow.ui
InputName=mainwindow

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(InputDir)\qt2\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\qt2\$(InputName).cpp \
	

"$(InputDir)\qt2\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\qt2\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Image Splitter - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=\build\unizone\branches\1.2\isplitter\src
InputPath=..\..\src\mainwindow.ui
InputName=mainwindow

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(InputDir)\qt2\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\qt2\$(InputName).cpp \
	

"$(InputDir)\qt2\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\qt2\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "qjpeg Source Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jcapimin.c
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jcapistd.c
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jccoefct.c
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jccolor.c
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jcdctmgr.c
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jchuff.c
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jcinit.c
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jcmainct.c
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jcmarker.c
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jcmaster.c
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jcomapi.c
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jconfig.cfg
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jcparam.c
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jcphuff.c
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jcprepct.c
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jcsample.c
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jctrans.c
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jdapimin.c
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jdapistd.c
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jdatadst.c
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jdatasrc.c
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jdcoefct.c
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jdcolor.c
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jddctmgr.c
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jdhuff.c
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jdinput.c
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jdmainct.c
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jdmarker.c
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jdmaster.c
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jdmerge.c
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jdphuff.c
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jdpostct.c
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jdsample.c
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jdtrans.c
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jerror.c
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jfdctflt.c
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jfdctfst.c
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jfdctint.c
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jidctflt.c
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jidctfst.c
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jidctint.c
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jidctred.c
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jmemmgr.c
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jmemnobs.c
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jpegio.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jquant1.c
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jquant2.c
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jutils.c
# End Source File
# End Group
# Begin Group "qjpeg Header Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jchuff.h
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jconfig.h
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jdct.h
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jdhuff.h
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jerror.h
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jinclude.h
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jmemsys.h
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jmorecfg.h
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jpegint.h
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jpegio.h
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jpeglib.h
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\jversion.h
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\libjpeg\qjpeg.h
# End Source File
# End Group
# Begin Group "UIC Sources"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\qt2\mainwindow.cpp
# End Source File
# End Group
# Begin Group "UIC Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\src\qt2\mainwindow.h

!IF  "$(CFG)" == "Image Splitter - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\branches\1.2\isplitter\src\qt2
InputPath=..\..\src\qt2\mainwindow.h
InputName=mainwindow

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Image Splitter - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\build\unizone\branches\1.2\isplitter\src\qt2
InputPath=..\..\src\qt2\mainwindow.h
InputName=mainwindow

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "CRT Sources"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\unizone\src\windows\_filwbuf.c
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\src\windows\_getbuf.c
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\src\windows\resetstk.c
# End Source File
# Begin Source File

SOURCE=..\..\..\unizone\src\windows\vwsscanf.c
# End Source File
# End Group
# End Target
# End Project
