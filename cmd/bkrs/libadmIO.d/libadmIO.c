/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:libadmIO.d/libadmIO.c	1.7.4.1"

#include	<sys/types.h>
#include	<stdio.h>
#include	<fcntl.h>
#include	<sys/errno.h>
#include	<signal.h>
#include	<string.h>
#include	"libadmIO.h"
#include	<sys/stat.h>
#include	<sys/statvfs.h>
#include	<sys/mkdev.h>
#include	<sys/stropts.h>
#include	<malloc.h>
#include	<memory.h>

#define	FALSE	0
#define	TRUE	(!FALSE)

extern int	close();
extern int	read();
extern int	write();
extern int	lseek();

extern int	errno;

static int	local_init();
static int	local_open();
static int	local_close();
static int	slow_open();
static int	safe_read();
static int	safe_write();
static int	safe_g_call();

/*
 * g_set_up: Determine the device being accessed, and call the
 * appropriate routine provided for that device if it exists.
 * If it does not exist, simply return.
 * The device specific routine should do things which are
 * to be done once per media and are optional, such as
 * retensioning a cartridge tape.
 * Return values: 0 on success, -1 otherwise.
 */

int
g_set_up(name, flags)
char	*name;
int	flags;
{
	char	command[32];
	char	arg1[4];
	char	arg2[4];
	char	arg3[32];
	char	arg4[8];
	char	*argv[6];
	GFILE	f;
	int	ret = 0;

#ifdef DEBUG
	brlog("libadmIO: g_set_up(%s, %d)", name, flags);
#endif

	if (!local_init(&f, name, flags, 0)) {
		errno = EFAULT;
		return(-1);
	}
	/*
	 * Call /usr/lib/libadmIO/%s for the device.
	 * The devices setup routine will be passed the following
	 * values in argv:
	 * char	*routine;	- program name to call
	 * int	command		- DS_SET_UP
	 * int	write_pipe;	- write pipe file descriptor
	 * char	*name;		- device name
	 * int	flags;		- flags
	 */
	if (f._dname[0]) {
		sprintf(command, "/usr/lib/libadmIO/%s", f._dname);
		argv[0] = command;

		if (pipe(f._pipe) != 0) {
			errno = EFAULT;
			return(-1);
		}
		sprintf(arg1, "%d", DS_SET_UP);
		argv[1] = arg1;
		sprintf(arg2, "%d", f._pipe[DSP_PIPE]);
		argv[2] = arg2;
		sprintf(arg3, "%s", name);
		argv[3] = arg3;
		sprintf(arg4, "%d", flags);
		argv[4] = arg4;
		argv[5] = NULL;

		if (safe_g_call(f._pipe[GIO_PIPE], argv, &ret) < 0) {
			errno = EFAULT;
			ret = -1;
		}
		else if (ret) {
			errno = ret;
			ret = -1;
		}
		if (close(f._pipe[GIO_PIPE]) != 0) {
#ifdef DEBUG
	brlog("libadmIO: close GIO_PIPE failed: errno=%d", errno);
#endif
			ret = -1;
		}
		if (close(f._pipe[DSP_PIPE]) != 0) {
#ifdef DEBUG
	brlog("libadmIO: close DSP_PIPE failed: errno=%d", errno);
#endif
			ret = -1;
		}
	}
	else {
		if (close(f._file) != 0) {
#ifdef DEBUG
	brlog("libadmIO: close f._file failed: errno=%d", errno);
#endif
			ret = -1;
		}
	}
	return(ret);
} /* g_set_up() */

/*
 * g_init: Determine the devices being accessed, and call the
 * appropriate routine provided for those devices if they use
 * the same libadmIO library.
 * If they do not, then a call is made to their respective
 * open routines if they exist.
 * The routines called must live under "/usr/lib/libadmIO/'devname'",
 * where "devname" is taken from the configuration table
 * The parameters for the program are the routine name
 * and the two GFILE pointers.
 * g_init returns a pointer to an array of two GFILE structures,
 * where the first element is the "from" device and the second
 * is the "to" device.
 * On error it returns NULL.
 */

