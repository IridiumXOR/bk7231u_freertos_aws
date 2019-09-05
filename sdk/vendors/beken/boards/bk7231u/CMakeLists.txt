cmake_minimum_required(VERSION 3.13)

set(vendor_prj_dir "${AFR_ROOT_DIR}/projects/beken/bk7231u")
set(vendor_sdk_dir "${AFR_VENDORS_DIR}/beken/beken378")
set(board_demos_dir "${CMAKE_CURRENT_LIST_DIR}/aws_demos")
set(board_tests_dir "${CMAKE_CURRENT_LIST_DIR}/aws_tests")
set(board_ports_dir "${CMAKE_CURRENT_LIST_DIR}/ports")

if(AFR_IS_TESTING)
    set(board_dir "${board_tests_dir}")
    set(exe_target aws_tests)
else()
    set(board_dir "${board_demos_dir}")
    set(exe_target aws_demos)
endif()

# -------------------------------------------------------------------------------------------------
# Amazon FreeRTOS Console metadata
# -------------------------------------------------------------------------------------------------
# Provide metadata for listing on Amazon FreeRTOS console.
afr_set_board_metadata(ID "BK7231U")
afr_set_board_metadata(DISPLAY_NAME "BK7231U")
afr_set_board_metadata(DESCRIPTION "Beken7231U Board for AmazonFreeRTOS")
afr_set_board_metadata(VENDOR_NAME "Beken")
afr_set_board_metadata(FAMILY_NAME "Wireless Microcontroller")
afr_set_board_metadata(IS_ACTIVE "TRUE")
afr_set_board_metadata(DATA_RAM_MEMORY "256KB")
afr_set_board_metadata(PROGRAM_MEMORY "2MB")
afr_set_board_metadata(CODE_SIGNER "AmazonFreeRTOS-Default")
afr_set_board_metadata(SUPPORTED_IDE "Eclipse")
afr_set_board_metadata(RECOMMENDED_IDE "Eclipse")
afr_set_board_metadata(IDE_Eclipse_NAME "Eclipse")
afr_set_board_metadata(IDE_Eclipse_COMPILER "GCC")
afr_set_board_metadata(IDE_Eclipse_RECOMMENDED "TRUE")
afr_set_board_metadata(IDE_Eclipse_PROJECT_LOCATION "")

# -------------------------------------------------------------------------------------------------
# Compiler settings
# -------------------------------------------------------------------------------------------------
# If you have multiple compiler supports, you can use AFR_TOOLCHAIN. Also might be a good idea to
# put your compiler settings to different files and just include them here, e.g.,
# include(<vendor_dir>/compilers/${AFR_TOOLCHAIN}.cmake)

# ==================== Example ====================

afr_mcu_port(compiler)

# Compile definitions/macros
target_compile_definitions(
    AFR::compiler::mcu_port
    INTERFACE $<$<COMPILE_LANGUAGE:C>:${compiler_defined_symbols}>
    -DMBEDTLS_USER_CONFIG_FILE="aws_tls_config.h"
    -DCFG_ENABLE_USER_APP
    -DCFG_SUPPORT_AWS
    #-DMULADDC_CANNOT_USE_R7
    # Needed by tinycbor to determine byte order
    -D__little_endian__=1
)

# Compiler flags
set(global-cflags-y
    "-mcpu=arm968e-s"
    "-marm"
    "-mthumb-interwork"
    "-mlittle-endian"
    "-g"
    "-Os"
    "-fdata-sections"
    "-ffunction-sections"
    #"-ffreestanding"
    #"-fno-strict-aliasing"
    #"-fno-common"
    "-gdwarf-2"
    "-nostdlib"
    "-std=c99"
    #"-Wall"
    #"-Wextra"
    #"-Wno-unused-parameter"
    #"-Wno-empty-body"
    #"-Wno-missing-field-initializers"
    #"-Wno-sign-compare"
    #"-Wno-type-limits"
)

target_compile_options(
    AFR::compiler::mcu_port
    INTERFACE ${global-cflags-y}
)

# Global include directories
target_include_directories(
    AFR::compiler::mcu_port
    INTERFACE
        "${board_dir}/config_files"
        "${board_demos_dir}/config_files"
        "${board_tests_dir}/config_files"
)

