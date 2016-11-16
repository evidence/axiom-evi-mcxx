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

AXIOMEVI_DIR=$AXIOMHOME
AXIOMEVI_GASNET_DIR=$AXIOMEVI_DIR/axiom-evi-gasnet
AXIOMEVI_OMPSS_DIR=$AXIOMEVI_DIR/ompss
AXIOMEVI_BR_DIR=$AXIOMEVI_DIR/axiom-evi-buildroot
AXIOMEVI_BR_HOST_DIR=$AXIOMEVI_BR_DIR/output/host
AXIOMEVI_BR_TRG_DIR=$AXIOMEVI_BR_DIR/output/target

OUTPUT=$AXIOMHOME/output

BUILD_ID='x86_64-unknown-linux-gnu'
TARGET_ID='aarch64-buildroot-linux-gnu'

../configure --prefix=$OUTPUT \
	     --build=$BUILD_ID --host=$BUILD_ID --target=$TARGET_ID \
	     --enable-ompss --with-nanox=$OUTPUT \
	     --disable-float128 --disable-int128 \
	     --disable-fortran-tests \
	     --disable-vectorization --disable-analysis \
	     "$@"
