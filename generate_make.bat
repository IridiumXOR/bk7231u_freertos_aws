REM rmdir /S /Q build
REM cmake -DVENDOR=beken -DBOARD=bk7231u -DCOMPILER=arm-gcc -DAFR_ENABLE_TESTS=1 -S. -Bbuild -G"Unix Makefiles"
cmake -DVENDOR=beken -DBOARD=bk7231u -DCOMPILER=arm-gcc -DAFR_ENABLE_TESTS=0 -S. -Bbuild -G"Unix Makefiles"
@PAUSE