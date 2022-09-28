#!/bin/sh
#ident	"@(#)sdb:cfront/scratch/bsd.sed	1.1"
echo "Fixing _iobuf structures:"
for f in */*..c
do
	echo $f
        sed -e '/__iobuf__base/s//&; int __iobuf__bufsiz/'  \
	-e '/char __iobuf__flag/s//short __iobuf__flag/' \
	-e '/_ctype/s//_ctype_/g' $f > temp$$
	mv temp$$ $f
done
