# Microsoft Developer Studio Project File - Name="vc6" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=vc6 - Win32 Debug Unicode
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "vc6.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "vc6.mak" CFG="vc6 - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "vc6 - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "vc6 - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "vc6 - Win32 Release Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "vc6 - Win32 Debug Unicode" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "vc6 - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "VC6_EXPORTS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\Framework" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "VC6_EXPORTS" /D "_WINXP" /D "SEPARATE_UNICODE" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib mpr.lib /nologo /dll /machine:I386 /def:"plugin.def" /out:"Release/FileCopyExA.dll"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "vc6 - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "VC6_EXPORTS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\Framework" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "VC6_EXPORTS" /D "_WINXP" /D "SEPARATE_UNICODE" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 mpr.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /def:"plugin.def" /out:"Debug/FileCopyExA.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "vc6 - Win32 Release Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "vc6___Win32_Release_Unicode"
# PROP BASE Intermediate_Dir "vc6___Win32_Release_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_Unicode"
# PROP Intermediate_Dir "Release_Unicode"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "..\Framework" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "VC6_EXPORTS" /D "_WINXP" /D "SEPARATE_UNICODE" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\Framework" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "VC6_EXPORTS" /D "UNICODE" /D "_UNICODE" /D "_WINXP" /D "SEPARATE_UNICODE" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib mpr.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib mpr.lib /nologo /dll /machine:I386 /def:"plugin.def" /out:"Release_Unicode/FileCopyExW.dll"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "vc6 - Win32 Debug Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "vc6___Win32_Debug_Unicode"
# PROP BASE Intermediate_Dir "vc6___Win32_Debug_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_Unicode"
# PROP Intermediate_Dir "Debug_Unicode"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\Framework" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "VC6_EXPORTS" /D "_WINXP" /D "SEPARATE_UNICODE" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\Framework" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "VC6_EXPORTS" /D "UNICODE" /D "_UNICODE" /D "_WINXP" /D "SEPARATE_UNICODE" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 mpr.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 mpr.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /def:"plugin.def" /out:"far\Plugins\filecopyex\FileCopyExW.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "vc6 - Win32 Release"
# Name "vc6 - Win32 Debug"
# Name "vc6 - Win32 Release Unicode"
# Name "vc6 - Win32 Debug Unicode"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\FileCopyEx\Api.cpp
# ADD CPP /Yu"StdHdr.h"
# End Source File
# Begin Source File

SOURCE=..\FileCopyEx\Classes.cpp
# ADD CPP /Yu"StdHdr.h"
# End Source File
# Begin Source File

SOURCE=..\FileCopyEx\Config.cpp
# ADD CPP /Yu"StdHdr.h"
# End Source File
# Begin Source File

SOURCE=..\FileCopyEx\CopyProgress.cpp
# ADD CPP /Yu"StdHdr.h"
# End Source File
# Begin Source File

SOURCE=..\FileCopyEx\Engine.cpp
# ADD CPP /Yu"StdHdr.h"
# End Source File
# Begin Source File

SOURCE=..\Framework\FAR\FarDialog.cpp
# ADD CPP /Yu"StdHdr.h"
# End Source File
# Begin Source File

SOURCE=..\Framework\FAR\FarDlgClass.cpp
# ADD CPP /Yu"StdHdr.h"
# End Source File
# Begin Source File

SOURCE=..\Framework\FAR\FarDlgObject.cpp
# ADD CPP /Yu"StdHdr.h"
# End Source File
# Begin Source File

SOURCE=..\Framework\FAR\FarPanel.cpp
# ADD CPP /Yu"StdHdr.h"
# End Source File
# Begin Source File

SOURCE=..\Framework\FAR\FarPlugin.cpp
# ADD CPP /Yu"StdHdr.h"
# End Source File
# Begin Source File

SOURCE=..\Framework\FAR\FarProgress.cpp
# ADD CPP /Yu"StdHdr.h"
# End Source File
# Begin Source File

SOURCE=..\Framework\FAR\FarRegistry.cpp
# ADD CPP /Yu"StdHdr.h"
# End Source File
# Begin Source File

SOURCE=..\Framework\FAR\FarUI.cpp
# ADD CPP /Yu"StdHdr.h"
# End Source File
# Begin Source File

SOURCE=..\FileCopyEx\FileCopyEx.cpp
# ADD CPP /Yu"StdHdr.h"
# End Source File
# Begin Source File

SOURCE=..\Framework\FileUtils.cpp
# ADD CPP /Yu"StdHdr.h"
# End Source File
# Begin Source File

SOURCE=..\Framework\FwCommon.cpp
# ADD CPP /Yu"StdHdr.h"
# End Source File
# Begin Source File

SOURCE=..\Framework\Heap.cpp
# ADD CPP /Yu"StdHdr.h"
# End Source File
# Begin Source File

