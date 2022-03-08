# CP949_TO_UTF8

`main.cpp, cp949_to_utf8_table.cpp, cp949_to_utf8_table.h`를 빌드하세요. 그러고 나서, cp949 파일들을 한 번에 변경하기 위해 `windows_cp949_converter.bat`를 이용하세요.

배치 파일은 소스 폴더에서 타겟 폴더로 파일들을 처음에 복사할 것이고, 그러고나서 각 파일을 utf-8 포맷의 파일들로 변경할 것입니다. 이 converter에 cp949 포맷이 아닌 파일을 포함시키지 마세요. 배치 파일에서는 `.h, .hpp, .c, .cpp`의 파일들만 바꾸도록 했습니다. 그래서 다른 확장자 파일을 변경할 필요가 있다면 스스로 바꾸도록 하세요.

사용예시)

```bash
./windows_cp949_convert.bat cp949_files_folder my_converted_files_folder converter.exe
```



src폴더의 `utf8_to_cp949_main.cpp`를 사용하면 바꾸었던 utf8 포맷으로 바꾸었던 cp949 파일을 다시 cp949 포맷으로 바꿀 수 있습니다.



---

Build `main.cpp, cp949_to_utf8_table.cpp, cp949_to_utf8_table.h`. And then, make use of `windows_cp949_converter.bat` to convert cp949 files at one time.

The batch file would copy the files first in a source directory into a target directory , and then it will convert each file into utf-8 formatted files. Do not include non-cp949 files with this converter. I made the batch file change only files in `.h, .hpp, .c, .cpp` . So You can change it by yourself if you need to change another extension files. 

usecase ex) 

```bash
./windows_cp949_convert.bat cp949_files_folder my_converted_files_folder converter.exe
```



you can use `utf8_to_cp949_main.cpp` to convert back the file into the cp949 format, which you converted into the utf8 format.
