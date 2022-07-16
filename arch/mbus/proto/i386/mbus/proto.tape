/dev/null
RAMSIZE RAMINODE
d--777 2 2
		.profile	---555 2 2 CMDS/main.sh
proc	d--777 2 2
$
opt	d--777 2 2
	unix	d--755 2 2
		main.sh		---555 2 2 CMDS/main.sh
		main.menu	---555 2 2 CMDS/main.menu
		install.sh	---555 2 2 CMDS/install.sh
		install.menu	---555 2 2 CMDS/install.menu
		lib.func	---555 2 2 CMDS/lib.func
		format.sh	---555 2 2 CMDS/format.sh
		format.menu	---555 2 2 CMDS/format.menu
		savefile	---444 2 2 CMDS/savefile
		setup		---555 2 2 CMDS/setup
	$
$
bin	d--777 2 2
$
var	d--777 2 2
	adm	d--777 2 2
	$
$
sbin	d--755 2 2
	mdl		---555 2 2 sbin/mdl
	hdformat	---555 2 2 sbin/hdformat 
	cpunum		---555 2 2 sbin/cpunum
	sgib		---555 2 2 sbin/sgib
	umount		---555 2 2 sbin/umount
	memsize		---700 0 3 sbin/memsize
	sh		---555 2 2 sbin/sh
	sync		---555 2 2 CMDS/sync
	uadmin		---555 2 2 CMDS/uadmin
	autopush	---555 2 2 CMDS/autopush
	mknod		---555 2 2 CMDS/mknod
	init		---555 2 2 CMDS/init
	initial		---555 2 2 CMDS/initial
	disksetup	---700 0 3 CMDS/disksetup
$
dev	d--755 2 2
	dsk	d--755 2 2
		0s1	b--600 2 2 0 1
		0s2	b--600 2 2 0 2
		0s3	b--600 2 2 0 3
		0s4	b--600 2 2 0 4
		0s6	b--600 2 2 0 6
		0s10	b--600 2 2 0 10
		0s11	b--600 2 2 0 11
		0s12	b--600 2 2 0 12
		0s13	b--600 2 2 0 13
		0s14	b--600 2 2 0 14
		0s15	b--600 2 2 0 15
		1s1	b--600 2 2 0 17
		1s2	b--600 2 2 0 18
		1s3	b--600 2 2 0 19
		1s4	b--600 2 2 0 20
		1s6	b--600 2 2 0 22
		1s10	b--600 2 2 0 26
		1s11	b--600 2 2 0 27
		1s12	b--600 2 2 0 28
		1s13	b--600 2 2 0 29
		1s14	b--600 2 2 0 30
		1s15	b--600 2 2 0 31
		2s1	b--600 2 2 0 33
		2s2	b--600 2 2 0 34
		2s3	b--600 2 2 0 35
		2s4	b--600 2 2 0 36
		2s6	b--600 2 2 0 38
		2s10	b--600 2 2 0 42
		2s11	b--600 2 2 0 43
		2s12	b--600 2 2 0 44
		2s13	b--600 2 2 0 45
		2s14	b--600 2 2 0 46
		2s15	b--600 2 2 0 47
		3s1	b--600 2 2 0 49
		3s2	b--600 2 2 0 50
		3s3	b--600 2 2 0 51
		3s4	b--600 2 2 0 52
		3s6	b--600 2 2 0 54
		3s10	b--600 2 2 0 59
		3s11	b--600 2 2 0 60
		3s12	b--600 2 2 0 61
		3s13	b--600 2 2 0 62
		3s14	b--600 2 2 0 63
		3s15	b--600 2 2 0 64
		f0t		b--600 2 2 0 76
		f0		b--600 2 2 0 77
		ram0	b--600 2 2 21 0
		ram1	b--600 2 2 21 1
	$
	rdsk	d--755 2 2
		0s0	c--600 2 2 0 0
		1s0	c--600 2 2 0 16
		2s0	c--600 2 2 0 32
		3s0	c--600 2 2 0 48
		f0t		c--600 2 2 0 76
		f0		c--600 2 2 0 77
		ram0	c--600 2 2 21 0
		ram1	c--600 2 2 21 1
	$
	root	b--600 2 2 5 0
	swap	b--600 2 2 5 1
	console	c--622 2 2 1 0
	sysmsg	c--622 2 2 19 0
	mem	c--666 2 2 2 0
	null	c--666 2 2 2 2
	kmem	c--666 2 2 2 1
	pmem	c--666 2 2 2 3
	zero	c--666 2 2 2 4
	ics	c--666 2 2 22 0
	sad	d--722 3 1
		user	c--666 2 2 25 0
		admin	c--666 2 2 25 1
	$
	rmt	d--722 3 1
		c0s0	c--666 0 3 3 124 
		c0s0n	c--666 0 3 3 125 
	$