SOURCE=..\Framework\LowLevelStr.cpp
# ADD CPP /Yu"StdHdr.h"
# End Source File
# Begin Source File

SOURCE=..\FileCopyEx\Main.cpp
# ADD CPP /Yu"StdHdr.h"
# End Source File
# Begin Source File

SOURCE=..\Framework\Object.cpp
# ADD CPP /Yu"StdHdr.h"
# End Source File
# Begin Source File

SOURCE=..\Framework\ObjString.cpp
# ADD CPP /Yu"StdHdr.h"
# End Source File
# Begin Source File

SOURCE=.\prec_hdr.cpp

!IF  "$(CFG)" == "vc6 - Win32 Release"

# ADD CPP /Yc"StdHdr.h"

!ELSEIF  "$(CFG)" == "vc6 - Win32 Debug"

# ADD CPP /Yc"StdHdr.h"

!ELSEIF  "$(CFG)" == "vc6 - Win32 Release Unicode"

# ADD BASE CPP /Yc"StdHdr.h"
# ADD CPP /Yc"StdHdr.h"

!ELSEIF  "$(CFG)" == "vc6 - Win32 Debug Unicode"

# ADD BASE CPP /Yc"StdHdr.h"
# ADD CPP /Yc"StdHdr.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\Framework\Properties.cpp
# ADD CPP /Yu"StdHdr.h"
# End Source File
# Begin Source File

SOURCE=..\Framework\Store.cpp
# ADD CPP /Yu"StdHdr.h"
# End Source File
# Begin Source File

SOURCE=..\Framework\StringList.cpp
# ADD CPP /Yu"StdHdr.h"
# End Source File
# Begin Source File

SOURCE=..\Framework\StrUtils.cpp
# ADD CPP /Yu"StdHdr.h"
# End Source File
# Begin Source File

SOURCE=..\FileCopyEx\TempPanel.cpp
# ADD CPP /Yu"StdHdr.h"
# End Source File
# Begin Source File

SOURCE=..\FileCopyEx\Utils.cpp
# ADD CPP /Yu"StdHdr.h"
# End Source File
# Begin Source File

SOURCE=..\Framework\ValueList.cpp
# ADD CPP /Yu"StdHdr.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\FileCopyEx\Api.h
# End Source File
# Begin Source File

SOURCE=..\Framework\Array.h
# End Source File
# Begin Source File

SOURCE=..\FileCopyEx\Classes.h
# End Source File
# Begin Source File

SOURCE=..\FileCopyEx\Common.h
# End Source File
# Begin Source File

SOURCE=..\FileCopyEx\CopyProgress.h
# End Source File
# Begin Source File

SOURCE=..\FileCopyEx\Engine.h
# End Source File
# Begin Source File

SOURCE=..\FileCopyEx\EngineTools.h
# End Source File
# Begin Source File

SOURCE=..\Framework\FAR\FarDialog.h
# End Source File
# Begin Source File

SOURCE=..\Framework\FAR\FarDlgClass.h
# End Source File
# Begin Source File

SOURCE=..\Framework\FAR\FarDlgObject.h
# End Source File
# Begin Source File

SOURCE=..\Framework\FAR\FarPanel.h
# End Source File
# Begin Source File

SOURCE=..\Framework\FAR\FarPlugin.h
# End Source File
# Begin Source File

SOURCE=..\Framework\FAR\FarProgress.h
# End Source File
# Begin Source File

SOURCE=..\Framework\FAR\FarRegistry.h
# End Source File
# Begin Source File

SOURCE=..\Framework\FAR\FarUI.h
# End Source File
# Begin Source File

SOURCE=..\FileCopyEx\FileCopyEx.h
# End Source File
# Begin Source File

SOURCE=..\Framework\FileUtils.h
# End Source File
# Begin Source File

SOURCE=..\Framework\FwCommon.h
# End Source File
# Begin Source File

SOURCE=..\Framework\Heap.h
# End Source File
# Begin Source File

SOURCE=..\Framework\LowLevelStr.h
# End Source File
# Begin Source File

SOURCE=..\Framework\Object.h
# End Source File
# Begin Source File

SOURCE=..\Framework\ObjString.h
# End Source File
# Begin Source File

SOURCE=..\Framework\Properties.h
# End Source File
# Begin Source File

SOURCE=..\FileCopyEx\resource.h
# End Source File
# Begin Source File

SOURCE=..\Framework\StdHdr.h
# End Source File
# Begin Source File

SOURCE=..\Framework\Store.h
# End Source File
# Begin Source File

SOURCE=..\Framework\StringList.h
# End Source File
# Begin Source File

SOURCE=..\Framework\StrUtils.h
# End Source File
# Begin Source File

SOURCE=..\FileCopyEx\TempPanel.h
# End Source File
# Begin Source File

SOURCE=..\Framework\ValueList.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\FileCopyEx\FileCopyEx.rc
# End Source File
# End Group
# End Target
# End Project
