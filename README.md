# BK7231U sdk/projects/beken/startup



## 编译

#### 搭建环境

编译基于`arm gcc windows`版本，比如`gcc-arm-none-eabi-5_4-2016q3-20160926-win32.exe`，安装成功后，将`gcc`路径加入`PATH`环境变量即可。



#### 编译固件

AWS FreeRTOS的编译包含测试和示例两种模式，在工程sdk目录下，运行如下命令可以编译示例工程



`cmake -DVENDOR=beken -DBOARD=bk7231u -DCOMPILER=arm-gcc -DAFR_TOOLCHAIN_PATH=D:/zhangheng/tools/env/tools/gnu_gcc/arm_gcc/mingw -S. -Bbuild -G"Unix Makefiles"`



运行如下命令可以编译测试工程（如：IDT）

`cmake -DVENDOR=beken -DBOARD=bk7231u -DCOMPILER=arm-gcc -DAFR_ENABLE_TESTS=1 -DAFR_TOOLCHAIN_PATH=D:/zhangheng/tools/env/tools/gnu_gcc/arm_gcc/mingw -S. -Bbuild -G"Unix Makefiles"`



> 也可以修改并运行批处理文件`bk7231_freertos_aws\sdk\generate_make.bat`，批处理默认生成示例工程的Makefile



#### 更新固件

固件的更新有三种方式，包括SPI烧录、UART升级、OTA升级，在介绍更新前，先了解一下BK7231U的接线，如下图：

![1567579303792](sdk/projects/beken/startup/update_overview.png)

BK7231U芯片与UART、SPI的连接图如上所示，其中：

1. BK7231U芯片
2. 外接天线
3. 专用SPI下载口，下载完整Flash程序
4. SPI下载板上的软/硬件SPI接口，一般用板里面那一组硬件SPI，速度稍快
5. UART2下载和测试口，下载Flash应用程序
6. 供电开关，电路板有标注ON和OFF
7. 供电接口：VCC连目标BK7231U模块板电源，可连3.3V/5V，默认为3.3V



##### SPI烧录

运行`bk7231_freertos_aws\sdk\projects\beken\tools\BKHidToolv2.5.2.exe`并选择`bk7231_freertos_aws\sdk\build\vendors\beken\boards\bk7231u\all.bin`，`all.bin`带`bootloader`和`bk7231u`固件

上图中的3即为SPI下载接口，下图为放大图，图中上半部分为硬件接口，下载板中有两个接口(”SW SPI”和”HW SPI”)，BK7231U推荐用”HW SPI”，框中P20，P21，P22，P23，CEN对应BK7231U芯片的对应PIN脚，按图1插入演示板即可。与硬件相对应的上位机中（下图下半部分）需要如图所示勾选“SPI HARD 硬件”，在软件左下脚会显示当前选择状态。如果下载板插了”SW SPI”，则上位机要选”SPI SOFT软件”。

![1567577926930](sdk/projects/beken/startup/update_spi_1.png)



完成硬件连接后，根据下图中的提示完成固件下载

![1567576895349](sdk/projects/beken/startup/update_spi_2.png)

##### UART升级

如果已经有`bootloader`（例如：已经通过SPI烧录过固件），可以使用`bk7231_freertos_aws\sdk\projects\beken\tools\bk_writer_V1.45_20180803_2M.exe`并选择`bk7231_freertos_aws\sdk\build\vendors\beken\boards\bk7231u\bk7231u_uart_*.bin`

![1567577182075](sdk/projects/beken/startup/update_uart_1.png)



步骤4中选择文件，当前脚本编译出来的UART升级文件名为`bk7231u_uart_0.0.1.bin`

![1566530229214](sdk/projects/beken/startup/update_uart_2.png)



步骤5完成后会提示操作成功，如果未成功请重新烧录，烧录时会自动复位开发板，如果未复位成功（会显示操作超时），可以在点击烧录后手动复位开发板

![1567499507943](sdk/projects/beken/startup/update_uart_3.png)



> 如果烧录时提示未找到端口，请确认端口是否正确以及是否被其他工具占用
>
> 如果下载前，勾选了"烧录后运行"，下载后会自动运行，否则，需要通过断开并重新连接UART板上的电源按钮复位（前面图中6所示位置）
>
> 由于IDT测试上电后会自动运行测试，建议手动复位