GFILE *
g_init(from_name, from_flags, from_mode, to_name, to_flags, to_mode)
char	*from_name;
int	from_flags;
int	from_mode;
char	*to_name;
int	to_flags;
int	to_mode;
{
	char	command[32];
	char	arg1[8];	/* write pipe */
	char	arg2[4];	/* routine to call */
	char	arg3[32];	/* from device name */
	char	arg4[8];	/* from flags */
	char	arg5[32];	/* to device name */
	char	arg6[8];	/* to flags */
	char	*argv[10];
	GFILE	*from_f;
	GFILE	*to_f;
	struct stat	f_st_buf;
	struct stat	t_st_buf;
	char	from_lib[32];	/* from library name */
	char	to_lib[32];	/* to library name */

#ifdef DEBUG
	brlog("libadmIO: g_init(%s, O_RDONLY, %d, %s, O_WRONLY, %d", from_name, from_mode, to_name, to_mode);
#endif
	if ((from_f = (GFILE *)malloc(2*sizeof(GFILE))) == (GFILE *)NULL) {
		errno = EFAULT;
		return(NULL);
	}
	to_f = from_f + 1;

	if (!local_init(from_f, from_name, from_flags, from_mode)) {
		free(from_f);
		return(NULL);
	}
	if (!local_init(to_f, to_name, to_flags, to_mode)) {
		free(from_f);
		return(NULL);
	}
	/* Check to see if they use the same libadmIO library */
	sprintf(from_lib, "/usr/lib/libadmIO/%s", from_f->_dname);
	sprintf(to_lib, "/usr/lib/libadmIO/%s", to_f->_dname);

	if ((stat(from_lib, &f_st_buf) == 0) &&
		(stat(to_lib, &t_st_buf) == 0) &&
			((f_st_buf.st_ino == t_st_buf.st_ino) &&
				(f_st_buf.st_dev == t_st_buf.st_dev))) {
		(void) strcpy(from_f->_dname, to_f->_dname);
	}
	/*
	 * If the devices are the same type, call
	 * /usr/lib/libadmIO/%s for the device.
	 * The devices init routine will be passed the following
	 * values in argv:
	 * char	*routine;	- program name to call
	 * int	command		- DS_INIT
	 * int	write_pipe;	- write pipe file descriptor
	 * char	*f_name;	- from device name
	 * int	f_flags;	- from flags for open
	 * char	*t_name;	- to device name
	 * int	t_flags;	- to flags for open
	 * 
	 * The device specific routine should pass the
	 * the buffer size and the open file descriptors
	 * back up through the pipe.
	 */

	/***
	**** The next conditional differs from the porting base.
	**** The porting base has this conditional:
	**** 	if (strcmp(from_f->_dname, to_f->_dname) == 0) {
	**** While SCSI requires our conditional
	****/

	if (from_f->_dname[0]) {
		int	r[3];
		
		sprintf(command, "/usr/lib/libadmIO/%s", from_f->_dname);
		argv[0] = command;
		if (pipe(from_f->_pipe) != 0) {
			free(from_f);
			return(NULL);
		}
		to_f->_pipe[GIO_PIPE] = from_f->_pipe[GIO_PIPE];
		to_f->_pipe[DSP_PIPE] = from_f->_pipe[DSP_PIPE];
		sprintf(arg1, "%d", DS_INIT);
		argv[1] = arg1;
		sprintf(arg2, "%d", from_f->_pipe[DSP_PIPE]);
		argv[2] = arg2;
		sprintf(arg3, "%s", from_name);
		argv[3] = arg3;
		sprintf(arg4, "%d", from_flags);
		argv[4] = arg4;
		sprintf(arg5, "%s", to_name);
		argv[5] = arg5;
		sprintf(arg6, "%d", to_flags);
		argv[6] = arg6;
		argv[7] = NULL;

		if (safe_g_call(from_f->_pipe[GIO_PIPE], argv, r) < 0) {
			if (close(from_f->_pipe[GIO_PIPE]) != 0) {
#ifdef DEBUG
	brlog("libadmIO: close GIO_PIPE failed: errno=%d", errno);
#endif
			}
			if (close(from_f->_pipe[DSP_PIPE]) != 0) {
#ifdef DEBUG
	brlog("libadmIO: close DSP_PIPE failed: errno=%d", errno);
#endif
			}
			free(from_f);
			return(NULL);
		}
		from_f->_size = to_f->_size = r[0];

		from_f->_file = r[1];
		to_f->_file = r[2];
	}
	/* Otherwise call the devices' open routine. */
	else {
		if (!local_open(from_f, from_name)) {
			free(from_f);
			return(NULL);
		}
		if (!local_open(to_f, to_name)) {
			local_close(from_f);
			if (from_f->_base) free(from_f->_base);
			free(from_f);
			return(NULL);
		}
		if (from_f->_base) {	/* Don't use buffering */
			free(from_f->_base);
			from_f->_base = NULL;
		}
		if (to_f->_base) {	/* Don't use buffering */
			free(to_f->_base);
			to_f->_base = NULL;
		}
	}
	from_f->_fixed = FIXED;	/* Don't use buffering with */
	to_f->_fixed = FIXED;	/* g_init.                  */
#ifdef DEBUG
	brlog("libadmIO: g_init returns (%d, %d)", from_f->_file, to_f->_file);
#endif
	return(from_f);
} /* g_init() */

