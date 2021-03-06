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

#AXIOMEVI_DIR=$AXIOMHOME
#AXIOMEVI_GASNET_DIR=$AXIOMEVI_DIR/axiom-evi-gasnet
#AXIOMEVI_OMPSS_DIR=$AXIOMEVI_DIR/ompss
#AXIOMEVI_BR_DIR=$AXIOMEVI_DIR/axiom-evi-buildroot
#AXIOMEVI_BR_HOST_DIR=$AXIOMEVI_BR_DIR/output/host
#AXIOMEVI_BR_TRG_DIR=$AXIOMEVI_BR_DIR/output/target
#
#OUTPUT=$AXIOMHOME/output

if [ X"$P" = X1 ]; then

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
	     --build=$BUILD_ID --host=$TARGET_ID --target=$TARGET_ID \
	     --with-sysroot=$SYSROOT_DIR \
	     --enable-ompss --with-nanox=$SYSROOT_DIR/usr \
	     --disable-float128 --disable-int128 \
	     --disable-fortran-tests \
	     --disable-vectorization --disable-analysis \
	     "$@"
