@setlocal
@set MINLAT=-18
@set MINLON=-150
@set MAXLAT=-17
@set MAXLON=-149

@set TMPXG=tempbbox.xg
@set TMPGSHHG=D:\gshhg\bin\gshhs_f.b
@REM set TMPGSHHG=D:\gshhg\bin\gshhs_i.b
@REM can get nothing...
@REM set TMPGSHHG=D:\gshhg\bin\wdb_rivers_f.b
@REM set TMPGSHHG=D:\gshhg\bin\wdb_borders_f.b

@if NOT EXIST %TMPGSHHG% goto NOGSHHG
@set TMPEXE=Release\gshhg.exe
@if NOT EXIST %TMPEXE% goto NOEXE

@set TMPBBOX=%MINLON%,%MINLAT%,%MAXLON%,%MAXLAT%

%TMPEXE% -x %TMPXG% -b %TMPBBOX% %TMPGSHHG% %*

@goto END

:NOGSHHG
@echo Can NOT locate file %TMPGSHHG%! *** FIX ME ***
@goto END

:NOEXE
@echo Can NOT locate file %TMPEXE%! *** FIX ME ***
@goto END

:END
