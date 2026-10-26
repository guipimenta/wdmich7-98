@echo off
rem Build HDA.sys with the Windows XP DDK (build 2600), free build.
rem portcls.lib and stdunk.lib come prebuilt in the DDK lib directory.
rem Usage: build.cmd [DDK root]   (default C:\WINDDK\2600)

setlocal
set DDKROOT=%1
if "%DDKROOT%"=="" set DDKROOT=C:\WINDDK\2600

call %DDKROOT%\bin\setenv.bat %DDKROOT% fre
cd /d %~dp0src
build -cZ
