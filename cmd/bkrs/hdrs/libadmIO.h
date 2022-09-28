/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:hdrs/libadmIO.h	1.4.2.1"

#define	DNAME_LEN	4

#define	DS_SET_UP	0	/* Used for device specific set up */

#define	DS_OPEN		1	/* Used for device specific open */
#define	DS_READ		2	/* Used for device specific read */
#define	DS_WRITE	3	/* Used for device specific write */
#define	DS_CLOSE	4	/* Used for device specific close */

#define	DS_INIT		11	/* Used for device specific init */
#define	DS_COPY		12	/* Used for device specific copy */
#define	DS_WRAP_UP	13	/* Used for device specific wrap_up */

	/*
	 * The following defines should be used to exchange
	 * data with the libadmIO routines.
	 */
#define	GIO_PIPE	0	/* Used by the GIO library */
#define	DSP_PIPE	1	/* Used by the device specific programs */

#define READ(f,r,n)	if (read(f, r, n) != n) local_exit(1)
#define WRITE(f,r,n)	if (write(f, r, n) != n) local_exit(1)

#define	FIXED		1

typedef struct {
	char		_dname[DNAME_LEN]; /* device name from g_init */
	int		_size;		/* total size of the buffer */
	int		_fixed;		/* tells if buffer size is fixed */
	int		_cnt;		/* number of avail chars in buffer */
	char		*_ptr;		/* next char from/to here in buffer */
	char		*_base;		/* buffer pointer */
	int		_flag;		/* the flags from g_open */
	int		_file;		/* file descriptor */
	int		_pipe[2];	/* pipe file descriptors*/
} GFILE;
