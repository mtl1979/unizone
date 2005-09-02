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
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "$(QTDIR)\include" /I "..\..\libjpeg" /I "..\..\src" /I "..\..\src\muscle" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "UNICODE" /D "QT_DLL" /D "QJPEG_STATIC" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40b /d "NDEBUG"
# ADD RSC /l 0x40b /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib qt-mt3.lib qtmain.lib /nologo /subsystem:windows /machine:I386 /nodefaultlib:"msvcrtd.lib" /libpath:"$(QTDIR)\lib" /libpath:"..\..\libjpeg\Release"

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
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "$(QTDIR)\include" /I "..\..\libjpeg" /I "..\..\src" /I "..\..\src\muscle" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "UNICODE" /D "QT_DLL" /D "QJPEG_STATIC" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40b /d "_DEBUG"
# ADD RSC /l 0x40b /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib qt-mt3.lib qtmain.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"msvcrt.lib" /pdbtype:sept /libpath:"$(QTDIR)\lib" /libpath:"..\..\libjpeg\Debug"

!ENDIF 

# Begin Target

# Name "Image Splitter - Win32 Release"
# Name "Image Splitter - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE=.\mainwindow.cpp
# End Source File
# Begin Source File

SOURCE=.\mainwindowimpl.cpp
# End Source File
# Begin Source File

SOURCE=.\menubar.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_mainwindow.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_mainwindowimpl.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_menubar.cpp
# End Source File
# Begin Source File

SOURCE=.\moc_previewimpl.cpp
# End Source File
# Begin Source File

SOURCE=.\previewimpl.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\uenv.cpp
# End Source File
# Begin Source File

SOURCE=..\..\src\wstring.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\mainwindow.h

!IF  "$(CFG)" == "Image Splitter - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.
InputPath=.\mainwindow.h
InputName=mainwindow

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Image Splitter - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.
InputPath=.\mainwindow.h
InputName=mainwindow

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\mainwindowimpl.h

!IF  "$(CFG)" == "Image Splitter - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.
InputPath=.\mainwindowimpl.h
InputName=mainwindowimpl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Image Splitter - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.
InputPath=.\mainwindowimpl.h
InputName=mainwindowimpl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\menubar.h

!IF  "$(CFG)" == "Image Splitter - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.
InputPath=.\menubar.h
InputName=menubar

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Image Splitter - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.
InputPath=.\menubar.h
InputName=menubar

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\previewimpl.h

!IF  "$(CFG)" == "Image Splitter - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.
InputPath=.\previewimpl.h
InputName=previewimpl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "Image Splitter - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.
InputPath=.\previewimpl.h
InputName=previewimpl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\uenv.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\mainwindow.ui

!IF  "$(CFG)" == "Image Splitter - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.
InputPath=.\mainwindow.ui
InputName=mainwindow

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(InputDir)\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\$(InputName).cpp \
	

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "Image Splitter - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.
InputPath=.\mainwindow.ui
InputName=mainwindow

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
# Begin Group "qjpeg Source Files"

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

SOURCE=..\..\libjpeg\jconfig.cfg
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
# Begin Group "qjpeg Header Files"

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
# Begin Source File

SOURCE=..\..\libjpeg\qjpeg.h
# End Source File
# End Group
# End Target
# End Project
