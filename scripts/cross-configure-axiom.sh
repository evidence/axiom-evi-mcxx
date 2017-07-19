#!/bin/sh

# autoreconf -fiv

[ -z "$AXIOMHOME" ] && AXIOMHOME=$HOME/axiom-evi
for arg in "$@";do
    echo "$arg" | grep -q '^--with-axiomhome=' \
	&& AXIOMHOME=$(echo "$arg" | sed 's#^--with-axiomhome=##')
done
[ -f "$AXIOMHOME/scripts/start_qemu.sh" ] || {
    echo "AXIOMHOME shell variable or --with-axiomhome are improperly set!"
    echo "(axiom home used is '$AXIOMHOME')"
    exit 255;
}

if [ X"$FS" = Xseco ]; then

    TARGET_DIR=$(realpath ${ROOTFS})
    #SYSROOT_DIR=$(realpath ${LINARO}/staging)
    SYSROOT_DIR=$(realpath ${ROOTFS})
    HOST_DIR=$(realpath ${LINARO}/host)

    BUILD_ID='x86_64-unknown-linux-gnu'
    TARGET_ID='aarch64-linux-gnu'

    export PATH=$HOST_DIR/usr/bin:$PATH

else
    
    OUTPUT_DIR=$AXIOMHOME/output
    TARGET_DIR=$(realpath ${OUTPUT_DIR}/target)
    SYSROOT_DIR=$(realpath ${OUTPUT_DIR}/staging)
    HOST_DIR=$(realpath ${OUTPUT_DIR}/host)

    BUILD_ID='x86_64-unknown-linux-gnu'
    TARGET_ID='aarch64-buildroot-linux-gnu'
    
fi

[ -z "$PREFIX" ] && PREFIX=$(HOST_DIR)/usr

../configure --prefix=$PREFIX \
	     --build=$BUILD_ID --host=$BUILD_ID --target=$TARGET_ID \
	     --enable-ompss --with-nanox=$SYSROOT_DIR/usr \
	     --disable-float128 --disable-int128 \
	     --disable-fortran-tests \
	     --disable-vectorization --disable-analysis \
	     "$@"
