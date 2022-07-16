#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)proto:i386/at386/cmd/mini_kernel.sh	1.3.2.1"

cleanup () {
mv $ROOT/etc/conf/cf.d/.jdevice $ROOT/etc/conf/cf.d/mdevice > /dev/null 2>&1
mv $ROOT/etc/conf/cf.d/.jstune $ROOT/etc/conf/cf.d/stune > /dev/null 2>&1
mv $ROOT/etc/conf/sdevice.d/.j/* $ROOT/etc/conf/sdevice.d > /dev/null 2>&1
rmdir $ROOT/etc/conf/sdevice.d/.j > /dev/null 2>&1
rm -f $ROOT/etc/conf/sdevice.d/ramd $ROOT/etc/conf/pack.d/ramd/space.c
}

turnoff () {
cd $ROOT/etc/conf/sdevice.d
for i in $*
do
if [ -f $i ]
then
ed $i << END > /dev/null 2>&1
1,\$s/	Y	/	N	/
w
w
q
END
fi
done
}

turnon () {
cd $ROOT/etc/conf/sdevice.d
for i in $*
do
if [ -f $i ]
then
ed $i << END > /dev/null 2>&1
1,\$s/	N	/	Y	/
w
w
q
END
fi
done
}

turn_fs_off () {
cd $ROOT/etc/conf/sfsys.d
for i in $*
do
if [ -f $i ]
then
ed $i << END > /dev/null 2>&1
1,\$s/	Y/	N/
w
w
q
END
fi
done
}

turn_fs_on () {
cd $ROOT/etc/conf/sfsys.d
for i in $*
do
if [ -f $i ]
then
ed $i << END > /dev/null 2>&1
1,\$s/	N/	Y/
w
w
q
END
fi
done
}

if [ $# != 1 ]
then echo Usage: $0 on/off
     exit 2
fi
case "$1" in
on)  cleanup
     turnon sem shm sxt vx xt osm lp ipc msg cpyrt xout rt
     turnon clist gvid kdvm log connld weitek
     turnon nmi nxt pipemod nsxt i286x raio kdb kdb-util clone ptm pts
     turnoff ramd

     turnoff sem shm sxt vx xt osm lp ipc msg cpyrt xout rt
     turnoff clist gvid kdvm log connld weitek
     turnoff nmi nxt pipemod nsxt i286x raio kdb kdb-util clone ptm pts
     cp $ROOT/usr/src/uts/i386/master.d/ramd/sdev $ROOT/etc/conf/sdevice.d/ramd
     cp $ROOT/usr/src/uts/i386/master.d/ramd/space.c $ROOT/etc/conf/pack.d/ramd/space.c
     turnon ramd
     mv $ROOT/etc/conf/cf.d/mdevice $ROOT/etc/conf/cf.d/.jdevice
     cp $ROOT/etc/conf/cf.d/stune $ROOT/etc/conf/cf.d/.jstune
     cd $ROOT/usr/src/proto/i386/at386
     cp cmd/mdevice.mini $ROOT/etc/conf/cf.d/mdevice
     # If kernel has merge turned on, this will put this into mini mdevice
     grep merge $ROOT/etc/conf/cf.d/.jdevice >> $ROOT/etc/conf/cf.d/mdevice
     #cp cmd/stune.mini $ROOT/etc/conf/cf.d/stune
     ;;
off) cleanup
     turnon sem shm sxt vx xt osm lp ipc msg cpyrt xout rt
     turnon clist gvid kdvm log connld weitek
     turnon nmi nxt pipemod nsxt i286x raio kdb kdb-util clone ptm pts
     turnoff ramd
     ;;
*)   echo Usage: $0 on/off
     exit 2
     ;;
esac
exit 0