# Linker flags
# gcc specific extra linker flags
set(global-lflags-y
    "-mcpu=arm968e-s"
    "-nostartfiles"
    "-Wl,--gc-sections"
    "-Wl,--wrap,malloc"
    "-Wl,--wrap,free"
    "-Wl,--wrap,calloc"
    "-Wl,--wrap,realloc"
    "-Wl,--wrap,_malloc_r"
    "-Wl,--wrap,_free_r"
    "-Wl,--wrap,_calloc_r"
    "-Wl,--wrap,_realloc_r"
    #"-Wl,--wrap,printf"
    #"-Wl,--wrap,fprintf"
)

# target_link_options(
#     AFR::compiler::mcu_port
#     INTERFACE
#         -mcpu=cortex-m4
# )

# Xlinker flags
set (CMAKE_C_LINKER_WRAPPER_FLAG "-Xlinker" " ")
#add_link_options("LINKER:--undefined")
#add_link_options("LINKER:uxTopUsedPriority")
#add_link_options("LINKER:--cref")
add_link_options("LINKER:--gc-sections")
#add_link_options("LINKER:--defsym=_rom_data=64")
add_link_options("LINKER:-M")
add_link_options("LINKER:-Map")
add_link_options("LINKER:${exe_target}.map")

# Linker Script
set(global-linkerscript-y "${vendor_prj_dir}/bk7231u.ld")
target_link_options(
     AFR::compiler::mcu_port
     INTERFACE ${global-lflags-y}
)

# Library search path for linker
# target_link_directories(
#     AFR::compiler::mcu_port
#     INTERFACE
#         "<lib_path_0>"
#         "<lib_path_1>"
# )

# Libraries to link
target_link_libraries(
    AFR::compiler::mcu_port
    INTERFACE
        "${vendor_sdk_dir}/ip/libip_7231u.a"
        -Wl,--end-group
)

# -------------------------------------------------------------------------------------------------
# Amazon FreeRTOS portable layers
# -------------------------------------------------------------------------------------------------
# Define portable layer targets with afr_mcu_port(<module_name>). We will create an CMake
# INTERFACE IMPORTED target called AFR::${module_name}::mcu_port for you. You can use it with
# standard CMake functions like target_*. To better organize your files, you can define your own
# targets and use target_link_libraries(AFR::${module_name}::mcu_port INTERFACE <your_targets>)
# to provide the public interface you want expose.

# ==================== Example ====================
set(vendor ${AFR_VENDOR_NAME})
set(board ${AFR_BOARD_NAME})
set(portable_dir "portable/${vendor}/${board}")

# Kernel
afr_mcu_port(kernel)

