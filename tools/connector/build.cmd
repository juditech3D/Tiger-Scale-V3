@echo off
rem Builds TigerScaleConnector.exe with the C# compiler that ships inside
rem Windows (.NET Framework 4.x). Nothing to install.
setlocal
set CSC=%WINDIR%\Microsoft.NET\Framework64\v4.0.30319\csc.exe
if not exist "%CSC%" set CSC=%WINDIR%\Microsoft.NET\Framework\v4.0.30319\csc.exe
if not exist "%CSC%" (echo The .NET Framework 4 compiler was not found. & exit /b 1)
cd /d "%~dp0"
"%CSC%" /nologo /optimize+ /target:winexe /platform:anycpu /out:TigerScaleConnector.exe /r:System.Windows.Forms.dll /r:System.Drawing.dll /r:System.Web.Extensions.dll TigerScaleConnector.cs
exit /b %ERRORLEVEL%
