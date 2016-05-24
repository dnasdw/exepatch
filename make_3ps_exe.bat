IF "%~1"=="" GOTO NOARG
IF "%~2"=="" GOTO NOARG
IF "%~3"=="" GOTO NOARG
IF NOT EXIST "%~1" (CALL :FILENOTFOUND "%~1" & GOTO EOF)
IF NOT EXIST "%~2" (CALL :FILENOTFOUND "%~2" & GOTO EOF)

IF DEFINED VS140COMNTOOLS (
  SET GENERATOR="Visual Studio 14"
) ELSE IF DEFINED VS120COMNTOOLS (
  SET GENERATOR="Visual Studio 12"
) ELSE IF DEFINED VS110COMNTOOLS (
  SET GENERATOR="Visual Studio 11"
) ELSE IF DEFINED VS100COMNTOOLS (
  SET GENERATOR="Visual Studio 10"
) ELSE IF DEFINED VS90COMNTOOLS (
  SET GENERATOR="Visual Studio 9 2008"
)
IF NOT DEFINED GENERATOR (
  ECHO Can not find VC2008 or VC2010 or VC2012 or VC2013 or VC2015 installed!
  GOTO ERROR
)

SET rootdir=%~dp0
SET cwdir=%CD%
MD "%rootdir%backup"
MOVE /Y "%rootdir%src\res\icon.ico" "%rootdir%backup"
COPY "%~1" "%rootdir%src\res\icon.ico"
MD "%rootdir%project"
CD /D "%rootdir%project"
cmake -G %GENERATOR% ..
cmake ..
cmake --build . --target install --config MinSizeRel --clean-first
CD /D "%cwdir%"
COPY "%rootdir%bin\exepatch.exe" /B + "%~2" /B "%~3.exe" /B
RD /S /Q "%rootdir%bin"
RD /S /Q "%rootdir%project"
MOVE /Y "%rootdir%backup\icon.ico" "%rootdir%src\res"
RD /S /Q "%rootdir%backup"
GOTO :EOF

:NOARG
ECHO usage %0 ico_path patch_path name_prefix
GOTO ERROR

:FILENOTFOUND
ECHO %1 not found
GOTO ERROR

:ERROR
PAUSE
