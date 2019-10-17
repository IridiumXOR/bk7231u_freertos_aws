REM rmdir /S /Q build
copy /y vendors\beken\boards\bk7231u\patches\iot_network_afr.c libraries\abstractions\platform\freertos\iot_network_afr.c
REM cmake -DVENDOR=beken -DBOARD=bk7231u -DCOMPILER=arm-gcc -DAFR_ENABLE_TESTS=1 -DAFR_TOOLCHAIN_PATH=D:/zhangheng/tools/env/tools/gnu_gcc/arm_gcc/mingw -S. -Bbuild -G"Unix Makefiles"
cmake -DVENDOR=beken -DBOARD=bk7231u -DCOMPILER=arm-gcc -DAFR_ENABLE_TESTS=0 -DAFR_METADATA_MODE=1 -S. -Bbuild -G"Unix Makefiles"
@PAUSE