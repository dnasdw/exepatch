IF "%~1"=="" GOTO NOARG
IF "%~2"=="" GOTO NOARG
IF "%~3"=="" GOTO NOARG
IF NOT EXIST "%~1" (CALL :FILENOTFOUND "%~1" & GOTO :EOF)
IF NOT EXIST "%~2" (CALL :FILENOTFOUND "%~2" & GOTO :EOF)

SET cwdir_exepatch=%CD%
SET rootdir_exepatch=%~dp0
MD "%rootdir_exepatch%backup"
MOVE /Y "%rootdir_exepatch%src\res\icon.ico" "%rootdir_exepatch%backup"
COPY "%~1" "%rootdir_exepatch%src\res\icon.ico"
MD "%rootdir_exepatch%project"
CD /D "%rootdir_exepatch%project"
cmake ..
cmake ..
cmake --build . --target install --config MinSizeRel --clean-first
CD /D "%cwdir_exepatch%"
COPY "%rootdir_exepatch%bin\exepatch.exe" /B + "%~2" /B "%~3.exe" /B
RD /S /Q "%rootdir_exepatch%bin"
RD /S /Q "%rootdir_exepatch%project"
MOVE /Y "%rootdir_exepatch%backup\icon.ico" "%rootdir_exepatch%src\res"
RD /S /Q "%rootdir_exepatch%backup"
GOTO :EOF

:NOARG
ECHO usage %0 ico_path patch_path name_prefix
GOTO ERROR

:FILENOTFOUND
ECHO %1 not found
GOTO ERROR

:ERROR
PAUSE
