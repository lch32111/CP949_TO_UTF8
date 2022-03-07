# CP949_TO_UTF8

Build `main.cpp, cp949_to_utf8_table.cpp, cp949_to_utf8_table.h`. And then, make use of `windows_cp949_converter.bat` to convert cp949 files at one time.



The batch file would copy the files in a source directory into a target director, and then it will convert each file into utf-8 formatted files. Do not include non-cp949 files with this converter. I made the batch file change only files in `.h, .hpp, .c, .cpp` . So You can change it by yourself if you need to change another extension files. 



usecase ex) 

```
./windows_cp949_convert.bat cp949_files_folder my_converted_files_folder converter.exe
```