target_sources(
     AFR::kernel::mcu_port
     INTERFACE
        "${vendor_sdk_dir}/app/app.c"
        "${vendor_sdk_dir}/app/ate_app.c"
        "${vendor_sdk_dir}/app/config/param_config.c"
        "${vendor_sdk_dir}/app/standalone-ap/sa_ap.c"
        "${vendor_sdk_dir}/app/standalone-station/sa_station.c"
        "${vendor_sdk_dir}/app/airkiss/airkiss.c"
        "${vendor_sdk_dir}/app/airkiss/airkiss_main.c"
        "${vendor_sdk_dir}/app/http/utils_httpc.c"
        "${vendor_sdk_dir}/app/http/utils_net.c"
        "${vendor_sdk_dir}/app/http/utils_timer.c"
        "${vendor_sdk_dir}/app/http/lite-log.c"
        "${vendor_sdk_dir}/app/tftp/tftpclient.c"
        "${vendor_sdk_dir}/demo/ieee802_11_demo.c"
        "${vendor_sdk_dir}/driver/common/dd.c"
        "${vendor_sdk_dir}/driver/common/drv_model.c"
        "${vendor_sdk_dir}/driver/dma/dma.c"
        "${vendor_sdk_dir}/driver/driver.c"
        "${vendor_sdk_dir}/driver/entry/arch_main.c"
        "${vendor_sdk_dir}/driver/fft/fft.c"
        "${vendor_sdk_dir}/driver/flash/flash.c"
        "${vendor_sdk_dir}/driver/general_dma/general_dma.c"
        "${vendor_sdk_dir}/driver/gpio/gpio.c"
        "${vendor_sdk_dir}/driver/i2s/i2s.c"
        "${vendor_sdk_dir}/driver/icu/icu.c"
        "${vendor_sdk_dir}/driver/intc/intc.c"
        "${vendor_sdk_dir}/driver/irda/irda.c"
        "${vendor_sdk_dir}/driver/macphy_bypass/mac_phy_bypass.c"
        "${vendor_sdk_dir}/driver/phy/phy_trident.c"
        "${vendor_sdk_dir}/driver/pwm/pwm.c"
        "${vendor_sdk_dir}/driver/pwm/mcu_ps_timer.c"
        "${vendor_sdk_dir}/driver/pwm/bk_timer.c"
        "${vendor_sdk_dir}/driver/rw_pub/rw_platf_pub.c"
        "${vendor_sdk_dir}/driver/saradc/saradc.c"
        "${vendor_sdk_dir}/driver/spi/spi.c"
        "${vendor_sdk_dir}/driver/spidma/spidma.c"
        "${vendor_sdk_dir}/driver/sys_ctrl/sys_ctrl.c"
        "${vendor_sdk_dir}/driver/uart/Retarget.c"
        "${vendor_sdk_dir}/driver/uart/uart.c"
        "${vendor_sdk_dir}/driver/wdt/wdt.c"
        "${vendor_sdk_dir}/func/func.c"
        "${vendor_sdk_dir}/func/bk7011_cal/bk7231_cal.c"
        "${vendor_sdk_dir}/func/bk7011_cal/manual_cal_bk7231.c"
        "${vendor_sdk_dir}/func/bk7011_cal/bk7231U_cal.c"
        "${vendor_sdk_dir}/func/bk7011_cal/manual_cal_bk7231U.c"
        "${vendor_sdk_dir}/func/joint_up/role_launch.c"
        "${vendor_sdk_dir}/func/joint_up/reconnect_startup.c"
        "${vendor_sdk_dir}/func/hostapd_intf/hostapd_intf.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/bk_patch/ddrv.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/bk_patch/signal.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/bk_patch/sk_intf.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/bk_patch/fake_socket.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/hostapd/main_none.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/crypto/aes-internal.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/crypto/aes-internal-dec.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/crypto/aes-internal-enc.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/crypto/aes-unwrap.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/crypto/aes-wrap.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/crypto/bk_md5.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/crypto/md5-internal.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/crypto/rc4.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/crypto/bk_sha1.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/crypto/sha1-internal.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/crypto/sha1-pbkdf2.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/crypto/sha1-prf.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/crypto/tls_none.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/ap/ap_config.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/ap/ap_drv_ops.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/ap/ap_list.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/ap/ap_mlme.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/ap/authsrv.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/ap/beacon.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/ap/bss_load.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/ap/dfs.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/ap/drv_callbacks.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/ap/eap_user_db.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/ap/hostapd.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/ap/hw_features.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/ap/ieee802_11_auth.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/ap/ieee802_11.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/ap/ieee802_11_ht.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/ap/ieee802_11_shared.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/ap/ieee802_1x.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/ap/pmksa_cache_auth.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/ap/sta_info.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/ap/tkip_countermeasures.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/ap/utils.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/ap/wmm.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/ap/wpa_auth.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/ap/wpa_auth_glue.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/ap/wpa_auth_ie.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/common/hw_features_common.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/common/ieee802_11_common.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/common/wpa_common.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/drivers/driver_beken.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/drivers/driver_common.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/drivers/drivers.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/eap_common/eap_common.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/eapol_auth/eapol_auth_sm.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/eap_server/eap_server.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/eap_server/eap_server_methods.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/l2_packet/l2_packet_none.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/rsn_supp/preauth.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/rsn_supp/wpa.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/rsn_supp/wpa_ie.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/utils/common.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/utils/eloop.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/utils/os_none.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/utils/wpabuf.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/wpa_supplicant/ap.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/wpa_supplicant/blacklist.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/wpa_supplicant/bss.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/wpa_supplicant/config.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/wpa_supplicant/config_none.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/wpa_supplicant/eap_register.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/wpa_supplicant/events.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/wpa_supplicant/main_supplicant.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/wpa_supplicant/notify.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/wpa_supplicant/wmm_ac.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/wpa_supplicant/wpa_scan.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/wpa_supplicant/wpas_glue.c"
        "${vendor_sdk_dir}/func/hostapd-2.5/wpa_supplicant/wpa_supplicant.c"
        "${vendor_sdk_dir}/func/misc/fake_clock.c"
        "${vendor_sdk_dir}/func/misc/target_util.c"
        "${vendor_sdk_dir}/func/misc/wdt_util.c"
        "${vendor_sdk_dir}/func/power_save/power_save.c"
        "${vendor_sdk_dir}/func/power_save/manual_ps.c"
        "${vendor_sdk_dir}/func/power_save/mcu_ps.c"
        "${vendor_sdk_dir}/func/rf_test/rx_sensitivity.c"
        "${vendor_sdk_dir}/func/rf_test/tx_evm.c"
        "${vendor_sdk_dir}/func/rwnx_intf/rw_ieee80211.c"
        "${vendor_sdk_dir}/func/rwnx_intf/rw_msdu.c"
        "${vendor_sdk_dir}/func/rwnx_intf/rw_msg_rx.c"
        "${vendor_sdk_dir}/func/rwnx_intf/rw_msg_tx.c"
        "${vendor_sdk_dir}/func/sim_uart/gpio_uart.c"
        "${vendor_sdk_dir}/func/sim_uart/pwm_uart.c"
        "${vendor_sdk_dir}/func/spidma_intf/spidma_intf.c"
        "${vendor_sdk_dir}/func/temp_detect/temp_detect.c"
        "${vendor_sdk_dir}/func/uart_debug/cmd_evm.c"
        "${vendor_sdk_dir}/func/uart_debug/cmd_help.c"
        "${vendor_sdk_dir}/func/uart_debug/cmd_reg.c"
        "${vendor_sdk_dir}/func/uart_debug/cmd_rx_sensitivity.c"
        "${vendor_sdk_dir}/func/uart_debug/command_line.c"
        "${vendor_sdk_dir}/func/uart_debug/command_table.c"
        "${vendor_sdk_dir}/func/uart_debug/udebug.c"
        "${vendor_sdk_dir}/func/user_driver/BkDriverFlash.c"
        "${vendor_sdk_dir}/func/user_driver/BkDriverGpio.c"
        "${vendor_sdk_dir}/func/user_driver/BkDriverPwm.c"
        "${vendor_sdk_dir}/func/user_driver/BkDriverUart.c"
        "${vendor_sdk_dir}/func/user_driver/BkDriverWdg.c"
        "${vendor_sdk_dir}/func/wlan_ui/wlan_cli.c"
        "${vendor_sdk_dir}/func/wlan_ui/wlan_ui.c"
        "${vendor_sdk_dir}/func/lwip_intf/dhcpd/dhcp-server.c"
        "${vendor_sdk_dir}/func/lwip_intf/dhcpd/dhcp-server-main.c"
        "${vendor_sdk_dir}/func/lwip_intf/tcp_server.c"
        "${vendor_sdk_dir}/func/lwip_intf/platform_stub.c"
        "${vendor_sdk_dir}/os/FreeRTOSv9.0.0/FreeRTOS/Source/portable/Keil/ARM968es/port.c"
        "${vendor_sdk_dir}/os/FreeRTOSv9.0.0/FreeRTOS/Source/portable/MemMang/heap_4.c"
        "${vendor_sdk_dir}/os/FreeRTOSv9.0.0/rtos_pub.c"
        "${vendor_sdk_dir}/os/mem_arch.c"
        "${vendor_sdk_dir}/os/str_arch.c"
        "${vendor_sdk_dir}/driver/entry/boot_handlers.S"
        "${vendor_sdk_dir}/driver/entry/boot_vectors.S"
        "${board_demos_dir}/kv/kvmgr.c"
        "${board_ports_dir}/lwip/ethernetif.c"
        "${board_ports_dir}/lwip/lwip_mem.c"
        "${board_ports_dir}/lwip/net.c"
        "${AFR_3RDPARTY_DIR}/lwip/src/portable/arch/sys_arch.c"
)

