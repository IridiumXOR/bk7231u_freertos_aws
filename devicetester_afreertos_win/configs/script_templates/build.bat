REM #!/bin/bash
REM # usage: script_templates/build.bat ablosute_path_to_build_path

cd /d %1
copy /y vendors\beken\boards\bk7231u\patches\iot_network_afr.c libraries\abstractions\platform\freertos\iot_network_afr.c
copy /y vendors\beken\boards\bk7231u\patches\bn_mul.h libraries\3rdparty\mbedtls\include\mbedtls\bn_mul.h
rmdir /s /q build
cmake -DVENDOR=beken -DBOARD=bk7231u -DCOMPILER=arm-gcc -DAFR_ENABLE_TESTS=1 -DAFR_TOOLCHAIN_PATH=D:/zhangheng/tools/env/tools/gnu_gcc/arm_gcc/mingw -S. -Bbuild -G"Unix Makefiles"
cd build
make