/*
 * g_wrap_up: calls the appropriate routine provided for that
 * device if it exists.
 * If it does not exist, it calls close.
 * g_wrap_up returns 0 on success, -1 otherwise with errno set.
 */

int
g_wrap_up(from_f)
GFILE	*from_f;
{
	int	t[1];
	int	ret;
	GFILE	*to_f = from_f + 1;
	int	from_errno = 0;
	int	to_errno = 0;

	if (!from_f) {
		errno = EBADF;
		return(-1);
	}
#ifdef DEBUG
	brlog("libadmIO: g_wrap_up(%d, %d)", from_f->_file, to_f->_file);
#endif
	/*
	 * Call /usr/lib/libadmIO/%s via the pipe.
	 * The devices wrap_up routine will be passed the
	 * following values in argv:
	 * int	command		- DS_WRAP_UP
	 * 
	 * The device specific routine should write two
	 * errno values to the pipe.
	 * 0 for success, and the value of errno on failure.
	 */

	/***
	**** The next conditional differs from the porting base.
	**** The porting base has this conditional:
	**** 	if (strcmp(from_f->_dname, to_f->_dname) == 0) {
	**** While SCSI requires our conditional
	****/

	if (from_f->_dname[0]) {
		int	p[2];

		t[0] = DS_WRAP_UP;

		if (write(from_f->_pipe[GIO_PIPE], t, sizeof(t)) != sizeof(t)) {
			ret = -1;
			errno = EFAULT;
		}
		else if (read(from_f->_pipe[GIO_PIPE], p, sizeof(p)) != sizeof(p)) {
			ret = -1;
			errno = EFAULT;
		}
		else {
			from_errno = p[0];
			to_errno = p[1];

			if (from_errno || to_errno) {
				if (to_errno == 0)
					errno = from_errno;
				else
					errno = to_errno;
				ret = -1;
			}
		}
		if (close(from_f->_pipe[GIO_PIPE]) < 0) {
			ret = -1;
#ifdef DEBUG
	brlog("libadmIO: close GIO_PIPE failed: errno=%d", errno);
#endif
		}
		if (close(from_f->_pipe[DSP_PIPE]) < 0) {
			ret = -1;
#ifdef DEBUG
	brlog("libadmIO: close DSP_PIPE failed: errno=%d", errno);
#endif
		}
		if (close(from_f->_file) < 0) {
			ret = -1;
#ifdef DEBUG
	brlog("libadmIO: close from_f->_file failed: errno=%d", errno);
#endif
		}
		if (close(to_f->_file) < 0) {
			ret = -1;
#ifdef DEBUG
	brlog("libadmIO: close to_f->_file failed: errno=%d", errno);
#endif
		}
	}
	else {
		ret = (ret | local_close(from_f));
		ret = (ret | local_close(to_f));
	}
	if (from_f->_base) free(from_f->_base);
	if (to_f->_base) free(to_f->_base);
	free(from_f);
	return(ret);
} /* g_wrap_up() */

/*
 * g_copy: Read nbytes of data from the "from" file and write nbytes
 * of data to the "to" file.
 * g_copy returns the number of bytes copied successfully,
 * otherwise it returns -1 with errno set.
 */

