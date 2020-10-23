Make suer that the SD card config.txt contains:

disable_overscan=1

to avoid having black bars on monitor boundaries!

This solves the issue with the framebuffer size not being 1920x1080 on 1080p @60Hz monitor.
This also solves the mouse cursor not being in the right location; now it can move within the
whole framebuffer area / monitor area.

debugging USB mouse wheel

logger: Circle 43.1 started on Raspberry Pi Zero W
00:00:01.00 timer: SpeedFactor is 1.00
00:00:01.30 usbdev0-1: Device descriptor:
00:00:01.31 usbdev: Dumping 0x12 bytes starting at 0x509260
00:00:01.31 usbdev: 9260: 12 01 10 01 00 00 00 08-8A 24 67 83 00 01 01 02
00:00:01.32 usbdev: 9270: 00 01
00:00:01.38 usbdev0-1: Configuration descriptor:
00:00:01.39 usbdev: Dumping 0x3B bytes starting at 0x5092C0
00:00:01.39 usbdev: 92C0: 09 02 3B 00 02 01 00 A0-19 09 04 00 00 01 03 01
00:00:01.40 usbdev: 92D0: 02 00 09 21 11 01 21 01-22 8E 00 07 05 82 03 08
00:00:01.41 usbdev: 92E0: 00 04 09 04 01 00 01 03-01 01 00 09 21 11 01 21
00:00:01.41 usbdev: 92F0: 01 22 3B 00 07 05 81 03-08 00 0A 8F BE
00:00:01.42 usbdev0-1: Device ven248a-8367 found
00:00:01.42 usbdev0-1: Interface descriptor:
00:00:01.43 usbdev: Dumping 0x9 bytes starting at 0x5092C9
00:00:01.43 usbdev: 92C9: 09 04 00 00 01 03 01 02-00
00:00:01.44 usbdev0-1: Interface int3-1-2 found
00:00:01.44 usbdev0-1: Using device/interface int3-1-2
00:00:01.45 usbdev0-1: Interface descriptor:
00:00:01.45 usbdev: Dumping 0x9 bytes starting at 0x5092E2
00:00:01.46 usbdev: 92E2: 09 04 01 00 01 03 01 01-00
00:00:01.46 usbdev0-1: Interface int3-1-1 found
00:00:01.47 usbdev0-1: Using device/interface int3-1-1
00:00:01.53 umouse: HID descriptor
00:00:01.53 umouse: Dumping 0x9 bytes starting at 0x5092D2
00:00:01.53 umouse: 92D2: 09 21 11 01 21 01 22 8E-00
00:00:01.54 umouse: Report descriptor
00:00:01.55 umouse: Dumping 0x8E bytes starting at 0x511060
00:00:01.55 umouse: 1060: 05 01 09 02 A1 01 85 01-09 01 A1 00 05 09 19 01
00:00:01.56 umouse: 1070: 29 05 15 00 25 01 95 05-75 01 81 02 95 01 75 03
00:00:01.57 umouse: 1080: 81 01 05 01 09 30 09 31-15 81 25 7F 75 08 95 02
00:00:01.57 umouse: 1090: 81 06 09 38 15 81 25 7F-75 08 95 01 81 06 C0 C0
00:00:01.58 umouse: 10A0: 05 0C 09 01 A1 01 85 03-75 10 95 02 15 01 26 8C
00:00:01.59 umouse: 10B0: 02 19 01 2A 8C 02 81 00-C0 05 01 09 80 A1 01 85
00:00:01.59 umouse: 10C0: 04 75 02 95 01 15 01 25-03 09 82 09 81 09 83 81
00:00:01.60 umouse: 10D0: 60 75 06 81 03 C0 05 01-09 00 A1 01 85 05 06 00
00:00:01.60 umouse: 10E0: FF 09 01 15 81 25 7F 75-08 95 07 B1 02 C0 A2 D2
00:00:01.61 usbhid: Endpoint descriptor
00:00:01.61 usbhid: Dumping 0x7 bytes starting at 0x5092DB
00:00:01.62 usbhid: 92DB: 07 05 82 03 08 00 04
00:00:01.63 usbhid: Setting boot protocol, Interface number 0
00:00:01.63 usbhid: m_nMaxReportSize 3
00:00:01.64 usbhid: Endpoint descriptor
00:00:01.64 usbhid: Dumping 0x7 bytes starting at 0x5092F4
00:00:01.65 usbhid: 92F4: 07 05 81 03 08 00 0A
00:00:01.65 usbhid: Setting boot protocol, Interface number 1
00:00:01.66 usbhid: m_nMaxReportSize 8
00:00:01.67 dwroot: Device configured
00:00:01.67 kernel: Compile time: Oct 16 2020 18:41:20
00:00:01.67 kernel: Mouse attached


If REPORT_PROTOCOL is set in usbhiddevice.cpp then 5 bytes are received in the 
ReportHandler() on each mouse move/button press/wheel scroll. Bytes are:

0   0x01 constant?
1   1 - left button, 2 - right button, 4 - middle button
2   X displacement (+/- 127 max)
3   Y displacement (+/- 127 max)
4   1 - wheel up, -1 wheel down