$
etc	d--777 2 2
	TIMEZONE	---444 2 2 etc/TIMEZONE
	passwd		---444 2 2 CMDS/passwd
	vfstab		---444 2 2 CMDS/vfstab
	boot		---444 2 2 etc/boot
	dsboot		---444 2 2 etc/dsboot
	shadow		---444 2 2 CMDS/shadow
	group		---444 2 2 CMDS/group
	inittab		---444 2 2 CMDS/inittab
	ioctl.syscon	---444 2 2 CMDS/ioctl.syscon
	mnttab		---666 2 2 CMDS/mnttab
	ap	d--777 2 2
		chan.ap		---755 2 2 CMDS/chan.ap
	$
	default	d--777 2 2
		disksetup	---755 2 2 CMDS/disk.dfl
	$
	fs	d--777 2 2
		s5	d--777 2 2
			labelit		---555 2 2 CMDS/labelit.s5
			mkfs		---555 2 2 CMDS/mkfs.s5
			fsck		---555 2 2 CMDS/fsck.s5
			mount		---555 2 2 CMDS/mount.s5
		$
		ufs	d--777 2 2
			labelit		---555 2 2 CMDS/labelit.ufs
			mkfs		---555 2 2 CMDS/mkfs.ufs
			fsck		---555 2 2 CMDS/fsck.ufs
			mount		---555 2 2 CMDS/mount.ufs
		$
		bfs	d--777 2 2
			mkfs		---555 2 2 CMDS/mkfs.bfs
			fsck		---555 2 2 CMDS/fsck.bfs
			mount		---555 2 2 CMDS/mount.bfs
		$
		proc	d--777 2 2
			mount		---555 2 2 CMDS/mount.proc
		$

		
	$
$
usr	d--777 2 2
	sbin	d--777 2 2
		swap		---711 2 2 usr/sbin/swap
		chroot		---555 2 2 CMDS/chroot
	$
	bin	d--777 2 2
		basename	---555 2 2 usr/bin/basename
		rmdir		---555 2 2 usr/bin/rmdir
		sleep		---555 2 2 usr/bin/sleep
		date		---555 2 2 CMDS/date
		find		---555 2 2 CMDS/find
		expr		---555 2 2 CMDS/expr
		uname		---555 2 2 CMDS/uname
		cpio		---555 2 2 CMDS/cpio
		mkdir		---555 2 2 CMDS/mkdir
		ln 		---555 2 2 CMDS/mv
		cat		---555 2 2 CMDS/cat
		rm		---555 2 2 CMDS/rm
		tee		---555 2 2 CMDS/tee
		kill		---555 2 2 CMDS/kill
		stty		---555 2 2 usr/bin/stty
	$
	lib	d--777 2 2
		libc.so.1		---555 2 2 usr/lib/libc.so.1
		tape	d--777 2 2
			tapecntl	---755 2 2 usr/lib/tape/tapecntl
		$
	$
$
mnt	d--777 2 2
$
tmp	d--777 2 2
$
floppy	d--777 2 2
$
$
