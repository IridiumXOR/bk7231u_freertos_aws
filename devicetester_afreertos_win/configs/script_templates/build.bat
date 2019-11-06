REM #!/bin/bash
REM # usage: script_templates/build.bat ablosute_path_to_build_path

cd /d %1
rmdir /s /q build
cmake -DVENDOR=beken -DBOARD=bk7231u -DCOMPILER=arm-gcc -DAFR_ENABLE_TESTS=1 -S. -Bbuild -G"Unix Makefiles"
cd build
make -j8