Sample data in CUSBMouseDevice::ReportHandler():
00:00:32.92 umouse: 01   2   0   0   0
00:00:32.93 umouse: 01   2   0   0   0
00:00:32.94 umouse: 01   2   0   0   0
00:00:32.95 umouse: 01   2   0   0   1
00:00:32.96 umouse: 01   2   0   0   0
00:00:32.96 umouse: 01   2   0   0   0
00:00:32.97 umouse: 01   2   0   0   0
00:00:32.98 umouse: 01   2   0   0   0
00:00:32.99 umouse: 01   2   0   0   0
00:00:33.00 umouse: 01   0   0   0   0
00:00:33.15 umouse: 01   0   5   1   0
00:00:33.16 umouse: 01   0   3   0   0
00:00:33.17 umouse: 01   0   4   0   0
00:00:33.18 umouse: 01   0   7   0   0
00:00:33.19 umouse: 01   0   9   0   0
00:00:33.19 umouse: 01   0  10   0   0
00:00:33.20 umouse: 01   0  11   0   0
00:00:33.21 umouse: 01   0  10   0   0
00:00:33.22 umouse: 01   0  10   0   0

https://github.com/rsta2/circle/issues/142

https://github.com/smuehlst/circle-stdlib needs to be built for compiling imgui code (sscanf, atof, ..). This may be relaxed in the future,
or at least allow using only small subset of the stdlib (newlib) from circle-stdlib.

Config:

hinxx@obzen ~/Projects/baremetal-rpi/circle-stdlib $ cat Config.mk 
CC = /home/hinxx/Projects/baremetal-rpi/gcc-arm-9.2-2019.12-x86_64-arm-none-eabi/bin/arm-none-eabi-gcc
ARCH = -DAARCH=32 -mcpu=arm1176jzf-s -marm -mfpu=vfp -mfloat-abi=hard
TOOLPREFIX = /home/hinxx/Projects/baremetal-rpi/gcc-arm-9.2-2019.12-x86_64-arm-none-eabi/bin/arm-none-eabi-
NEWLIB_BUILD_DIR = /home/hinxx/Projects/baremetal-rpi/circle-stdlib/build/circle-newlib
NEWLIB_INSTALL_DIR = /home/hinxx/Projects/baremetal-rpi/circle-stdlib/install
CFLAGS_FOR_TARGET = -DAARCH=32 -mcpu=arm1176jzf-s -marm -mfpu=vfp -mfloat-abi=hard -Wno-parentheses
CPPFLAGS_FOR_TARGET = -I"/home/hinxx/Projects/baremetal-rpi/circle-stdlib/libs/circle/include" -I"/home/hinxx/Projects/baremetal-rpi/circle-stdlib/libs/circle/addon" -I"/home/hinxx/Projects/baremetal-rpi/circle-stdlib/include"
CC_FOR_TARGET = /home/hinxx/Projects/baremetal-rpi/gcc-arm-9.2-2019.12-x86_64-arm-none-eabi/bin/arm-none-eabi-gcc
CXX_FOR_TARGET = /home/hinxx/Projects/baremetal-rpi/gcc-arm-9.2-2019.12-x86_64-arm-none-eabi/bin/arm-none-eabi-g++
GCC_FOR_TARGET = /home/hinxx/Projects/baremetal-rpi/gcc-arm-9.2-2019.12-x86_64-arm-none-eabi/bin/arm-none-eabi-gcc
AR_FOR_TARGET = /home/hinxx/Projects/baremetal-rpi/gcc-arm-9.2-2019.12-x86_64-arm-none-eabi/bin/arm-none-eabi-gcc-ar
AS_FOR_TARGET = /home/hinxx/Projects/baremetal-rpi/gcc-arm-9.2-2019.12-x86_64-arm-none-eabi/bin/arm-none-eabi-gcc-as
LD_FOR_TARGET = /home/hinxx/Projects/baremetal-rpi/gcc-arm-9.2-2019.12-x86_64-arm-none-eabi/bin/arm-none-eabi-gcc-ld
RANLIB_FOR_TARGET = /home/hinxx/Projects/baremetal-rpi/gcc-arm-9.2-2019.12-x86_64-arm-none-eabi/bin/arm-none-eabi-gcc-ranlib
OBJCOPY_FOR_TARGET = /home/hinxx/Projects/baremetal-rpi/gcc-arm-9.2-2019.12-x86_64-arm-none-eabi/bin/arm-none-eabi-gcc-objcopy
OBJDUMP_FOR_TARGET = /home/hinxx/Projects/baremetal-rpi/gcc-arm-9.2-2019.12-x86_64-arm-none-eabi/bin/arm-none-eabi-gcc-objdump
NEWLIB_ARCH = arm-none-circle

hinxx@obzen ~/Projects/baremetal-rpi/circle-stdlib $ cat libs/circle/Config.mk 
RASPPI = 1
PREFIX = /home/hinxx/Projects/baremetal-rpi/gcc-arm-9.2-2019.12-x86_64-arm-none-eabi/bin/arm-none-eabi-
FLOAT_ABI = hard
STDLIB_SUPPORT = 3
STDDEF_INCPATH = "/home/hinxx/Projects/baremetal-rpi/gcc-arm-9.2-2019.12-x86_64-arm-none-eabi/bin/../lib/gcc/arm-none-eabi/9.2.1/include"
CFLAGS = -Wno-parentheses 
SERIALPORT = /dev/ttyUSB0
FLASHBAUD = 921600
USERBAUD = 115200

build!