int
g_copy(from_f, nbytes, rd_start, wt_start)
GFILE	*from_f;
unsigned nbytes;
int	rd_start;
int	wt_start;
{
	int	t[5];
	int	ret;
	GFILE	*to_f = from_f + 1;

	if (!from_f) {
		errno = EBADF;
		return(-1);
	}
#ifdef DEBUG
	brlog("libadmIO: g_copy(%d, %d, %d, %d, %d)", from_f->_file, to_f->_file, nbytes, rd_start, wt_start);
#endif
	if (nbytes == 0) {
		return (0);
	}
	/*
	 * Call /usr/lib/libadmIO/%s via the pipe.
	 * device specific routines will be passed the
	 * following values in argv:
	 * int	command		- DS_COPY
	 * int	from_f;		- child from file descriptor
	 * int	to_f;		- child to file descriptor
	 * int	rd_start;	- starting byte address for from device
	 * int	wt_start;	- starting byte address for to device
	 * int	size;		- optimal block size of the device
	 * int	nbytes;		- job size
	 * 
	 * The device specific routine should return the
	 * actual number of bytes copied, and a negative
	 * value with errno set on failure.
	 */

	/***
	**** The next conditional differs from the porting base.
	**** The porting base has this conditional:
	**** 	if (strcmp(from_f->_dname, to_f->_dname) == 0) {
	**** While SCSI requires our conditional
	****/

	if (from_f->_dname[0]) {
		int	p[2];

		t[0] = DS_COPY;
		t[1] = rd_start;
		t[2] = wt_start;
		t[3] = to_f->_size;
		t[4] = nbytes;

		if (write(from_f->_pipe[GIO_PIPE], t, sizeof(t)) != sizeof(t)) {
			ret = -1;
			errno = EFAULT;
		}
		else if (read(from_f->_pipe[GIO_PIPE], p, sizeof(p)) != sizeof(p)) {
			ret = -1;
			errno = EFAULT;
		}
		else {
			ret = p[0];
			errno = p[1];
		}
	} else {
		char	*ptr;
		int	cnt;
		int	i = nbytes;
		int	leave = 0;

		if (from_f->_size > to_f->_size)
			cnt = from_f->_size;
		else
			cnt = to_f->_size;

		if (cnt == 0) cnt = 512;

		if ((ptr = malloc(cnt)) == NULL) {
			errno = EFAULT;
			return(NULL);
		}
		while ((leave == 0) && (i > 0)) {
			int	ret;

			if (i < cnt)
				cnt = i;

			if ((ret = g_read(from_f, ptr, cnt)) != cnt) {

				if (ret < 0) {
					free(ptr);
					return(ret);
				}
				cnt = ret;
				leave = 1;
			}
			if ((ret = g_write(to_f, ptr, cnt)) != cnt) {

				if (ret < 0) {
					free(ptr);
					return(ret);
				}
				cnt = ret;
				leave = 1;
			}
			i -= cnt;
		}
		free(ptr);
		ret = nbytes - i;
	}
	return(ret);
} /* g_copy() */

/*
 * g_open: Determine the device being accessed, and call the
 * appropriate routine provided for that device if it exists.
 * If it does not exist, set the buffer size and return as if
 * it is a normal file.
 * The parameters for the routine is the GFILE pointer.
 * g_open returns a pointer to a GFILE structure.
 */

GFILE *
g_open(name, flags, mode)
char	*name;
int	flags;
int	mode;
{
	GFILE	*f;

#ifdef DEBUG
	brlog("libadmIO: g_open(%s, 0x%x, 0x%x)", name, flags, mode);
#endif
	if ((f = (GFILE *)malloc(sizeof(GFILE))) == (GFILE *)NULL) {
		errno = EFAULT;
		return(NULL);
	}
	if (!local_init(f, name, flags, mode)) {
		free(f);
		return(NULL);
	}
	if (!local_open(f, name)) {
		free(f);
		return(NULL);
	}
#ifdef DEBUG
	brlog("libadmIO: g_open f->_file=%d, f->_size=%d", f->_file, f->_size);
#endif
	return(f);
} /* g_open() */

/*
 * g_close: calls local_close to do the work, the the GFILE
 * structure is freed.
 * g_close returns 0 on success, -1 otherwise with errno set.
 */

int
g_close(f)
GFILE	*f;
{
	int	ret = 0;

#ifdef DEBUG
	brlog("libadmIO: g_close(%d)", f->_file);
#endif
	ret = local_close(f);

	if (f->_base) {
		free(f->_base);
	}
	free(f);

	return(ret);
} /* g_close() */

/*
 * g_read: call safe_read if the buffer is empty, otherwise
 * simply transfer the requested number of bytes.
 * g_read returns the number of bytes "read" (not from the
 * device, but transfered into the users buffer area)
 * otherwise it returns -1 with errno set.
 */

int
g_read(f, buf, size)
GFILE	*f;
char	*buf;
int	size;
{
	int	count = 0;
#ifdef DEBUG
	int	sz = size;
#endif

	if ((!f) || (f->_flag&O_WRONLY)) {
		errno = EBADF;
		return(-1);
	}
	if (size < 0) {
		errno = EINVAL;
		return(-1);
	}
	if (size == 0) {
		return (0);
	}
	/*
	 * If buffering is turned off or the buffer is empty
	 * and the size is a * multiple of the buffer size, just call
	 * safe_read directly - to avoid the memcopy calls.
	 */
	if ((f->_size <= 0) || (f->_base == NULL) ||
			((f->_cnt == 0) && ((size % f->_size) == 0))) {
		return (safe_read(f, buf, size));
	}
#ifdef DEBUG
	brlog("libadmIO: g_read(%d, 0x%x, %d) cnt=%d", f->_file, buf, sz, f->_cnt);
#endif
	while (size > 0) {
		int	n;

		if (f->_cnt == 0) { /* empty buffer */
			f->_cnt = safe_read(f, f->_base, f->_size);

			if (f->_cnt <= 0) {
				f->_cnt = 0;
				break;
			}
			f->_ptr = f->_base;
		}
		n = (size < f->_cnt) ? size : f->_cnt;
		(void) memcpy(buf, f->_ptr, n);
		buf += n;
		count += n;
		f->_cnt -= n;
		f->_ptr += n;
		size -= n;
	}
#ifdef DEBUG
if (count != sz) {
	brlog("libadmIO: g_read returning %d, errno=%d", count, errno);
}
#endif
	return (count);
} /* g_read() */

