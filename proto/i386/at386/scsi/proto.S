/dev/null
2370 150
d--775 0 3
INSTALLA ---555 2 2 scsi/INSTALLA
INSTALL2 ---555 2 2 scsi/INSTALL2
INSTALL4 ---555 2 2 scsi/INSTALL4
readfloppy ---755 2 2 readfloppy
usr	d--775 0 3
	lib	d--775 0 2
		libc.so.1	---555 2 2 libc.so.1
		$
	bin	d--775 2 2
		chmod   ---555 2 2 chmod
		chgrp	---555 2 2 chgrp
		chown	---555 2 2 chown
		find    ---555 2 2 find
		cpio    ---555 2 2 cpio
		mkdir   ---555 2 2 mkdir
		ed	---555 2 2 ed
		dd	---555 2 2 dd
		sleep	---555 2 2 sleep
		expr	---555 2 2 expr
		mv	---555 2 2 mv
		rm	---555 2 2 rm
		$
	sbin	d--755 0 3
		machine_type ---544 0 2 machine_type
		$
	$
etc	d--755 0 3
	boot    ---444 2 2 boot
	default	d--755 2 2
		default.at386	---444 0 3 default.at386
		default.att	---444 0 3 default.att
		default.att512	---444 0 3 default.att512
		default.cpq	---444 0 3 default.cpq
		$
	initial	---555 2 2 initial
	initprog d--755 2 2
		att	---444 0 3 att
		compaq	---444 0 3 compaq
		$
	fs	d--755 0 3
		s5	d--755 0 3
			fsck	---555 0 3 fsck.s5
			labelit ---555 0 3 labelit.s5
			mkfs	---555 0 3 mkfs.s5
			$
		bfs	d--755 0 3
			fsck	---555 0 3 fsck.bfs
			mkfs	---555 0 3 mkfs.bfs
			mount	---555 0 3 mount.bfs
			$
		ufs	d--755 0 3
			fsck	---555 0 3 fsck.ufs
			labelit	---555 0 3 labelit.ufs
			mkfs	---555 0 3 mkfs.ufs
			mount	---555 0 3 mount.ufs
			$
		$
	inittab2 ---644 0 3 inittab2
	disk.ele ---644 0 3 disk.ele
	disk.elesm ---644 0 3 disk.elesm
	scsi	d--755 2 2
			scsiformat	---555 2 2 scsiformat
		format.d d--755 2 2
			sd00.0	---555 2 2 sd00.0
			sd01.1	---555 2 2 sd01.1
			DISK	---555 2 2 DISK
			$
		tc.index	---555 2 2 tc.index
		$
	$
sbin	d--777 2 2
	fdisk	---755 2 2 fdisk
	flop_disk ---555 2 2 flop_disk
	disksetup ---555 2 2 disksetup
	tapecntl ---555 2 2 stapecntl
	mknod	---555 2 2 mknod
	mount	---555 2 2 mount
	partsize ---555 2 2 partsize
	memsize ---544 0 2 memsize
	sync	---555 2 2 sync
	$
$
