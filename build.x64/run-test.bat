@setlocal
@REM For ????
@set MINLAT=-18
@set MINLON=-150
@set MAXLAT=-17
@set MAXLON=-149

@REM For YSSY-LFPG
@set MINLAT=-35
@set MINLON=2
@set MAXLAT=50
@set MAXLON=152

@REM for VHHH LFPG
@set MINLAT=22.31
@set MAXLAT=49.01
@set MINLON=2.55
@set MAXLON=113.92

@set TMPXG=temp-VH-PG-c.xg
@REM set TMPXG=temp-VH-PG-f.xg
@REM set TMPXG=temp-Y-Lb-f.xg
@set TMPGSHHG=D:\gshhg\bin\gshhs_c.b
@REM set TMPGSHHG=D:\gshhg\bin\gshhs_f.b
@REM set TMPGSHHG=D:\gshhg\bin\gshhs_i.b
@REM can get nothing...
@REM set TMPGSHHG=D:\gshhg\bin\wdb_rivers_f.b
@rem set TMPGSHHG=D:\gshhg\bin\wdb_borders_f.b

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
