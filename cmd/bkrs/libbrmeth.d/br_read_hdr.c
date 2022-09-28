/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:libbrmeth.d/br_read_hdr.c	1.9.2.1"

#include <sys/types.h>
#include <backup.h>
#include <fcntl.h>
#include <brarc.h>
#include <bktypes.h>
#include <sys/errno.h>
#include <errno.h>
#include "libadmIO.h"

extern int	g_close();
extern GFILE	*g_open();
extern int	g_read();
extern int	g_seek();

extern char *sys_errlist[];


static char *hd = NULL;
static long hd_size = 0;

br_read_hdr(f, ai, partsize, dev)
GFILE **f;
register struct archive_info *ai;
long partsize;			/* if dpart, size in bytes , o/w 0 */
char *dev;
{
	register struct br_arc *a;
	register char *wrk;
	int ret, readsize = 0, i, savi;
	int	buf_sz;

#ifdef TRACE
	brlog(" br_read_hdr: dev=%s", dev);
#endif

	if(hd == NULL) {
		hd = (char *) malloc((unsigned) 512);
		if(hd == NULL) {
			brlog("no memory for archive hdr");
			return(-1);
		}
		else {
			hd_size = 512;
		}
	}
#ifdef TRACE
	brlog(" br_read_hdr: (*f)->_file=%d", (*f)->_file);
#endif

	a = (struct br_arc *) hd;

	if(partsize) {
		if (g_seek((*f), (partsize - 512), 0) < 0) {
			brlog("g_seek failed for archive hdr");
			return(-1);
		}
	}
	buf_sz = (*f)->_size;

	if (g_flush((*f), 512) < 0) {
		brlog("g_flush failed");
		return(-1);
	}
	for(i=0; i<10; i++) {
		ret = g_read((*f), hd, 512);
#ifdef TRACE
		if(ret != 512)
			brlog("br_read g_read returns %d f=%d %s",
				ret,(*f),sys_errlist[errno]);
#endif
		if(a->br_magic == BR_MAGIC)
			break;
#ifdef TRACE
		else {
			brlog(" br_read_hdr: a->br_magic != BR_MAGIC i=%d", i);
		}
#endif
	}
	if((ret != 512) || (i >= 10)) {
		if (g_seek((*f), 0l, 0) < 0) {
			brlog("g_seek failed for archive hdr");
		}
		brlog("br_read_hdr no BR_MAGIC");
		return(-1);
	}
	savi = i;

#ifdef TRACE
	brlog(" br_read_hdr: ret=%d, i=%d, a->br_length=%d, hd_size=%d",
		ret, i, a->br_length, hd_size);
#endif

	if(a->br_length > hd_size) {
		hd_size = a->br_length;
		hd = (char *) realloc(hd, (unsigned)(a->br_length));
		if(hd == NULL) {
			brlog("not enough memory for archive hdr");
			hd_size = 0;

			if(partsize)
				(void) g_seek((*f), 0l, 0);
			else
				(void) g_seek((*f), (hd_size - 512), 1);
			return(-1);
		}
		else {
			a = (struct br_arc *) hd;
		}
	}

#ifdef TRACE
	brlog(" br_read_hdr: partsize=%d", partsize);
#endif
	if(partsize) {
		readsize = a->br_length - 512;
		wrk = hd + 512;
	}
	else {
		readsize = a->br_length;
		wrk = hd;
		g_close(*f);
		(*f) = g_open(dev, O_RDONLY, 0);
		if((*f) == NULL) {
			brlog("reopen of %s failed %s",dev,sys_errlist[errno]);
			return(-1);
		}
		buf_sz = (*f)->_size;

		if (g_flush((*f), 512) < 0) {
			brlog("g_flush failed");
			return(-1);
		}
		for(i=0; i<savi; i++) {
			ret = g_read((*f), hd, 512);
#ifdef TRACE
		brlog(" br_read_hdr: ret=%d, i=%d", ret, i);
#endif
		}
	}
#ifdef TRACE
	brlog(" br_read_hdr: readsize=%d", readsize);
#endif

	if(readsize > 0) {
		errno = 0;
		if(partsize) {
			if (g_seek((*f), -(a->br_length), 1) < 0) {
				brlog("g_seek failed for archive hdr");
				return(-1);
			}
		}
		ret = g_read((*f), wrk, readsize);
#ifdef TRACE
		brlog(" br_read_hdr: ret=%d", ret);
#endif
		if(ret != readsize) {
			if(ret <= 0) {
				brlog("read of extended hdr failed %s",
					sys_errlist[errno]);

				if(partsize)
					(void) g_seek((*f), 0l, 0);
				else
					(void) g_seek((*f), (hd_size - 512), 1);
				return(-1);
			}
			else {
				brlog("read hdr incomplete");

				if(partsize)
					(void) g_seek((*f), 0l, 0);
				else
					(void) g_seek((*f), (hd_size - (512 + ret)), 1);
				return(-1);
			}
		}
	}
	wrk = (char *) (&(a->br_data));

	ai->br_date = a->br_date;	/* date-time of backup */
	ai->br_seqno = a->br_seqno;	/* sequence num of this vol */
	ai->br_blk_est = a->br_blk_est;	/* num of blks this archive */
	ai->br_flags = a->br_flags;
	ai->br_media_cap = a->br_media_cap;
	ai->br_length = a->br_length;

	ai->br_sysname = (wrk + a->br_sysname_off);	/* system originating */
	ai->br_method = (wrk + a->br_method_off);	/* method name */
	ai->br_fsname = (wrk + a->br_fsname_off);	/* orginating fs */
	ai->br_dev = (wrk + a->br_dev_off);		/* originating device */
	ai->br_fstype = (wrk + a->br_fstype_off);	/* fstype string */
	ai->br_mname = (wrk + a->br_mname_off);		/* media name */

	if(partsize) {
		if (g_seek((*f), 0l, 0) < 0) {
			brlog("g_seek failed for archive hdr");
			return(-1);
		}
	}
	if (g_flush((*f), buf_sz) < 0) {
		brlog("g_flush failed");
		return(-1);
	}
#ifdef TRACE
	brlog(" br_read_hdr: returning=%d", a->br_length);
#endif
	return(a->br_length);
}
