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

../configure --prefix=$PREFIX \
	     --enable-ompss --with-nanox=/usr/local \
	     --disable-float128 --disable-int128 \
	     --disable-fortran-tests \
	     --disable-vectorization --disable-analysis \
	     "$@"