/*
 * g_write: call safe_write if the buffer is full, otherwise
 * simply copy the data into the write buffer.
 * g_write returns the number of bytes "written" (not to the
 * device, but transfered from the users buffer area)
 * otherwise it returns -1 with errno set.
 */

int
g_write(f, buf, size)
GFILE	*f;
char	*buf;
int	size;
{
	int	count = 0;
#ifdef DEBUG
	int	sz = size;
#endif

	if ((!f) || (!(f->_flag&O_WRONLY))) {
		errno = EBADF;
		return(-1);
	}
	if (size < 0) {
		errno = EINVAL;
		return(-1);
	}
	if (size == 0) {
		return (0);
	}
	/*
	 * If buffering is turned off or the buffer is empty
	 * and the size is a multiple of the buffer size, just call
	 * safe_write directly - to avoid the memcopy calls.
	 */
	if ((f->_size <= 0) || (f->_base == NULL) ||
			((f->_cnt == 0) && ((size % f->_size) == 0))) {
		return (safe_write(f, buf, size));
	}
#ifdef DEBUG
	brlog("libadmIO: g_write(%d, 0x%x, %d) cnt=%d", f->_file, buf, sz, f->_cnt);
#endif
	while (size > 0) {
		int	n;

		if (f->_cnt == f->_size) { /* full buffer */
			int	ret;

			ret = safe_write(f, f->_base, f->_cnt);

			f->_ptr = f->_base;
			f->_cnt = 0;

			if (ret != f->_size) {
				break;
			}
		}
		n = ((f->_size - f->_cnt) < size) ? (f->_size - f->_cnt) : size;
		(void) memcpy(f->_ptr, buf, n);
		f->_ptr += n;
		f->_cnt += n;
		buf += n;
		size -= n;
		count += n;
	}
#ifdef DEBUG
if (count != sz) {
	brlog("libadmIO: g_write returning %d, errno=%d", count, errno);
}
#endif
	return (count);
} /* g_write() */

/*
 * g_flush: flushes out the buffer if the device was open'd
 * for writing. It resets the counter and the pointer if the
 * device was open'd for reading.
 * After doing the actual flush, it adjusts the buffer size
 * to be used as follows:
 *    If bufsize < 0, then it turns off buffering.
 *    If bufsize == 0, then it does not change the buffer size.
 *    If bufsize > 0, then it changes the buffer size to the new value.
 * It returns the number of bytes written to the device,
 * otherwise it returns -1 with errno set.
 */
int
g_flush(f, bufsize)
GFILE	*f;
int	bufsize;
{
	int	ret;

	if (!f) {
		errno = EBADF;
		return(-1);
	}
#ifdef DEBUG
	brlog("libadmIO: g_flush(%d, %d), cnt=%d", f->_file, bufsize, f->_cnt);
#endif
	if (f->_cnt == 0) { /* No data */
		return(0);
	}
	if ((f->_size <= 1) || (f->_base == NULL)) { /* No buffering */
		ret = 0;
	}
	errno = 0;

	if (!(f->_flag&O_WRONLY)) { /* O_RDONLY */
		ret = f->_cnt;
	}
	else {
		if (f->_cnt != (f->_ptr - f->_base)) { /* O_WRONLY check */
#ifdef DEBUG
	brlog("libadmIO: g_flush: cnt=%d != ptr-base=%d", f->_cnt, f->_ptr-f->_base);
#endif
			errno = EFAULT;
			return(-1);
		}
		if (f->_fixed) {	/* zero fill before write */
			char	*p;

			for (p = f->_ptr; p < (f->_base + f->_size); p++) {
				*p = 0;
			}
			f->_cnt = f->_size;
		}
		ret = safe_write(f, f->_base, f->_cnt);
	}
	if ((bufsize != 0) && !f->_fixed) {
		if (bufsize < 0) {	/* Turn off buffering */
			bufsize = 0;

			if (f->_base) {
				free(f->_base);
				f->_base = NULL;
			}
		}
		if (bufsize > f->_size) {
			if (f->_base) {
				f->_base = realloc(f->_base, bufsize);
			}
			else {
				if ((f->_base = malloc(bufsize)) == NULL) {
#ifdef DEBUG
	brlog("libadmIO: g_flush malloc failed");
#endif
					errno = EFAULT;
					return(-1);
				}
			}
		}
		f->_size = bufsize;
	}
	f->_ptr = f->_base;
	f->_cnt = 0;

	return (ret);
} /* g_flush() */

