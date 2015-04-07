@setlocal

@set TMPEXE=Release\gshhg.exe
@if NOT EXIST %TMPEXE% goto NOEXE

@set TMPCMD=
:RPT
@if "%~1x" == "x" goto GOTCMD
@set TMPCMD=%TMPCMD% %1
@shift
@goto RPT
:GOTCMD

%TMPEXE% %TMPCMD%

@goto END

:NOEXE
@echo Can NOT locate %TMPEXE%! Has it been built?
:END
