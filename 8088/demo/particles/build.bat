@echo off
set oldpath=%path%
set path=C:\Windows\system32;C:\Windows;C:\Windows\System32\Wbem;D:\t\Utils;D:\msvc\bin
D:\msvc\bin\cl /nologo /W3 /DNDEBUG /Ox /AT /D_DOS /c /Ic:\msvc\include /Fa particle.cpp
D:\msvc\bin\link c:\msvc\lib\crtcom.lib particle.obj, particle.com, particle.map, c:\msvc\lib\slibce.lib c:\msvc\lib\oldnames.lib,  /NOLOGO /ONERROR:NOEXE /NOI /TINY /NOE
set path=%oldpath%