set(
    kernel_inc_directories
        "${vendor_sdk_dir}/app"
        "${vendor_sdk_dir}/app/config"
        "${vendor_sdk_dir}/app/http"
        "${vendor_sdk_dir}/app/standalone-station"
        "${vendor_sdk_dir}/app/standalone-ap"
        "${vendor_sdk_dir}/common"
        "${vendor_sdk_dir}/demo"
        "${vendor_sdk_dir}/ip/common"
        "${vendor_sdk_dir}/ip/ke/"
        "${vendor_sdk_dir}/ip/mac/"
        "${vendor_sdk_dir}/ip/lmac/src/hal"
        "${vendor_sdk_dir}/ip/lmac/src/mm"
        "${vendor_sdk_dir}/ip/lmac/src/ps"
        "${vendor_sdk_dir}/ip/lmac/src/rd"
        "${vendor_sdk_dir}/ip/lmac/src/rwnx"
        "${vendor_sdk_dir}/ip/lmac/src/rx"
        "${vendor_sdk_dir}/ip/lmac/src/scan"
        "${vendor_sdk_dir}/ip/lmac/src/sta"
        "${vendor_sdk_dir}/ip/lmac/src/tx"
        "${vendor_sdk_dir}/ip/lmac/src/vif"
        "${vendor_sdk_dir}/ip/lmac/src/rx/rxl"
        "${vendor_sdk_dir}/ip/lmac/src/tx/txl"
        "${vendor_sdk_dir}/ip/lmac/src/p2p"
        "${vendor_sdk_dir}/ip/lmac/src/chan"
        "${vendor_sdk_dir}/ip/lmac/src/td"
        "${vendor_sdk_dir}/ip/lmac/src/tpc"
        "${vendor_sdk_dir}/ip/lmac/src/tdls"
        "${vendor_sdk_dir}/ip/umac/src/mesh"
        "${vendor_sdk_dir}/ip/umac/src/rc"
        "${vendor_sdk_dir}/ip/umac/src/apm"
        "${vendor_sdk_dir}/ip/umac/src/bam"
        "${vendor_sdk_dir}/ip/umac/src/llc"
        "${vendor_sdk_dir}/ip/umac/src/me"
        "${vendor_sdk_dir}/ip/umac/src/rxu"
        "${vendor_sdk_dir}/ip/umac/src/scanu"
        "${vendor_sdk_dir}/ip/umac/src/sm"
        "${vendor_sdk_dir}/ip/umac/src/txu"
        "${vendor_sdk_dir}/driver/include"
        "${vendor_sdk_dir}/driver/common/reg"
        "${vendor_sdk_dir}/driver/entry"
        "${vendor_sdk_dir}/driver/dma"
        "${vendor_sdk_dir}/driver/intc"
        "${vendor_sdk_dir}/driver/phy"
        "${vendor_sdk_dir}/driver/rc_beken"
        "${vendor_sdk_dir}/driver/flash"
        "${vendor_sdk_dir}/driver/rw_pub"
        "${vendor_sdk_dir}/driver/common/reg"
        "${vendor_sdk_dir}/driver/common"
        "${vendor_sdk_dir}/driver/uart"
        "${vendor_sdk_dir}/driver/sys_ctrl"
        "${vendor_sdk_dir}/driver/gpio"
        "${vendor_sdk_dir}/driver/general_dma"
        "${vendor_sdk_dir}/driver/spidma"
        "${vendor_sdk_dir}/driver/icu"
        "${vendor_sdk_dir}/func/include"
        "${vendor_sdk_dir}/func/rf_test"
        "${vendor_sdk_dir}/func/user_driver"
        "${vendor_sdk_dir}/func/power_save"
        "${vendor_sdk_dir}/func/uart_debug"
        "${vendor_sdk_dir}/func/ethernet_intf"
        "${vendor_sdk_dir}/func/hostapd-2.5/hostapd"
        "${vendor_sdk_dir}/func/hostapd-2.5/bk_patch"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/utils"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/ap"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/common"
        "${vendor_sdk_dir}/func/hostapd-2.5/src/drivers"
        "${vendor_sdk_dir}/func/hostapd-2.5/src"
        "${vendor_sdk_dir}/func/temp_detect"
        "${vendor_sdk_dir}/func/spidma_intf"
        "${vendor_sdk_dir}/func/rwnx_intf"
        "${vendor_sdk_dir}/func/joint_up"
        "${vendor_sdk_dir}/os/include"
        "${vendor_sdk_dir}/os/FreeRTOSv9.0.0"
        "${vendor_sdk_dir}/os/FreeRTOSv9.0.0/FreeRTOS/Source/portable/Keil/ARM968es"
        "${board_demos_dir}/demos"
        "${board_demos_dir}/demos/net/uart1_tcp_server"
        "${board_demos_dir}"
        "${board_ports_dir}/lwip"
        "${AFR_KERNEL_DIR}/include"
        "${AFR_3RDPARTY_DIR}/jsmn"
        "${AFR_3RDPARTY_DIR}/lwip/src/include"
        "${AFR_3RDPARTY_DIR}/lwip/src/include/lwip"
        "${AFR_3RDPARTY_DIR}/lwip/src/portable"
        "${AFR_3RDPARTY_DIR}/mbedtls/include/mbedtls"
        "${AFR_3RDPARTY_DIR}/pkcs11"
        "${AFR_3RDPARTY_DIR}/tinycbor"
        "${AFR_3RDPARTY_DIR}/unity/src/"
        "${AFR_MODULES_ABSTRACTIONS_DIR}/wifi/include"
        "${AFR_MODULES_C_SDK_DIR}/standard/common/include"
        "$<IF:${AFR_IS_TESTING},${AFR_TESTS_DIR},${AFR_DEMOS_DIR}>/include"
)

