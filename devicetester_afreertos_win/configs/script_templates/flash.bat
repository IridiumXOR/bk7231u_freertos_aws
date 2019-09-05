#!/bin/bash

cd /d %1

start /wait projects\beken\tools\bk_writer_V1.52_20190709.exe -tBK7231 -p%2@1000000 -fbuild\vendors\beken\boards\bk7231u\bk7231u_uart_0.0.1.bin@00011000

echo %errorlevel%
