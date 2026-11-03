@echo off
cd /d %~dp0

set CompilerFlags=-Od -nologo -MD -Oi -W4 -FC -Z7
set LinkerFlags=-incremental:no -opt:ref

 call vcvarsall x64
IF NOT EXIST BuildOutput mkdir BuildOutput
pushd BuildOutput || goto bad

REM cl %CompilerFlags% ../hello.c /P /C /I../extern\SDA3\include 
REM cl %CompilerFlags% /Tphello.i /link %LinkerFlags%  || goto bad

cl %CompilerFlags% ../main.cpp /P /C /I../../extern/SDA3/include /I../../extern/imgui /I../../extern/imgui/backends /I../../extern/implot
cl %CompilerFlags% /Tpmain.i /link %LinkerFlags% ../../extern-lib/SDL3.lib Ws2_32.lib || goto bad
xcopy /E /I /Y /Q ..\asset asset 

goto end
:bad
	echo ERRORLEVEL:
	echo %ERRORLEVEL%
	popd

:end
popd
echo ERRORLEVEL:
echo %ERRORLEVEL%