/*
 * g_seek checks to see if the requested flush amount is
 * within the local buffer area, and if so, it simply adjusts
 * its local pointers. If not, it does a g_flush and then
 * seeks to the specified location
 */
int
g_seek(f, offset, whence)
GFILE	*f;
int	offset;
int	whence;
{
	if (!f) {
		errno = EBADF;
		return(-1);
	}
#ifdef DEBUG
	brlog("libadmIO: g_seek(%d, %d, %d)", f->_file, offset, whence);
	brlog("libadmIO: currently at %d", lseek(f->_file, 0, 1));
#endif
	if ((f->_size <= 1) || (f->_base == NULL)) { /* No buffering */
		return(lseek(f->_file, offset, whence));
	}
	if (f->_flag&O_WRONLY) {
		if ((whence == 1) && (offset < 0) && (-offset <= f->_cnt)) {
			f->_ptr += offset;
			f->_cnt += offset;
			return(lseek(f->_file, 0, 1)+offset);
		}
		if (g_flush(f, 0) < 0) {
			return(-1);
		}
	} else { /* O_RDONLY */
		int	left_over;

		if ((whence == 1) &&
			(((offset > 0) && (offset <= f->_cnt)) ||
			((offset < 0) && (-offset <= (f->_ptr - f->_base))))) {
			f->_ptr += offset;
			f->_cnt -= offset;
			return(lseek(f->_file, 0, 1)+offset);
		}
		if ((left_over = g_flush(f, 0)) < 0) {
			return(-1);
		}
		if (whence == 1) {
			offset -= left_over;
		}
	}
	return(lseek(f->_file, offset, whence));
} /* g_seek() */

/*
 * local_init scans the configuration table for the device matching
 * the major number associated with the given device. It sets the
 * dname field to the name from the table if found. If no name is
 * found, it sets up the device as a "normal" file.
 */

static int
local_init(f, name, flags, mode)
GFILE	*f;
char	*name;
int	flags;
int	mode;
{
	int	i;
	int	maj;
	int	count;
	int	size;
	struct stat	st_buf;
	struct s3bconf	*buffer;
	struct s3bc	*table;

	if ((flags&O_RDWR) || (name == NULL)) {
		errno = EINVAL;
		return(FALSE);
	}
	if (stat(name, &st_buf) == -1) {
		st_buf.st_mode = 0;
	}
	if ((f->_file = slow_open(name, flags, mode, st_buf.st_mode)) < 0) {
		return(FALSE);
	}
	f->_dname[0] = NULL;
	f->_size = 0;
	f->_cnt = 0;
	f->_ptr = f->_base = NULL;
	f->_flag = flags;
	f->_fixed = ~FIXED;

#ifdef u3b2
	if (((st_buf.st_mode & S_IFMT) == S_IFBLK) ||
		((st_buf.st_mode & S_IFMT) == S_IFCHR)) { /* Device file */
		maj = major(st_buf.st_rdev);

		if (sys3b(S3BCONF, (struct s3bconf *)&count, sizeof(count)) == -1) {
			errno = EFAULT;
			return(FALSE);
		}
		size = sizeof(int) + (count * sizeof(struct s3bconf));
		buffer = (struct s3bconf *)malloc((unsigned)size);

		if (sys3b(S3BCONF, buffer, size) == -1) {
			if (close (f->_file) != 0) {
#ifdef DEBUG
	brlog("libadmIO: close f->_file failed: errno=%d", errno);
#endif
			}
			errno = EFAULT;
			free(buffer);
			return(FALSE);
		}
		table = (struct s3bc *)((char *)buffer + sizeof(int));
		/*
		 * The following is 3b2 specific code for associating
		 * a major number with a device name and library.
		 */
		for (i = 0; i < count; i++) {
			if (maj == (int)table->board) {
				char	dev_lib[32];

				sprintf(dev_lib, "/usr/lib/libadmIO/%s", table->name);

				if (stat(dev_lib, &st_buf) == 0) {
					if (close (f->_file) != 0) {
#ifdef DEBUG
	brlog("libadmIO: close f->_file failed: errno=%d", errno);
#endif
					}
					strcpy(f->_dname, table->name);
					free(buffer);
					return(TRUE);
				}
			}
			table++;
		}
		free(buffer);
	}
#else
	if (((st_buf.st_mode & S_IFMT) == S_IFBLK) ||
		((st_buf.st_mode & S_IFMT) == S_IFCHR)) {
		char *dname;
		char	dev_lib[32];

		dname=(char*)devattr(name, "bklib");
		if ( dname ) {
			sprintf(dev_lib, "/usr/lib/libadmIO/%s", dname);
			if (stat(dev_lib, &st_buf) == 0) {
				if (close (f->_file) != 0) {
#ifdef DEBUG
	brlog("libadmIO: close f->_file failed: errno=%d", errno);
#endif
				}
				strcpy(f->_dname, dname);
				return(TRUE);
			}
		}
	}
#endif
	return(TRUE);
} /* local_init() */