# Global include directories
target_include_directories(
     AFR::kernel::mcu_port
     INTERFACE ${kernel_inc_directories}
)

# WiFi
afr_mcu_port(wifi)
target_sources(
    AFR::wifi::mcu_port
    INTERFACE "${board_ports_dir}/wifi/aws_wifi.c"
)

# PKCS11
afr_mcu_port(pkcs11)
target_sources(
    AFR::pkcs11::mcu_port
    INTERFACE "${board_ports_dir}/pkcs11/aws_pkcs11_pal.c"
)

# Link to AFR::pkcs11_mbedtls if you want to use default implementation based on mbedtls.
target_link_libraries(
    AFR::pkcs11::mcu_port
    INTERFACE AFR::pkcs11_mbedtls
)

# Secure sockets
afr_mcu_port(secure_sockets)
target_link_libraries(
    AFR::secure_sockets::mcu_port
    INTERFACE AFR::tls 3rdparty::lwip
)
target_sources(
    AFR::secure_sockets::mcu_port
    INTERFACE "${AFR_MODULES_DIR}/abstractions/secure_sockets/lwip/aws_secure_sockets.c"
)

# -------------------------------------------------------------------------------------------------
# Amazon FreeRTOS demos and tests
# -------------------------------------------------------------------------------------------------
# We require you to define at least demos and tests executable targets. Available demos and tests
# will be automatically enabled by us. You need to provide other project settings such as linker
# scripts and post build commands.

