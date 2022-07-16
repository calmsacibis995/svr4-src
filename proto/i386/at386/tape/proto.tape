/dev/null
2370 208
d--775 0 3
unix ---744 0 1 tape/unix
INSTALL ---555 2 2 tape/INSTALL
.profile ---555 2 2 profile
LABEL ---644 2 2 LABEL
usr	d--775 0 3
	lib	d--775 0 2
		$
	bin	d--775 2 2
		stty    ---555 2 2 stty
		$
	$
dev	d--775 0 3
	console	c--622 0 3 5 0
	cram	c--666 0 3 18 0
	dsk	d--775 0 0
		0s0	b--600 0 3 0 0
		0s1	b--600 0 3 0 1
		0s2	b--600 0 3 0 2
		0s3	b--600 0 3 0 3
		0s4     b--600 0 3 0 4
		0s5	b--600 0 3 0 5
		0s6     b--600 0 3 0 6
		0s10	b--600 0 3 0 10
		0s11	b--600 0 3 0 11
		0s12	b--600 0 3 0 12
		0s13	b--600 0 3 0 13
		1s0	b--600 0 3 0 16
		1s1	b--600 0 3 0 17
		1s2	b--600 0 3 0 18
		1s3	b--600 0 3 0 19
		1s4     b--600 0 3 0 20
		1s5	b--600 0 3 0 21
		1s6     b--600 0 3 0 22
		1s10	b--600 0 3 0 26
		1s11	b--600 0 3 0 27
		1s12	b--600 0 3 0 28
		1s13	b--600 0 3 0 29
		f0q15d  b--666 0 3 1 4
		f0q15dt b--666 0 3 1 0
		f0	b--666 0 3 1 132
		f0t	b--666 0 3 1 128
		f1	b--666 0 3 1 133
		f1t	b--666 0 3 1 129
		$
	kd	d--775 0 0
		kd00	c--400 3 3 30 0
		kd01	c--400 3 3 30 1
		kd02	c--400 3 3 30 2
		kd03	c--400 3 3 30 3
		kd04	c--400 3 3 30 4
		kd05	c--400 3 3 30 5
		kd06	c--400 3 3 30 6
		kd07	c--400 3 3 30 7
		kd08	c--400 3 3 30 8
		kd09	c--400 3 3 30 9
		kd10	c--400 3 3 30 10
		kd11	c--400 3 3 30 11
		kd12	c--400 3 3 30 12
		kd13	c--400 3 3 30 13
		kd14	c--400 3 3 30 14
		kdvm00	c--400 3 3 20 0
		kdvm01	c--400 3 3 20 1
		kdvm02	c--400 3 3 20 2
		kdvm03	c--400 3 3 20 3
		kdvm04	c--400 3 3 20 4
		kdvm05	c--400 3 3 20 5
		kdvm06	c--400 3 3 20 6
		kdvm07	c--400 3 3 20 7
		kdvm08	c--400 3 3 20 8
		kdvm09	c--400 3 3 20 9
		kdvm10	c--400 3 3 20 10
		kdvm11	c--400 3 3 20 11
		kdvm12	c--400 3 3 20 12
		kdvm13	c--400 3 3 20 13
		kdvm14	c--400 3 3 20 14
		$
	mem	c--440 3 3 2 0
	pmem	c--640 0 3 2 3
	osm		c--644 0 3 17 0
	null	c--666 0 3 2 2
	rdsk	d--775 0 0
		0s0	c--600 0 3 0 0
		0s1	c--600 0 3 0 1
		0s2	c--600 0 3 0 2
		0s3	c--600 0 3 0 3
		0s4     c--600 0 3 0 4
		0s5	c--600 0 3 0 5
		0s6     c--600 0 3 0 6
		0s10	c--600 0 3 0 10
		0s11	c--600 0 3 0 11
		0s12	c--600 0 3 0 12
		0s13	c--600 0 3 0 13
		1s0	c--600 0 3 0 16
		1s1	c--600 0 3 0 17
		1s2	c--600 0 3 0 18
		1s3	c--600 0 3 0 19
		1s4     c--600 0 3 0 20
		1s5	c--600 0 3 0 21
		1s6     c--600 0 3 0 22
		1s10	c--600 0 3 0 26
		1s11	c--600 0 3 0 27
		1s12	c--600 0 3 0 28
		1s13	c--600 0 3 0 29
		f0q15d  c--666 0 3 1 4
		f0q15dt c--666 0 3 1 0
		f0	c--666 0 3 1 132
		f0t	c--666 0 3 1 128
		f1	c--666 0 3 1 133
		f1t	c--666 0 3 1 129
		$
	rmt	d--755 0 0
		c0s0n	c--666 0 3 4 0
		c0s0	c--666 0 3 4 4
		c0s0nr	c--666 0 3 4 8
		c0s0r	c--666 0 3 4 12
		$
	sad	d--755 0 3
		user	c--666 0 3 25 0
		admin	c--666 0 3 25 1
		$
	syscon	c--622 2 2 5 0
	sysmsg	c--222 2 2 19 0
	systty	c--622 2 2 5 0
	swap	b--640 0 3 0 2
	term	d--775 0 3
		$
	tty	c--666 2 2 16 0
	tty00	c--666 2 2 3 0
	tty01	c--666 2 2 3 1
	vidadm	c--666 2 2 29 1
	video	c--666 2 2 29 0
	vt01	c--666 0 3 5 1
	vt02	c--666 0 3 5 2
	vt03	c--666 0 3 5 3
	vt04	c--666 0 3 5 4
	vt05	c--666 0 3 5 5
	vt06	c--666 0 3 5 6
	vt07	c--666 0 3 5 7
	vt08	c--666 0 3 5 8
	vtmon	c--666 3 3 30 15
	zero	c--666 0 3 2 4
	$
etc	d--755 0 3
	ap	d--755 2 2
		chan.ap	---444 0 3 chan.ap
		$
	default	d--755 2 2
		workstations	---444 0 3 work.1
		$
	initprog d--755 2 2
		at386	---444 0 3 at386
		$
	fs	d--755 0 3
		s5	d--755 0 3
			mount ---555 0 3 mount.s5
			$
		$
	group   ---644 0 3 group
	inittab ---644 0 3 inittab
	ioctl.syscon ---444 2 2 ioctl.syscon
	passwd  ---444 0 3 passwd
	TIMEZONE ---444 0 3 TIMEZONE
	emulator ---744 2 2 emulator
	$
sbin	d--777 2 2
	autopush	---555 0 3 autopush
	chkconsole    ---755 0 3 chkconsole
	machid ---544 0 2 tape/machid
	sh      ---555 2 2 sh
	uadmin ---755 2 2 uadmin
	umount ---555 2 2 umount
	wsinit	---555 0 2 wsinit
	init    ---555 0 2 init
	$
mnt	d--777 2 2
	$
tmp	d--777 2 2
	$
install	d--777 2 2
	$
var	d--755 0 3
	adm	d--755 0 3
		$
	$
$
