#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)initpkg:./inittab.sh	1.18.21.2"

if u3b2
then echo "ap::sysinit:/sbin/autopush -f /etc/iu.ap
fs::sysinit:/sbin/bcheckrc </dev/console >/dev/console 2>&1
ck::sysinit:/sbin/setclk </dev/console >/dev/console 2>&1
xdc::sysinit:/sbin/sh -c 'if [ -x /etc/rc.d/0xdc ] ; then /etc/rc.d/0xdc ; fi' >/dev/console 2>&1
ac::sysinit:/sbin/ckmunix </dev/console >/dev/console 2>&1
pt:123:bootwait:/sbin/sh -c '> /etc/rc2.d/.ports.sem' </dev/console >/dev/console 2>&1
is:2:initdefault:
p1:s1234:powerfail:/sbin/led -f			# start green LED flashing
p3:s1234:powerfail:/sbin/shutdown -y -i0 -g0 >/dev/console 2>&1
fl:056:wait:/sbin/led -f 			# start green LED flashing
s0:0:wait:/sbin/rc0 off >/dev/console 2>&1 </dev/console
s0:5:wait:/sbin/rc0 firmware >/dev/console 2>&1 </dev/console
s1:1:wait:/sbin/rc1 >/dev/console 2>&1 </dev/console
s2:23:wait:/sbin/rc2 >/dev/console 2>&1 </dev/console
s3:3:wait:/sbin/rc3 >/dev/console 2>&1 </dev/console
s6:6:wait:/sbin/rc6 reboot >/dev/console 2>&1 < /dev/console
sc:234:respawn:/usr/lib/saf/sac -t 300
co:1234:respawn:/usr/lib/saf/ttymon -g -p \"Console Login: \" -m ldterm -d /dev/console -l console
ct:234:respawn:/usr/lib/saf/ttymon -g -m ldterm -d /dev/contty -l contty
he:234:respawn:/usr/sbin/hdelogger" \
>inittab

elif i386
then echo "ap::sysinit:/sbin/autopush -f /etc/ap/chan.ap
ak::sysinit:/sbin/wsinit 1>/etc/wsinit.err 2>&1
ck::sysinit:/sbin/setclk </dev/console >/dev/sysmsg 2>&1
bchk::sysinit:/sbin/bcheckrc </dev/console >/dev/sysmsg 2>&1
is:2:initdefault:
r0:0:wait:/sbin/rc0 off 1> /dev/sysmsg 2>&1 </dev/console
r1:1:wait:/sbin/rc1  1> /dev/sysmsg 2>&1 </dev/console
r2:23:wait:/sbin/rc2 1> /dev/sysmsg 2>&1 </dev/console
r3:3:wait:/sbin/rc3  1> /dev/sysmsg 2>&1 </dev/console
r5:5:wait:/sbin/rc0 reboot 1> /dev/sysmsg 2>&1 </dev/console
r6:6:wait:/sbin/rc6 reboot 1> /dev/sysmsg 2>&1 </dev/console
sd:0:wait:/sbin/uadmin 2 0 >/dev/sysmsg 2>&1 </dev/console
fw:5:wait:/sbin/uadmin 2 2 >/dev/sysmsg 2>&1 </dev/console
rb:6:wait:/sbin/uadmin 2 1 >/dev/sysmsg 2>&1 </dev/console
li:23:wait:/usr/bin/ln /dev/systty /dev/syscon >/dev/null 2>&1
sc:234:respawn:/usr/lib/saf/sac -t 300
co:12345:respawn:/sbin/vtgetty console console
" >inittab

else
echo "sys0::sysinit:/etc/setmrf d
sys1::sysinit:/etc/don -s tn83 0
sys2::sysinit:/etc/don -s all
is:s:initdefault:
bchk::bootwait:/sbin/bcheckrc < /dev/console > /dev/console 2>&1
brc::bootwait:/sbin/brc 1> /dev/console 2>&1
link::wait:(rm -f /dev/syscon;ln /dev/systty /dev/syscon) 1> /dev/console 2>&1
r0:0:wait:/sbin/rc0 1> /dev/console 0>&1
r2:2:wait:/sbin/rc2 1> /dev/console 2>&1
r3:3:wait:/sbin/rc3 1> /dev/console 2>&1
st::off:/etc/stload > /dev/console 2>&1
pf::powerfail:/etc/powerfail 1> /dev/console 2>&1
co::respawn:/etc/getty console console
" >inittab
fi