# ==================== Example ====================
set(CMAKE_EXECUTABLE_SUFFIX ".axf")

set(default_modules AFR::utils AFR::secure_sockets_lwip)
afr_glob_src(app_src RECURSE DIRECTORY "${board_dir}/application_code")
add_executable(${exe_target} ${app_src})

target_link_libraries(
    ${exe_target}
    PRIVATE
        -Wl,--start-group
        ${default_modules}
        ${link_extra_flags}
        -T${global-linkerscript-y}
)

# Convert afx file to bin file
configure_file("${AFR_ROOT_DIR}/projects/beken/tools/beken_packager.exe" "beken_packager.exe" COPYONLY)
configure_file("${AFR_ROOT_DIR}/projects/beken/tools/config.json" "config.json" COPYONLY)
configure_file("${vendor_prj_dir}/bootloader_bk7231u_aws_v1.0.bin" "bootloader_bk7231u_aws_v1.0.bin" COPYONLY)

afr_find_compiler(AFR_COMPILER_OBJCOPY arm-none-eabi-objcopy)
afr_find_compiler(AFR_COMPILER_SIZE arm-none-eabi-size)
set(output_bin_file "$<TARGET_FILE_DIR:${exe_target}>/${exe_target}.bin")
set(output_axf_file "$<TARGET_FILE_DIR:${exe_target}>/${exe_target}.axf")

add_custom_command(
    TARGET ${exe_target} POST_BUILD
    COMMAND "echo" "Running Post-build step"
    COMMAND "${CMAKE_COMMAND}" -E remove "bk7231u.bin"
    COMMAND "${AFR_COMPILER_OBJCOPY}" -O binary "${output_axf_file}" "bk7231u.bin"
    COMMAND "${AFR_COMPILER_SIZE}" "${output_axf_file}"
    COMMAND "beken_packager.exe"
)

set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "bk7231u.bin" "${exe_target}.map")
