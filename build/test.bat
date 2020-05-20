@setlocal
@set TMPEXE=Release\gshhg.exe
@if NOT EXIST %TMPEXE% (
@echo Can NOT find %TMPEXE%! *** FIX ME ***
@exit /b 1
)
@set TMPDATA=D:\DATA\GSHHG\gshhs_f.b
@if NOT EXIST %TMPDATA% (
@echo Can NOT find %TMPDATA%! *** FIX ME ***
@exit /b 1
)



%TMPEXE% -b -30,63,-20,70 %TMPDATA% -x BIKF.xg

%TMPEXE% -b -30,63,-10,70 %TMPDATA% -x BIKF2.xg