hinxx@obzen ~/Projects/baremetal-rpi/circle-stdlib $ ll install/
total 16
drwxrwxr-x  3 hinxx hinxx 4096 Oct 13 22:30 ./
drwxrwxr-x 10 hinxx hinxx 4096 Oct 13 22:37 ../
drwxrwxr-x  4 hinxx hinxx 4096 Oct 13 22:30 arm-none-circle/
-rw-rw-r--  1 hinxx hinxx   40 Oct 13 21:04 .gitignore
hinxx@obzen ~/Projects/baremetal-rpi/circle-stdlib $ ll include/
total 32
drwxrwxr-x  3 hinxx hinxx 4096 Oct 13 21:04 ./
drwxrwxr-x 10 hinxx hinxx 4096 Oct 13 22:37 ../
-rw-rw-r--  1 hinxx hinxx  565 Oct 13 21:04 circle_glue.h
drwxrwxr-x  2 hinxx hinxx 4096 Oct 13 21:04 circle-mbedtls/
-rw-rw-r--  1 hinxx hinxx 8548 Oct 13 21:04 circle_stdlib_app.h
-rw-rw-r--  1 hinxx hinxx  295 Oct 13 21:04 wrap_fatfs.h

copy the ./install to circle/stdlib, copy ./include to circle/include
adjust makefiles to use sdlibs (see circle-stdlib/samples for that.


$ ./gcc-arm-9.2-2019.12-x86_64-arm-none-eabi/bin/arm-none-eabi-gcc -v -x c -E - 2>&1 
Using built-in specs.
COLLECT_GCC=./gcc-arm-9.2-2019.12-x86_64-arm-none-eabi/bin/arm-none-eabi-gcc
Target: arm-none-eabi
Configured with: /tmp/dgboter/bbs/rhev-vm10--rhe6x86_64/buildbot/rhe6x86_64--arm-none-eabi/build/src/gcc/configure --target=arm-none-eabi --prefix=/tmp/dgboter/bbs/rhev-vm10--rhe6x86_64/buildbot/rhe6x86_64--arm-none-eabi/build/build-arm-none-eabi/install// --with-gmp=/tmp/dgboter/bbs/rhev-vm10--rhe6x86_64/buildbot/rhe6x86_64--arm-none-eabi/build/build-arm-none-eabi/host-tools --with-mpfr=/tmp/dgboter/bbs/rhev-vm10--rhe6x86_64/buildbot/rhe6x86_64--arm-none-eabi/build/build-arm-none-eabi/host-tools --with-mpc=/tmp/dgboter/bbs/rhev-vm10--rhe6x86_64/buildbot/rhe6x86_64--arm-none-eabi/build/build-arm-none-eabi/host-tools --with-isl=/tmp/dgboter/bbs/rhev-vm10--rhe6x86_64/buildbot/rhe6x86_64--arm-none-eabi/build/build-arm-none-eabi/host-tools --disable-shared --disable-nls --disable-threads --disable-tls --enable-checking=release --enable-languages=c,c++,fortran --with-newlib --with-multilib-list=aprofile --with-pkgversion='GNU Toolchain for the A-profile Architecture 9.2-2019.12 (arm-9.10)' --with-bugurl=https://bugs.linaro.org/
Thread model: single
gcc version 9.2.1 20191025 (GNU Toolchain for the A-profile Architecture 9.2-2019.12 (arm-9.10)) 
COLLECT_GCC_OPTIONS='-v' '-E' '-mcpu=arm7tdmi' '-mfloat-abi=soft' '-marm' '-march=armv4t'
 /home/hinkokocevar/Projects/rpi/gcc-arm-9.2-2019.12-x86_64-arm-none-eabi/bin/../libexec/gcc/arm-none-eabi/9.2.1/cc1 -E -quiet -v -iprefix /home/hinkokocevar/Projects/rpi/gcc-arm-9.2-2019.12-x86_64-arm-none-eabi/bin/../lib/gcc/arm-none-eabi/9.2.1/ -D__USES_INITFINI__ - -mcpu=arm7tdmi -mfloat-abi=soft -marm -march=armv4t
ignoring nonexistent directory "/home/hinkokocevar/Projects/rpi/gcc-arm-9.2-2019.12-x86_64-arm-none-eabi/bin/../lib/gcc/arm-none-eabi/9.2.1/../../../../arm-none-eabi/sys-include"
ignoring duplicate directory "/home/hinkokocevar/Projects/rpi/gcc-arm-9.2-2019.12-x86_64-arm-none-eabi/bin/../lib/gcc/../../lib/gcc/arm-none-eabi/9.2.1/include"
ignoring duplicate directory "/home/hinkokocevar/Projects/rpi/gcc-arm-9.2-2019.12-x86_64-arm-none-eabi/bin/../lib/gcc/../../lib/gcc/arm-none-eabi/9.2.1/include-fixed"
ignoring nonexistent directory "/home/hinkokocevar/Projects/rpi/gcc-arm-9.2-2019.12-x86_64-arm-none-eabi/bin/../lib/gcc/../../lib/gcc/arm-none-eabi/9.2.1/../../../../arm-none-eabi/sys-include"
ignoring duplicate directory "/home/hinkokocevar/Projects/rpi/gcc-arm-9.2-2019.12-x86_64-arm-none-eabi/bin/../lib/gcc/../../lib/gcc/arm-none-eabi/9.2.1/../../../../arm-none-eabi/include"
#include "..." search starts here:
#include <...> search starts here:
 /home/hinkokocevar/Projects/rpi/gcc-arm-9.2-2019.12-x86_64-arm-none-eabi/bin/../lib/gcc/arm-none-eabi/9.2.1/include
 /home/hinkokocevar/Projects/rpi/gcc-arm-9.2-2019.12-x86_64-arm-none-eabi/bin/../lib/gcc/arm-none-eabi/9.2.1/include-fixed
 /home/hinkokocevar/Projects/rpi/gcc-arm-9.2-2019.12-x86_64-arm-none-eabi/bin/../lib/gcc/arm-none-eabi/9.2.1/../../../../arm-none-eabi/include
End of search list.



make -C test
make[1]: Entering directory '/home/hinkokocevar/Projects/rpi/circle/test'
CC = /home/hinkokocevar/Projects/rpi/gcc-arm-9.2-2019.12-x86_64-arm-none-eabi/bin/arm-none-eabi-gcc
CPP = /home/hinkokocevar/Projects/rpi/gcc-arm-9.2-2019.12-x86_64-arm-none-eabi/bin/arm-none-eabi-g++
LD = /home/hinkokocevar/Projects/rpi/gcc-arm-9.2-2019.12-x86_64-arm-none-eabi/bin/arm-none-eabi-ld
CPPFLAGS = -fno-exceptions -fno-rtti -nostdinc++ -DAARCH=32 -mcpu=arm1176jzf-s -marm -mfpu=vfp -mfloat-abi=hard -Wall -fsigned-char -ffreestanding -D__circle__ -DRASPPI=1 -DSTDLIB_SUPPORT=1 -D__VCCOREVER__=0x04000000 -U__unix__ -U__linux__  -I ../include -I ../addon -I ../app/lib -I ../addon/vc4 -I ../addon/vc4/interface/khronos/include -O2 -g -std=c++14 -Wno-aligned-new
CFLAGS = -DAARCH=32 -mcpu=arm1176jzf-s -marm -mfpu=vfp -mfloat-abi=hard -Wall -fsigned-char -ffreestanding -D__circle__ -DRASPPI=1 -DSTDLIB_SUPPORT=1 -D__VCCOREVER__=0x04000000 -U__unix__ -U__linux__  -I ../include -I ../addon -I ../app/lib -I ../addon/vc4 -I ../addon/vc4/interface/khronos/include -O2 -g
LDFLAGS = --section-start=.init=0x8000
AFLAGS = -DAARCH=32 -mcpu=arm1176jzf-s -marm -mfpu=vfp -mfloat-abi=hard -D__circle__ -DRASPPI=1 -DSTDLIB_SUPPORT=1 -D__VCCOREVER__=0x04000000 -U__unix__ -U__linux__  -I ../include -I ../addon -I ../app/lib -I ../addon/vc4 -I ../addon/vc4/interface/khronos/include -O2
LIBS = 
EXTRALIBS = /home/hinkokocevar/Projects/rpi/gcc-arm-9.2-2019.12-x86_64-arm-none-eabi/bin/../lib/gcc/arm-none-eabi/9.2.1/arm/v5te/hard/libgcc.a /home/hinkokocevar/Projects/rpi/gcc-arm-9.2-2019.12-x86_64-arm-none-eabi/bin/../lib/gcc/arm-none-eabi/9.2.1/../../../../arm-none-eabi/lib/arm/v5te/hard/libm.a
CRTBEGIN = 

$ arm-none-eabi-g++ -dM -E -x c++ - < /dev/null | sort

#define __ACCUM_EPSILON__ 0x1P-15K
#define __ACCUM_FBIT__ 15
#define __ACCUM_IBIT__ 16
#define __ACCUM_MAX__ 0X7FFFFFFFP-15K
#define __ACCUM_MIN__ (-0X1P15K-0X1P15K)
#define __APCS_32__ 1
#define __arm__ 1
#define __ARM_32BIT_STATE 1
#define __ARM_ARCH 4
#define __ARM_ARCH_4T__ 1
#define __ARM_ARCH_ISA_ARM 1
#define __ARM_ARCH_ISA_THUMB 1
#define __ARM_EABI__ 1
#define __ARMEL__ 1
#define __ARM_FEATURE_COPROC 1
#define __ARM_PCS 1
#define __ARM_SIZEOF_MINIMAL_ENUM 1
#define __ARM_SIZEOF_WCHAR_T 4
#define __ATOMIC_ACQ_REL 4
#define __ATOMIC_ACQUIRE 2
#define __ATOMIC_CONSUME 1
#define __ATOMIC_RELAXED 0
#define __ATOMIC_RELEASE 3
#define __ATOMIC_SEQ_CST 5
#define __BIGGEST_ALIGNMENT__ 8
#define __BYTE_ORDER__ __ORDER_LITTLE_ENDIAN__
#define __CHAR16_TYPE__ short unsigned int
#define __CHAR32_TYPE__ long unsigned int
#define __CHAR_BIT__ 8
#define __CHAR_UNSIGNED__ 1
#define __cplusplus 201402L
#define __cpp_aggregate_nsdmi 201304
#define __cpp_alias_templates 200704
#define __cpp_attributes 200809
#define __cpp_binary_literals 201304
#define __cpp_constexpr 201304
#define __cpp_decltype 200707
#define __cpp_decltype_auto 201304
#define __cpp_delegating_constructors 200604
#define __cpp_digit_separators 201309
#define __cpp_exceptions 199711
#define __cpp_generic_lambdas 201304
#define __cpp_hex_float 201603
#define __cpp_inheriting_constructors 201511
#define __cpp_init_captures 201304
#define __cpp_initializer_lists 200806
#define __cpp_lambdas 200907
#define __cpp_nsdmi 200809
#define __cpp_range_based_for 200907
#define __cpp_raw_strings 200710
#define __cpp_ref_qualifiers 200710
#define __cpp_return_type_deduction 201304
#define __cpp_rtti 199711
#define __cpp_runtime_arrays 198712
#define __cpp_rvalue_reference 200610
#define __cpp_rvalue_references 200610
#define __cpp_sized_deallocation 201309
#define __cpp_static_assert 200410
#define __cpp_threadsafe_static_init 200806
#define __cpp_unicode_characters 200704
#define __cpp_unicode_literals 200710
#define __cpp_user_defined_literals 200809
#define __cpp_variable_templates 201304
#define __cpp_variadic_templates 200704
#define __DA_FBIT__ 31
#define __DA_IBIT__ 32
#define __DBL_DECIMAL_DIG__ 17
#define __DBL_DENORM_MIN__ double(4.9406564584124654e-324L)
#define __DBL_DIG__ 15
#define __DBL_EPSILON__ double(2.2204460492503131e-16L)
#define __DBL_HAS_DENORM__ 1
#define __DBL_HAS_INFINITY__ 1
#define __DBL_HAS_QUIET_NAN__ 1
#define __DBL_MANT_DIG__ 53
#define __DBL_MAX_10_EXP__ 308
#define __DBL_MAX__ double(1.7976931348623157e+308L)
#define __DBL_MAX_EXP__ 1024
#define __DBL_MIN_10_EXP__ (-307)
#define __DBL_MIN__ double(2.2250738585072014e-308L)
#define __DBL_MIN_EXP__ (-1021)
#define __DEC128_EPSILON__ 1E-33DL
#define __DEC128_MANT_DIG__ 34
#define __DEC128_MAX__ 9.999999999999999999999999999999999E6144DL
#define __DEC128_MAX_EXP__ 6145
#define __DEC128_MIN__ 1E-6143DL
#define __DEC128_MIN_EXP__ (-6142)
#define __DEC128_SUBNORMAL_MIN__ 0.000000000000000000000000000000001E-6143DL
#define __DEC32_EPSILON__ 1E-6DF
#define __DEC32_MANT_DIG__ 7
#define __DEC32_MAX__ 9.999999E96DF
#define __DEC32_MAX_EXP__ 97
#define __DEC32_MIN__ 1E-95DF
#define __DEC32_MIN_EXP__ (-94)
#define __DEC32_SUBNORMAL_MIN__ 0.000001E-95DF
#define __DEC64_EPSILON__ 1E-15DD
#define __DEC64_MANT_DIG__ 16
#define __DEC64_MAX__ 9.999999999999999E384DD
#define __DEC64_MAX_EXP__ 385
#define __DEC64_MIN__ 1E-383DD
#define __DEC64_MIN_EXP__ (-382)
#define __DEC64_SUBNORMAL_MIN__ 0.000000000000001E-383DD
#define __DEC_EVAL_METHOD__ 2
#define __DECIMAL_DIG__ 17
#define __DEPRECATED 1
#define __DQ_FBIT__ 63
#define __DQ_IBIT__ 0
#define __ELF__ 1
#define __EXCEPTIONS 1
#define __FINITE_MATH_ONLY__ 0
#define __FLOAT_WORD_ORDER__ __ORDER_LITTLE_ENDIAN__
#define __FLT32_DECIMAL_DIG__ 9
#define __FLT32_DENORM_MIN__ 1.4012984643248171e-45F32
#define __FLT32_DIG__ 6
#define __FLT32_EPSILON__ 1.1920928955078125e-7F32
#define __FLT32_HAS_DENORM__ 1
#define __FLT32_HAS_INFINITY__ 1
#define __FLT32_HAS_QUIET_NAN__ 1
#define __FLT32_MANT_DIG__ 24
#define __FLT32_MAX_10_EXP__ 38
#define __FLT32_MAX__ 3.4028234663852886e+38F32
#define __FLT32_MAX_EXP__ 128
#define __FLT32_MIN_10_EXP__ (-37)
#define __FLT32_MIN__ 1.1754943508222875e-38F32
#define __FLT32_MIN_EXP__ (-125)
#define __FLT32X_DECIMAL_DIG__ 17
#define __FLT32X_DENORM_MIN__ 4.9406564584124654e-324F32x
#define __FLT32X_DIG__ 15
#define __FLT32X_EPSILON__ 2.2204460492503131e-16F32x
#define __FLT32X_HAS_DENORM__ 1
#define __FLT32X_HAS_INFINITY__ 1
#define __FLT32X_HAS_QUIET_NAN__ 1
#define __FLT32X_MANT_DIG__ 53
#define __FLT32X_MAX_10_EXP__ 308
#define __FLT32X_MAX__ 1.7976931348623157e+308F32x
#define __FLT32X_MAX_EXP__ 1024
#define __FLT32X_MIN_10_EXP__ (-307)
#define __FLT32X_MIN__ 2.2250738585072014e-308F32x
#define __FLT32X_MIN_EXP__ (-1021)
#define __FLT64_DECIMAL_DIG__ 17
#define __FLT64_DENORM_MIN__ 4.9406564584124654e-324F64
#define __FLT64_DIG__ 15
#define __FLT64_EPSILON__ 2.2204460492503131e-16F64
#define __FLT64_HAS_DENORM__ 1
#define __FLT64_HAS_INFINITY__ 1
#define __FLT64_HAS_QUIET_NAN__ 1
#define __FLT64_MANT_DIG__ 53
#define __FLT64_MAX_10_EXP__ 308
#define __FLT64_MAX__ 1.7976931348623157e+308F64
#define __FLT64_MAX_EXP__ 1024
#define __FLT64_MIN_10_EXP__ (-307)
#define __FLT64_MIN__ 2.2250738585072014e-308F64
#define __FLT64_MIN_EXP__ (-1021)
#define __FLT_DECIMAL_DIG__ 9
#define __FLT_DENORM_MIN__ 1.4012984643248171e-45F
#define __FLT_DIG__ 6
#define __FLT_EPSILON__ 1.1920928955078125e-7F
#define __FLT_EVAL_METHOD__ 0
#define __FLT_EVAL_METHOD_TS_18661_3__ 0
#define __FLT_HAS_DENORM__ 1
#define __FLT_HAS_INFINITY__ 1
#define __FLT_HAS_QUIET_NAN__ 1
#define __FLT_MANT_DIG__ 24
#define __FLT_MAX_10_EXP__ 38
#define __FLT_MAX__ 3.4028234663852886e+38F
#define __FLT_MAX_EXP__ 128
#define __FLT_MIN_10_EXP__ (-37)
#define __FLT_MIN__ 1.1754943508222875e-38F
#define __FLT_MIN_EXP__ (-125)
#define __FLT_RADIX__ 2
#define __FRACT_EPSILON__ 0x1P-15R
#define __FRACT_FBIT__ 15
#define __FRACT_IBIT__ 0
#define __FRACT_MAX__ 0X7FFFP-15R
#define __FRACT_MIN__ (-0.5R-0.5R)
#define __GCC_ATOMIC_BOOL_LOCK_FREE 1
#define __GCC_ATOMIC_CHAR16_T_LOCK_FREE 1
#define __GCC_ATOMIC_CHAR32_T_LOCK_FREE 1
#define __GCC_ATOMIC_CHAR_LOCK_FREE 1
#define __GCC_ATOMIC_INT_LOCK_FREE 1
#define __GCC_ATOMIC_LLONG_LOCK_FREE 1
#define __GCC_ATOMIC_LONG_LOCK_FREE 1
#define __GCC_ATOMIC_POINTER_LOCK_FREE 1
#define __GCC_ATOMIC_SHORT_LOCK_FREE 1
#define __GCC_ATOMIC_TEST_AND_SET_TRUEVAL 1
#define __GCC_ATOMIC_WCHAR_T_LOCK_FREE 1
#define __GCC_IEC_559 0
#define __GCC_IEC_559_COMPLEX 0
#define __GNUC__ 9
#define __GNUC_MINOR__ 2
#define __GNUC_PATCHLEVEL__ 1
#define __GNUC_STDC_INLINE__ 1
#define __GNUG__ 9
#define __GXX_ABI_VERSION 1013
#define __GXX_EXPERIMENTAL_CXX0X__ 1
#define __GXX_RTTI 1
#define __GXX_TYPEINFO_EQUALITY_INLINE 0
#define __GXX_WEAK__ 1
#define __HA_FBIT__ 7
#define __HA_IBIT__ 8
#define __has_include_next(STR) __has_include_next__(STR)
#define __has_include(STR) __has_include__(STR)
#define __HAVE_SPECULATION_SAFE_VALUE 1
#define __HQ_FBIT__ 15
#define __HQ_IBIT__ 0
#define __INT16_C(c) c
#define __INT16_MAX__ 0x7fff
#define __INT16_TYPE__ short int
#define __INT32_C(c) c ## L
#define __INT32_MAX__ 0x7fffffffL
#define __INT32_TYPE__ long int
#define __INT64_C(c) c ## LL
#define __INT64_MAX__ 0x7fffffffffffffffLL
#define __INT64_TYPE__ long long int
#define __INT8_C(c) c
#define __INT8_MAX__ 0x7f
#define __INT8_TYPE__ signed char
#define __INT_FAST16_MAX__ 0x7fffffff
#define __INT_FAST16_TYPE__ int
#define __INT_FAST16_WIDTH__ 32
#define __INT_FAST32_MAX__ 0x7fffffff
#define __INT_FAST32_TYPE__ int
#define __INT_FAST32_WIDTH__ 32
#define __INT_FAST64_MAX__ 0x7fffffffffffffffLL
#define __INT_FAST64_TYPE__ long long int
#define __INT_FAST64_WIDTH__ 64
#define __INT_FAST8_MAX__ 0x7fffffff
#define __INT_FAST8_TYPE__ int
#define __INT_FAST8_WIDTH__ 32
#define __INT_LEAST16_MAX__ 0x7fff
#define __INT_LEAST16_TYPE__ short int
#define __INT_LEAST16_WIDTH__ 16
#define __INT_LEAST32_MAX__ 0x7fffffffL
#define __INT_LEAST32_TYPE__ long int
#define __INT_LEAST32_WIDTH__ 32
#define __INT_LEAST64_MAX__ 0x7fffffffffffffffLL
#define __INT_LEAST64_TYPE__ long long int
#define __INT_LEAST64_WIDTH__ 64
#define __INT_LEAST8_MAX__ 0x7f
#define __INT_LEAST8_TYPE__ signed char
#define __INT_LEAST8_WIDTH__ 8
#define __INT_MAX__ 0x7fffffff
#define __INTMAX_C(c) c ## LL
#define __INTMAX_MAX__ 0x7fffffffffffffffLL
#define __INTMAX_TYPE__ long long int
#define __INTMAX_WIDTH__ 64
#define __INTPTR_MAX__ 0x7fffffff
#define __INTPTR_TYPE__ int
#define __INTPTR_WIDTH__ 32
#define __INT_WIDTH__ 32
#define __LACCUM_EPSILON__ 0x1P-31LK
#define __LACCUM_FBIT__ 31
#define __LACCUM_IBIT__ 32
#define __LACCUM_MAX__ 0X7FFFFFFFFFFFFFFFP-31LK
#define __LACCUM_MIN__ (-0X1P31LK-0X1P31LK)
#define __LDBL_DECIMAL_DIG__ 17
#define __LDBL_DENORM_MIN__ 4.9406564584124654e-324L
#define __LDBL_DIG__ 15
#define __LDBL_EPSILON__ 2.2204460492503131e-16L
#define __LDBL_HAS_DENORM__ 1
#define __LDBL_HAS_INFINITY__ 1
#define __LDBL_HAS_QUIET_NAN__ 1
#define __LDBL_MANT_DIG__ 53
#define __LDBL_MAX_10_EXP__ 308
#define __LDBL_MAX__ 1.7976931348623157e+308L
#define __LDBL_MAX_EXP__ 1024
#define __LDBL_MIN_10_EXP__ (-307)
#define __LDBL_MIN__ 2.2250738585072014e-308L
#define __LDBL_MIN_EXP__ (-1021)
#define __LFRACT_EPSILON__ 0x1P-31LR
#define __LFRACT_FBIT__ 31
#define __LFRACT_IBIT__ 0
#define __LFRACT_MAX__ 0X7FFFFFFFP-31LR
#define __LFRACT_MIN__ (-0.5LR-0.5LR)
#define __LLACCUM_EPSILON__ 0x1P-31LLK
#define __LLACCUM_FBIT__ 31
#define __LLACCUM_IBIT__ 32
#define __LLACCUM_MAX__ 0X7FFFFFFFFFFFFFFFP-31LLK
#define __LLACCUM_MIN__ (-0X1P31LLK-0X1P31LLK)
#define __LLFRACT_EPSILON__ 0x1P-63LLR
#define __LLFRACT_FBIT__ 63
#define __LLFRACT_IBIT__ 0
#define __LLFRACT_MAX__ 0X7FFFFFFFFFFFFFFFP-63LLR
#define __LLFRACT_MIN__ (-0.5LLR-0.5LLR)
#define __LONG_LONG_MAX__ 0x7fffffffffffffffLL
#define __LONG_LONG_WIDTH__ 64
#define __LONG_MAX__ 0x7fffffffL
#define __LONG_WIDTH__ 32
#define __NO_INLINE__ 1
#define __ORDER_BIG_ENDIAN__ 4321
#define __ORDER_LITTLE_ENDIAN__ 1234
#define __ORDER_PDP_ENDIAN__ 3412
#define __PRAGMA_REDEFINE_EXTNAME 1
#define __PTRDIFF_MAX__ 0x7fffffff
#define __PTRDIFF_TYPE__ int
#define __PTRDIFF_WIDTH__ 32
#define __QQ_FBIT__ 7
#define __QQ_IBIT__ 0
#define __REGISTER_PREFIX__ 
#define __SACCUM_EPSILON__ 0x1P-7HK
#define __SACCUM_FBIT__ 7
#define __SACCUM_IBIT__ 8
#define __SACCUM_MAX__ 0X7FFFP-7HK
#define __SACCUM_MIN__ (-0X1P7HK-0X1P7HK)
#define __SA_FBIT__ 15
#define __SA_IBIT__ 16
#define __SCHAR_MAX__ 0x7f
#define __SCHAR_WIDTH__ 8
#define __SFRACT_EPSILON__ 0x1P-7HR
#define __SFRACT_FBIT__ 7
#define __SFRACT_IBIT__ 0
#define __SFRACT_MAX__ 0X7FP-7HR
#define __SFRACT_MIN__ (-0.5HR-0.5HR)
#define __SHRT_MAX__ 0x7fff
#define __SHRT_WIDTH__ 16
#define __SIG_ATOMIC_MAX__ 0x7fffffff
#define __SIG_ATOMIC_MIN__ (-__SIG_ATOMIC_MAX__ - 1)
#define __SIG_ATOMIC_TYPE__ int
#define __SIG_ATOMIC_WIDTH__ 32
#define __SIZE_MAX__ 0xffffffffU
#define __SIZEOF_DOUBLE__ 8
#define __SIZEOF_FLOAT__ 4
#define __SIZEOF_INT__ 4
#define __SIZEOF_LONG__ 4
#define __SIZEOF_LONG_DOUBLE__ 8
#define __SIZEOF_LONG_LONG__ 8
#define __SIZEOF_POINTER__ 4
#define __SIZEOF_PTRDIFF_T__ 4
#define __SIZEOF_SHORT__ 2
#define __SIZEOF_SIZE_T__ 4
#define __SIZEOF_WCHAR_T__ 4
#define __SIZEOF_WINT_T__ 4
#define __SIZE_TYPE__ unsigned int
#define __SIZE_WIDTH__ 32
#define __SOFTFP__ 1
#define __SQ_FBIT__ 31
#define __SQ_IBIT__ 0
#define __STDC__ 1
#define __STDC_HOSTED__ 1
#define __STDC_UTF_16__ 1
#define __STDC_UTF_32__ 1
#define __TA_FBIT__ 63
#define __TA_IBIT__ 64
#define __THUMB_INTERWORK__ 1
#define __TQ_FBIT__ 127
#define __TQ_IBIT__ 0
#define __UACCUM_EPSILON__ 0x1P-16UK
#define __UACCUM_FBIT__ 16
#define __UACCUM_IBIT__ 16
#define __UACCUM_MAX__ 0XFFFFFFFFP-16UK
#define __UACCUM_MIN__ 0.0UK
#define __UDA_FBIT__ 32
#define __UDA_IBIT__ 32
#define __UDQ_FBIT__ 64
#define __UDQ_IBIT__ 0
#define __UFRACT_EPSILON__ 0x1P-16UR
#define __UFRACT_FBIT__ 16
#define __UFRACT_IBIT__ 0
#define __UFRACT_MAX__ 0XFFFFP-16UR
#define __UFRACT_MIN__ 0.0UR
#define __UHA_FBIT__ 8
#define __UHA_IBIT__ 8
#define __UHQ_FBIT__ 16
#define __UHQ_IBIT__ 0
#define __UINT16_C(c) c
#define __UINT16_MAX__ 0xffff
#define __UINT16_TYPE__ short unsigned int
#define __UINT32_C(c) c ## UL
#define __UINT32_MAX__ 0xffffffffUL
#define __UINT32_TYPE__ long unsigned int
#define __UINT64_C(c) c ## ULL
#define __UINT64_MAX__ 0xffffffffffffffffULL
#define __UINT64_TYPE__ long long unsigned int
#define __UINT8_C(c) c
#define __UINT8_MAX__ 0xff
#define __UINT8_TYPE__ unsigned char
#define __UINT_FAST16_MAX__ 0xffffffffU
#define __UINT_FAST16_TYPE__ unsigned int
#define __UINT_FAST32_MAX__ 0xffffffffU
#define __UINT_FAST32_TYPE__ unsigned int
#define __UINT_FAST64_MAX__ 0xffffffffffffffffULL
#define __UINT_FAST64_TYPE__ long long unsigned int
#define __UINT_FAST8_MAX__ 0xffffffffU
#define __UINT_FAST8_TYPE__ unsigned int
#define __UINT_LEAST16_MAX__ 0xffff
#define __UINT_LEAST16_TYPE__ short unsigned int
#define __UINT_LEAST32_MAX__ 0xffffffffUL
#define __UINT_LEAST32_TYPE__ long unsigned int
#define __UINT_LEAST64_MAX__ 0xffffffffffffffffULL
#define __UINT_LEAST64_TYPE__ long long unsigned int
#define __UINT_LEAST8_MAX__ 0xff
#define __UINT_LEAST8_TYPE__ unsigned char
#define __UINTMAX_C(c) c ## ULL
#define __UINTMAX_MAX__ 0xffffffffffffffffULL
#define __UINTMAX_TYPE__ long long unsigned int
#define __UINTPTR_MAX__ 0xffffffffU
#define __UINTPTR_TYPE__ unsigned int
#define __ULACCUM_EPSILON__ 0x1P-32ULK
#define __ULACCUM_FBIT__ 32
#define __ULACCUM_IBIT__ 32
#define __ULACCUM_MAX__ 0XFFFFFFFFFFFFFFFFP-32ULK
#define __ULACCUM_MIN__ 0.0ULK
#define __ULFRACT_EPSILON__ 0x1P-32ULR
#define __ULFRACT_FBIT__ 32
#define __ULFRACT_IBIT__ 0
#define __ULFRACT_MAX__ 0XFFFFFFFFP-32ULR
#define __ULFRACT_MIN__ 0.0ULR
#define __ULLACCUM_EPSILON__ 0x1P-32ULLK
#define __ULLACCUM_FBIT__ 32
#define __ULLACCUM_IBIT__ 32
#define __ULLACCUM_MAX__ 0XFFFFFFFFFFFFFFFFP-32ULLK
#define __ULLACCUM_MIN__ 0.0ULLK
#define __ULLFRACT_EPSILON__ 0x1P-64ULLR
#define __ULLFRACT_FBIT__ 64
#define __ULLFRACT_IBIT__ 0
#define __ULLFRACT_MAX__ 0XFFFFFFFFFFFFFFFFP-64ULLR
#define __ULLFRACT_MIN__ 0.0ULLR
#define __UQQ_FBIT__ 8
#define __UQQ_IBIT__ 0
#define __USACCUM_EPSILON__ 0x1P-8UHK
#define __USACCUM_FBIT__ 8
#define __USACCUM_IBIT__ 8
#define __USACCUM_MAX__ 0XFFFFP-8UHK
#define __USACCUM_MIN__ 0.0UHK
#define __USA_FBIT__ 16
#define __USA_IBIT__ 16
#define __USER_LABEL_PREFIX__ 
#define __USES_INITFINI__ 1
#define __USFRACT_EPSILON__ 0x1P-8UHR
#define __USFRACT_FBIT__ 8
#define __USFRACT_IBIT__ 0
#define __USFRACT_MAX__ 0XFFP-8UHR
#define __USFRACT_MIN__ 0.0UHR
#define __USQ_FBIT__ 32
#define __USQ_IBIT__ 0
#define __UTA_FBIT__ 64
#define __UTA_IBIT__ 64
#define __UTQ_FBIT__ 128
#define __UTQ_IBIT__ 0
#define __VERSION__ "9.2.1 20191025"
#define __VFP_FP__ 1
#define __WCHAR_MAX__ 0xffffffffU
#define __WCHAR_MIN__ 0U
#define __WCHAR_TYPE__ unsigned int
#define __WCHAR_UNSIGNED__ 1
#define __WCHAR_WIDTH__ 32
#define __WINT_MAX__ 0xffffffffU
#define __WINT_MIN__ 0U
#define __WINT_TYPE__ unsigned int
#define __WINT_WIDTH__ 32
