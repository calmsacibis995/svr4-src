/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef _SYS_CONF_H
#define _SYS_CONF_H

#ident	"@(#)head.sys:sys/conf.h	11.21.3.1"

/*
 * Declaration of block device switch. Each entry (row) is
 * the only link between the main unix code and the driver.
 * The initialization of the device switches is in the file conf.c.
 */
struct bdevsw {
	int	(*d_open)();
	int	(*d_close)();
	int	(*d_strategy)();
	int	(*d_print)();
	int	(*d_size)();
	int	(*d_xpoll)();
	int	(*d_xhalt)();
	char	*d_name;
	struct iobuf	*d_tab;
	int	*d_flag;
};

extern struct bdevsw bdevsw[];
extern struct bdevsw shadowbsw[];

/*
 * Character device switch.
 */
struct cdevsw {
	int	(*d_open)();
	int	(*d_close)();
	int	(*d_read)();
	int	(*d_write)();
	int	(*d_ioctl)();
	int	(*d_mmap)();
	int	(*d_segmap)();
	int	(*d_poll)();
	int	(*d_xpoll)();
	int	(*d_xhalt)();
	struct tty *d_ttys;
	struct streamtab *d_str;
	char	*d_name;
	int	*d_flag;
};

extern struct cdevsw cdevsw[];
extern struct cdevsw shadowcsw[];


/*
 * And the console co routine.  This is declared as
 * a configuration parameter so that it can be changed
 * to match /dev/console.
 */
struct  conssw {
    int (*co)();
    int co_dev;
    int (*ci)();
};

extern struct conssw conssw;



/*
 * Device flags.
 *
 * Bit 0 to bit 15 are reserved for kernel.
 * Bit 16 to bit 31 are reserved for different machines.
 */
#define D_NEW		0x00	/* new-style driver */
#define	D_OLD		0x01	/* old-style driver */
#define D_DMA		0x02    /* driver does DMA  */
/*
 * Added for UFS.
 */
#define D_SEEKNEG       0x04    /* Negative seek offsets are OK */
#define D_TAPE          0x08    /* Magtape device (no bdwrite when cooked) */
/*
 * Added for pre-4.0 drivers backward compatibility.
 */
#define D_NOBRKUP	0x10	/* No breakup needed for new drivers */

#define ROOTFS_NAMESZ	7	/* Maximum length of root fstype name */

#define	FMNAMESZ	8

struct fmodsw {
	char	f_name[FMNAMESZ+1];
	struct streamtab *f_str;
	int	*f_flag;		/* same as device flag */
};
extern struct fmodsw fmodsw[];

extern int	bdevcnt;
extern int	cdevcnt;
extern int	fmodcnt;

/*
 * Line discipline switch.
 */
struct linesw {
	int	(*l_open)();
	int	(*l_close)();
	int	(*l_read)();
	int	(*l_write)();
	int	(*l_ioctl)();
	int	(*l_input)();
	int	(*l_output)();
	int	(*l_mdmint)();
};
extern struct linesw linesw[];

extern int	linecnt;
/*
 * Terminal switch
 */
struct termsw {
	int	(*t_input)();
	int	(*t_output)();
	int	(*t_ioctl)();
};
extern struct termsw termsw[];

extern int	termcnt;

#endif	/* _SYS_CONF_H */
