@echo off

cd Dependencies\Optick
premake vs2022
cd ..\..

premake5 vs2022