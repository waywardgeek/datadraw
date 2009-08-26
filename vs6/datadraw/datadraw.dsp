# Microsoft Developer Studio Project File - Name="datadraw" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=datadraw - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "datadraw.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "datadraw.mak" CFG="datadraw - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "datadraw - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "datadraw - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "datadraw - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX- /O2 /I "..\..\util" /I "." /I "..\..\src" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "_WINDOWS" /D "_WIN32" /YX /FD /c
# ADD BASE RSC /l 0x407 /d "NDEBUG"
# ADD RSC /l 0x407 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "datadraw - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX- /ZI /Od /I "..\..\util" /I "." /I "..\..\src" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "_WINDOWS" /D "_WIN32" /YX /FD /GZ /c
# ADD BASE RSC /l 0x407 /d "_DEBUG"
# ADD RSC /l 0x407 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "datadraw - Win32 Release"
# Name "datadraw - Win32 Debug"
# Begin Source File

SOURCE=..\..\src\dv.h
# End Source File
# Begin Source File

SOURCE=..\..\src\dvadmin.c
# End Source File
# Begin Source File

SOURCE=..\..\src\dvbuild.c
# End Source File
# Begin Source File

SOURCE=..\..\src\dvdatabase.c
# End Source File
# Begin Source File

SOURCE=..\..\src\dvdatabase.h
# End Source File
# Begin Source File

SOURCE=..\..\src\dvgenc.c
# End Source File
# Begin Source File

SOURCE=..\..\src\dvgenerate.c
# End Source File
# Begin Source File

SOURCE=..\..\src\dvgenh.c
# End Source File
# Begin Source File

SOURCE=..\..\src\dvlexwrap.c
# End Source File
# Begin Source File

SOURCE=..\..\src\dvmain.c
# End Source File
# Begin Source File

SOURCE=.\dvparse.c
# End Source File
# Begin Source File

SOURCE=.\dvparse.h
# End Source File
# Begin Source File

SOURCE=..\..\src\dvparse.y

!IF  "$(CFG)" == "datadraw - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
InputPath=..\..\src\dvparse.y

"dvparse.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	bison -l -d -p dv -o dvparse.c -b dv ../../src/dvparse.y

# End Custom Build

!ELSEIF  "$(CFG)" == "datadraw - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
InputPath=..\..\src\dvparse.y

"dvparse.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	bison -l -d -p dv -o dvparse.c -b dv ../../src/dvparse.y

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\dvread.c
# End Source File
# Begin Source File

SOURCE=.\dvscan.c
# End Source File
# Begin Source File

SOURCE=..\..\src\dvscan.l

!IF  "$(CFG)" == "datadraw - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
InputPath=..\..\src\dvscan.l

"dvscan.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex -L -f -Pdvlex -odvscan.c $(InputPath)

# End Custom Build

!ELSEIF  "$(CFG)" == "datadraw - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build
InputPath=..\..\src\dvscan.l

"dvscan.c" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	flex -L -f -Pdvlex -odvscan.c $(InputPath)

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\src\dvutil.c
# End Source File
# Begin Source File

SOURCE=..\..\src\dvversion.h
# End Source File
# End Target
# End Project
