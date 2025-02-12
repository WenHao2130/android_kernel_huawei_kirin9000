export PATH="/home/remedgit/build/prebuilts/clang/host/linux-x86/clang-r416183b1/bin:$PATH"

echo
echo
echo "PATH SET OK!"

sleep 1

echo
echo
echo "Clear OUT!"

rm -rf ../../out/*

sleep 1

echo
echo
echo "COPY DECONFIG!"

sleep 1

make KBUILD_BUILD_USER=tejat KBUILD_BUILD_HOST=localhost ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- CROSS_COMPILE_ARM32=arm-linux-gnuabeihf- CC=clang LD=ld.lld HOSTCC=clang HOSTLD=ld.lld O=../../out KCFLAGS=-Wno-error -j8 merge_baltimore_defconfig

echo
echo
echo "START BUILD!"

sleep 1

make KBUILD_BUILD_USER=tejat KBUILD_BUILD_HOST=localhost ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- CROSS_COMPILE_ARM32=arm-linux-gnuabeihf- CC=clang LD=ld.lld HOSTCC=clang HOSTLD=ld.lld O=../../out KCFLAGS=-Wno-error -j8 tejat_deconfig

make KBUILD_BUILD_USER=tejat KBUILD_BUILD_HOST=localhost ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- CROSS_COMPILE_ARM32=arm-linux-gnuabeihf- CC=clang LD=ld.lld HOSTCC=clang HOSTLD=ld.lld O=../../out KCFLAGS=-Wno-error -j8

echo
echo
echo "BUILD END"
