@echo off

set source_dir=%1
set target_dir=%2
set converter_exe=%3

IF %1.==. GOTO No1
IF %2.==. GOTO No2
IF %3.==. GOTO No3

xcopy "%source_dir%" "%target_dir%" /s /e

for /R "%target_dir%" %%G in (*.h *.hpp *.c *.cpp) do (
     @echo %%G
     call %converter_exe% "%%G" "%%G"
)


GOTO End1

:No1
  ECHO No param 1 : Source Directory
GOTO End1
:No2
  ECHO No param 2 : Target Directory
GOTO End1
:No3
  ECHO No param 3 : Converter Exe File Name
GOTO End1

:End1