/*
 * local_open calls /usr/lib/libadmIO/%s for the device.
 * The devices open routine will be passed the
 * following values in argv:
 * char	*routine;	- routine name to call
 * int	command		- DS_OPEN
 * int	write_pipe;	- write pipe file descriptor
 * char	*name;		- device path name
 * int	flags;		- flags to open with
 * 
 * The device specific routine will pass
 * the buffer size, the file descriptor
 * and the fixed flag to the pipe.
 */
static int
local_open(f, name)
GFILE	*f;
char	*name;
{
	char	command[32];
	char	arg1[4];	/* DS_OPEN */
	char	arg2[8];	/* write pipe */
	char	arg3[32];	/* device name */
	char	arg4[8];	/* flags */
	char	*argv[6];

	if (f->_dname[0]) { /* Call /usr/lib/libadmIO/%s */
		int	r[3];

		sprintf(command, "/usr/lib/libadmIO/%s", f->_dname);
		argv[0] = command;
		if (pipe(f->_pipe) != 0) {
#ifdef DEBUG
	brlog("libadmIO: local_open pipe failed");
#endif
			return(FALSE);
		}
		sprintf(arg1, "%d", DS_OPEN);
		argv[1] = arg1;
		sprintf(arg2, "%d", f->_pipe[DSP_PIPE]);
		argv[2] = arg2;
		sprintf(arg3, "%s", name);
		argv[3] = arg3;
		sprintf(arg4, "%d", f->_flag);
		argv[4] = arg4;
		argv[5] = NULL;

		if (safe_g_call(f->_pipe[GIO_PIPE], argv, r) < 0) {
#ifdef DEBUG
	brlog("libadmIO: local_open safe_g_call failed");
#endif
			return(FALSE);
		}
		f->_size = r[0];
		f->_file = r[1];
		f->_fixed = r[2];

		if ((f->_size > 0) &&
			((f->_base = malloc(f->_size)) == NULL)) {
#ifdef DEBUG
	brlog("libadmIO: local_open malloc failed");
#endif
			local_close(f);
			return(FALSE);
		}
	}
	f->_ptr = f->_base;
	return(TRUE);
} /* local_open() */

/*
 * local_close: calls g_flush to clear out the buffer in case the
 * device was open'd for writing, the calls the appropriate
 * routine provided for that device if it exists.
 * If it does not exist, it calls close.
 * local_close returns 0 on success, -1 otherwise with errno set.
 */
static int
local_close(f)
GFILE	*f;
{
	int	t[1];
	int	ret = 0;

	if (!f) {
		errno = EBADF;
		return(-1);
	}
	errno = 0;

	g_flush(f, 0);
	/*
	 * Call /usr/lib/libadmIO/%s for the device.
	 * device specific routines will be passed the
	 * following values in argv:
	 * int	command		- DS_CLOSE
	 * 
	 * The device specific routine should return
	 * 0 on success and errno on failure.
	 */
	if (f->_dname[0]) {
		int	p;

		t[0] = DS_CLOSE;

		if (write(f->_pipe[GIO_PIPE], t, sizeof(t)) != sizeof(t)) {
#ifdef TRACE
	brlog("libadmIO: local_close: write failed, errno=%d", errno);
#endif
			ret = -1;
			errno = EFAULT;
		}
		else if (read(f->_pipe[GIO_PIPE], &p, sizeof(int)) != sizeof(int)) {
#ifdef TRACE
	brlog("libadmIO: local_close: read failed, errno=%d", errno);
#endif
			ret = -1;
			errno = EFAULT;
		}
		else {
			if (errno = p)
				ret = -1;
		}
		if (close(f->_pipe[GIO_PIPE]) < 0) {
#ifdef DEBUG
	brlog("libadmIO: close GIO_PIPE failed: errno=%d", errno);
#endif
			ret = -1;
		}
		if (close(f->_pipe[DSP_PIPE]) < 0) {
#ifdef DEBUG
	brlog("libadmIO: close DSP_PIPE failed: errno=%d", errno);
#endif
			ret = -1;
		}
	}
	if (close(f->_file) != 0) {
		ret = -1;
#ifdef DEBUG
	brlog("libadmIO: close f->_file failed: errno=%d", errno);
#endif
	}
	return(ret);
} /* local_close() */