##### OTA升级

OTA分为本地OTA和远程OTA，命令如下：

暂略



## 验证



#### 启动`echo`服务

```shell
python echo.py -s -p 8883
python echo.py -s -t -p 443
```

`echo.py`是用python脚本实现的基于`TLS`的echo服务器，同时也可以作为客户端，命令如下

```shell
python echo.py -a 127.0.0.1
```

`echo.py`作为服务器时所使用的证书为`mosquitto_client_unauth.crt`，私钥为`mosquitto_client.key`。



#### 启动自动测试工具


修改`devicetester_afreertos_win/configs/userdata.json`中sourcePath及command

```json
{
  "sourcePath": "D:/bk7231_freertos_aws/sdk",
  "buildTool": {
    "name": "build",
    "version": "1.0",
    "command": [
      "D:/bk7231_freertos_aws/devicetester_afreertos_win/configs/script_templates/build.bat {{testData.sourcePath}}"
    ]
  },
  "flashTool":{
    "name": "burn",
    "version": "1.52",
    "command": [
      "..\\..\\..\\..\\..\\..\\sdk\\projects\\beken\\tools\\bk_writer_V1.52_20190709.exe {{testData.sourcePath}}"
    ]
  },
  "clientWifiConfig": {
    "wifiSSID": "TP-LINK_37B7",
    "wifiPassword": "1111117231",
    "wifiSecurityType": "eWiFiSecurityWPA2"
  },
  "testWifiConfig": {
    "wifiSSID": "FAST_5AC4",
    "wifiPassword": "12345678",
    "wifiSecurityType": "eWiFiSecurityWPA2"
  },
  "otaConfiguration":{
    "otaFirmwareFilePath":"..\\..\\..\\..\\..\\..\\sdk\\demos\\beken\\bk7231u\\bk7231u_uart_0.0.1.bin",
    "deviceFirmwareFileName":"bk7231u_uart_0.0.1.bin",
    "awsSignerPlatform":"AmazonFreeRTOS-BEKEN-7231U",
    "awsSignerCertificateArn":"<arn:partition:service:region:account-id:resource:qualifier>",
    "awsUntrustedSignerCertificateArn":"<arn:partition:service:region:account-id:resourcetype:resource:qualifier>",
    "awsSignerCertificateFileName":"<awsSignerCertificate-file-name>",
    "compileCodesignerCertificate": false
  }
}
```



修改`devicetester_afreertos_win/configs/script_templates/build.bat`中AFR_TOOLCHAIN_PATH路径

```bash
REM #!/bin/bash
REM # usage: script_templates/build.bat ablosute_path_to_build_path

cd /d %1
rmdir /s /q build
cmake -DVENDOR=beken -DBOARD=bk7231u -DCOMPILER=arm-gcc -DAFR_ENABLE_TESTS=1 -DAFR_TOOLCHAIN_PATH=D:/zhangheng/tools/env/tools/gnu_gcc/arm_gcc/mingw -S. -Bbuild -G"Unix Makefiles"
cd build
make
```




可以按group分别测试

```shell
devicetester_win_x86-64.exe run-suite --group-id CmakeBuildSystem --userdata userdata.json
devicetester_win_x86-64.exe run-suite --group-id FreeRTOSIntegrity --userdata userdata.json
devicetester_win_x86-64.exe run-suite --group-id FreeRTOSVersion --userdata userdata.json
devicetester_win_x86-64.exe run-suite --group-id FullBLE --userdata userdata.json
devicetester_win_x86-64.exe run-suite --group-id FullOTA --userdata userdata.json
devicetester_win_x86-64.exe run-suite --group-id FullPKCS11 --userdata userdata.json
devicetester_win_x86-64.exe run-suite --group-id FullMQTT --userdata userdata.json
devicetester_win_x86-64.exe run-suite --group-id FullSecureSockets --userdata userdata.json
devicetester_win_x86-64.exe run-suite --group-id FullTLS --userdata userdata.json
devicetester_win_x86-64.exe run-suite --group-id FullWiFi --userdata userdata.json
```

如果需要测试所有group，可以直接运行如下命令：

`devicetester_win_x86-64.exe run-suite --userdata userdata.json`
