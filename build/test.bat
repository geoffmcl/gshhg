@setlocal
@set TMPEXE=Release\gshhg.exe
@if NOT EXIST %TMPEXE% (
@echo Can NOT find %TMPEXE%! *** FIX ME ***
@exit /b 1
)

%TMPEXE% -b -30,63,-20,70 D:\DATA\GSHHG\gshhs_f.b -x BIKF.xg

%TMPEXE% -b -30,63,-10,70 D:\DATA\GSHHG\gshhs_f.b -x BIKF2.xg