static int
slow_open(name, flags, mode, st_mode)
char	*name;
int	flags;
int	mode;
int	st_mode;
{
	register int	k = 1;
	int		fd = -1;

	errno = 0;

	if (((st_mode & S_IFMT) == S_IFBLK) ||
		((st_mode & S_IFMT) == S_IFCHR)) { /* Device file */
		k = 6;
	}
	while ((k > 0) && (fd < 0)) {
		fd = open(name, flags, mode);

#ifdef DEBUG
	if (fd < 0) {
		brlog("libadmIO: slow_open failed: errno=%d", errno);
	}
#endif
		if ((fd < 0) && (--k > 0)) {
			errno = 0;
			sleep(5);
		}
	}
	return(fd);
} /* slow_open()  */

static int
safe_read(f, buf, size)
GFILE	*f;
char	*buf;
int	size;
{
	int	ret;
	int	t[3];

	errno = 0;

	while (((ret = read(f->_file, buf, size)) < 0) && (errno == EINTR))
		continue;

	if (f->_dname[0]) {
		int	p[2];

		t[0] = DS_READ;
		t[1] = ret;
		t[2] = errno;

		if (write(f->_pipe[GIO_PIPE], t, 3*sizeof(int)) != 3*sizeof(int)) {
			errno = EFAULT;
			return(-1);
		}
		if (read(f->_pipe[GIO_PIPE], p, 2*sizeof(int)) != 2*sizeof(int)) {
			errno = EFAULT;
			return(-1);
		}
		ret = p[0];
		errno = p[1];
	}
	return(ret);
} /* safe_read */

static int
safe_write(f, buf, size)
GFILE	*f;
char	*buf;
int	size;
{
	int	ret;
	int	t[3];

	errno = 0;

	while (((ret = write(f->_file, buf, size)) < 0) && (errno == EINTR))
		continue;

	if (f->_dname[0]) {
		int	p[2];

		t[0] = DS_WRITE;
		t[1] = ret;
		t[2] = errno;

		if (write(f->_pipe[GIO_PIPE], t, 3*sizeof(int)) != 3*sizeof(int)) {
			errno = EFAULT;
			return(-1);
		}
		if (read(f->_pipe[GIO_PIPE], p, 2*sizeof(int)) != 2*sizeof(int)) {
			errno = EFAULT;
			return(-1);
		}
		ret = p[0];
		errno = p[1];
	}
	return(ret);
} /* safe_write */

/*
 * safe_g_call - executes the specified command and reads
 * data bytes from it into the specified return area (ret).
 * Called by g_setup, g_init and g_open only.
 */
static int
safe_g_call(rp, argv, ret)
int	rp;
char	*argv[];
int	*ret;
{
	int			pid;
	int			i;
	struct strrecvfd	f;

#ifdef TRACE
	brlog("libadmIO: safe_g_call calling %s", argv[0]);
#endif
	switch (pid = fork()) {
		case -1:		/* fork error */
			return(-1);
			break;
		case 0:			/* child process */
			if (execv(argv[0], argv) < 0) {
				return(-1);
			}
			return(-1);
			break;
		default:	/* parent process */
			if ((i = read(rp, ret, sizeof(int))) != sizeof(int)) {
				return(-1);
			}
			break;
	}
	if (atoi(argv[1]) == DS_SET_UP) {
		return(0);
	}
	/* DS_INIT or DS_OPEN */

	if (ret[0] < 0) {
		errno = -ret[0];
		return(-1);
	}
	if (ioctl(rp, I_RECVFD, &f) < 0) {
#ifdef DEBUG
		switch (errno) {
		case EBADMSG:
			brlog("libadmIO: safe_g_call: EBADMSG returned");
		case EAGAIN:
			brlog("libadmIO: safe_g_call: EAGAIN returned");
		default:
			brlog("libadmIO: safe_g_call: %d returned", errno);
		}
#endif
		return(-1);
	}
	ret[1] = f.fd;	/* from file descriptor */

	if (atoi(argv[1]) == DS_OPEN) {
		if ((i = read(rp, &ret[2], sizeof(int))) != sizeof(int)) {
			return(-1);
		}
	}
	if (atoi(argv[1]) == DS_INIT) {
		if (ioctl(rp, I_RECVFD, &f) < 0) {
#ifdef DEBUG
		switch (errno) {
		case EBADMSG:
			brlog("libadmIO: safe_g_call: EBADMSG returned");
		case EAGAIN:
			brlog("libadmIO: safe_g_call: EAGAIN returned");
		default:
			brlog("libadmIO: safe_g_call: %d returned", errno);
		}
#endif
		return(-1);
		}
		ret[2] = f.fd;	/* to file descriptor */
	}
	return(0);
} /* safe_g_call() */
