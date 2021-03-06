@setlocal
@set TMPBGN=%TIME%

@set TMPPRJ=gshhg
@echo Setup for 64-bit %TMPROJ% build
@set TMPLOG=bldlog-1.txt
@set TMPSRC=..
@set TMP3RD=D:\Projects\3rdParty.x64
@REM set BOOST_ROOT=X:\install\msvc100\boost
@REM if NOT EXIST %BOOST_ROOT%\nul goto NOBOOST

@call chkmsvc %TMPPRJ%
@REM call setupqt32
@REM if EXIST build-cmake.bat (
@REM call build-cmake
@REM if ERRORLEVEL 1 goto NOBCM
@REM )

@REM ###########################################
@REM NOTE: Specific install location
@REM ###########################################
@set TMPINST=%TMP3RD%
@REM ###########################################

@REM Nothing below need be touched..
@REM if NOT EXIST F:\nul goto NOXD
@REM if NOT EXIST %TMPSRC%\nul goto NOSRC
@if NOT EXIST %TMP3RD%\nul goto NO3RD
@REM if NOT EXIST %BOOST_ROOT%\nul goto NOBOOST
@if NOT EXIST %TMPSRC%\CMakeLists.txt goto NOSRC2

@set TMPOPTS=-DCMAKE_INSTALL_PREFIX=%TMPINST%
@REM set TMPOPTS=%TMPOPTS% -DCMAKE_PREFIX_PATH:PATH=%TMP3RD%
@set TMPOPTS=%TMPOPTS% -DBUILD_GMT2IMG:BOOL=ON
@REM set TMPOPTS=%TMPOPTS% -DBUILD_HDFDATA:BOOL=ON


:RPT
@if "%~1x" == "x" goto GOTCMD
@set TMPOPTS=%TMPOPTS% %1
@shift
@goto RPT
:GOTCMD

@echo Building %TMPPRJ% begin %TMPBGN% > %TMPLOG%
@echo All output to %TMPLOG%...

@REM echo Set ENV BOOST_ROOT=%BOOST_ROOT% >> %TMPLOG%

@echo Doing 'cmake %TMPSRC% %TMPOPTS%' out to %TMPLOG%
@echo Doing 'cmake %TMPSRC% %TMPOPTS%' >> %TMPLOG%
@cmake %TMPSRC% %TMPOPTS% >> %TMPLOG% 2>&1
@if ERRORLEVEL 1 goto ERR1

@echo Doing 'cmake --build . --config Debug'
@echo Doing 'cmake --build . --config Debug'  >> %TMPLOG%
@cmake --build . --config Debug  >> %TMPLOG% 2>&1
@if ERRORLEVEL 1 goto ERR2

@echo Doing: 'cmake --build . --config Release'
@echo Doing: 'cmake --build . --config Release'  >> %TMPLOG%
@cmake --build . --config Release  >> %TMPLOG% 2>&1
@if ERRORLEVEL 1 goto ERR3

@REM type %TMPLOG%
@fa4 "***" %TMPLOG%
@echo.
@echo Appears a successful build... see %TMPLOG%
@call elapsed %TMPBGN%
@echo.
@echo No install at this time
@goto END

@echo Proceed with an install - Debug then Release
@echo.
@echo *** CONTINUE? *** Only Ctrl+C aborts... all other keys continue...
@pause

@echo Doing: 'cmake --build . --config Debug --target INSTALL'
@echo Doing: 'cmake --build . --config Debug --target INSTALL' >> %TMPLOG%
@cmake --build . --config Debug --target INSTALL >> %TMPLOG% 2>&1
@if ERRORLEVEL 1 goto ERR4

@echo Doing: 'cmake --build . --config Release --target INSTALL'
@echo Doing: 'cmake --build . --config Release --target INSTALL' >> %TMPLOG% 2>&1
@echo cmake --build . --config Release --target INSTALL >> %TMPLOG% 2>&1
@if ERRORLEVEL 1 goto ERR5

@call elapsed %TMPBGN%
@echo.
@echo All done... build and install to %TMPINST%
@echo See %TMPLOG% for details...
@echo.
@goto END

:ERR1
@echo ERROR: Cmake config or geneation FAILED!
@goto ISERR

:ERR2
@echo ERROR: Cmake build Debug FAILED!
@goto ISERR

:ERR3
@echo ERROR: Cmake build Release FAILED!
@goto ISERR

:ERR4
@echo ERROR: Cmake install debug FAILED!
@goto ISERR

:ERR5
@echo ERROR: Cmake install release FAILED!
@goto ISERR

:NOXD
@echo Error: X:\ drive NOT found!
@goto ISERR
 
:NOSRC
@echo Error: No %TMPSRC% found!
@goto ISERR

:NO3RD
@echo Erro: No directory %TMP3RD% found!
@goto ISERR

:NOBOOST
@echo Error: Boost directory %BOOST_ROOT% not found!
@goto ISERR
 
:NOSRC2
@echo Error: File %TMPSRC%\CMakeLists.txt not found!
@goto ISERR

:NOBCM
@echo Error: Running build-cmake.bat caused an error!
@goto ISERR

:ISERR
@endlocal
@exit /b 1

:END
@endlocal
@exit /b 0

@REM eof

