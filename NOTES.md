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
