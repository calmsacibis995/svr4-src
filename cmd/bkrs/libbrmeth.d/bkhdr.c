/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:libbrmeth.d/bkhdr.c	1.8.2.1"

#include <limits.h> 	/* get PATH_MAX from here, not stdio */
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/fcntl.h>
#include <signal.h>
#include <backup.h>
#include <bktypes.h>
#include <brarc.h>
#include <string.h>
#include <errno.h>
#include "libadmIO.h"

#define IS_DPART 0

extern int	brlog();
extern void	*malloc();
extern void	*realloc();
extern int	g_close();
extern GFILE	*g_open();
extern int	g_read();
extern int	g_write();
extern int	g_seek();
extern int	g_flush();
extern int	g_set_up();
extern GFILE	*g_init();
extern int	g_wrap_up();

extern int	bklevels;
extern char	*sys_errlist[];
extern int	sys_nerr;

static char	*unk_sys_err = "errno out of range";
#define SE	(((errno>0)&&(errno<sys_nerr))?sys_errlist[errno]:unk_sys_err)

static char buf [1024+512];

struct br_arc *
bld_hdr (ai, size)
register struct bld_archive_info *ai;
int *size;		/* return size of br hdr */
{
	int hdrsize, mn_len = 0;
	int meth_len=0, fs_len=0, dev_len=0, fstype_len=0;
	static int sysname_len;
	static struct utsname un;
	static short first = 1;
	short offset = 0;
	static int cur_size = 0;
	static struct br_arc *hdr = NULL;

	if (first) {
		first = 0;
		(void) uname (&un);
		sysname_len = strlen(un.sysname);
	}

	hdrsize = sizeof(struct br_arc);
	if (ai->br_method) {
		meth_len = strlen(ai->br_method) + 1;
		hdrsize += meth_len;
	}
	if (ai->br_fsname) {
		fs_len = strlen(ai->br_fsname) + 1;
		hdrsize += fs_len;
	}
	if (ai->br_dev) {
		dev_len = strlen(ai->br_dev) + 1;
		hdrsize += dev_len;
	}
	if (ai->br_fstype) {
		fstype_len = strlen(ai->br_fstype) + 1;
		hdrsize += dev_len;
	}
	hdrsize += (sysname_len + 1);
	if (ai->br_mname) {
		mn_len = strlen(ai->br_mname) + 1;
		hdrsize += mn_len;
	}
	hdrsize += 511;
	hdrsize &= ~511;

#ifdef TRACE
	brlog("bld_hdr sysname=%s hdrsize=%d",un.sysname,hdrsize);
#endif

	if ( hdr == NULL ) {
		hdr = (struct br_arc *) malloc(hdrsize);
		if (hdr == NULL) {
			brlog("no memory for archive hdr");
			return(NULL);
		}
		else {
			cur_size = hdrsize;
		}
	}
	else {
		if (hdrsize > cur_size) {
			hdr = (struct br_arc *) realloc(hdr, hdrsize);
			if (hdr == NULL) {
				brlog("no memory for archive hdr");
				return(NULL);
			}
			else {
				cur_size = hdrsize;
			}
		}
	}

	*size = hdrsize;

	hdr->br_magic = BR_MAGIC;
	hdr->br_seqno = ai->br_seqno;
	hdr->br_length = (long) hdrsize;
	hdr->br_date = ai->br_date;
	hdr->br_blk_est = ai->br_blk_est;
	hdr->br_media_cap = ai->br_media_cap;
	hdr->br_flags = ai->br_flags;

	hdr->br_sysname_off = offset;
	strcpy(hdr->br_data, un.sysname);
	offset += (sysname_len + 1);

	if (meth_len) {
		hdr->br_method_off = offset;
		strcpy(((hdr->br_data) + offset), ai->br_method);
		offset += meth_len;
	}
	else {
		hdr->br_method_off = (offset-1);
	}

	if (fs_len) {
		hdr->br_fsname_off = offset;
		strcpy(((hdr->br_data) + offset), ai->br_fsname);
		offset += fs_len;
	}
	else {
		hdr->br_fsname_off = (offset-1);
	}

	if (dev_len) {
		hdr->br_dev_off = offset;
		strcpy(((hdr->br_data) + offset), ai->br_dev);
		offset += dev_len;
	}
	else {
		hdr->br_dev_off = (offset-1);
	}

	if (fstype_len) {
		hdr->br_fstype_off = offset;
		strcpy(((hdr->br_data) + offset), ai->br_fstype);
		offset += fstype_len;
	}
	else {
		hdr->br_fstype_off = (offset-1);
	}

	if (mn_len) {
		hdr->br_mname_off = offset;
		strcpy(((hdr->br_data) + offset), ai->br_mname);
		offset += mn_len;
	}
	else {
		hdr->br_mname_off = (offset-1);
	}

	hdr->br_blk_est = ai->br_blk_est;
	hdr->br_media_cap = ai->br_media_cap;

#ifdef TRACE
	brlog("build_hdr returns 0x%x hdrsize=%d br_length=%d",
				hdr,hdrsize,hdr->br_length);
	brlog("estimate=%d cap=%d",hdr->br_blk_est,hdr->br_media_cap);
	brlog("br_sysname_off = %d sysname=%s",hdr->br_sysname_off,(hdr->br_data)+(hdr->br_sysname_off));
	brlog("br_method_off = %d method=%s",hdr->br_method_off,(hdr->br_data)+(hdr->br_method_off));
	brlog("br_fsname_off = %d fsname=%s",hdr->br_fsname_off,(hdr->br_data)+(hdr->br_fsname_off));
	brlog("br_dev_off = %d dev=%s",hdr->br_dev_off,(hdr->br_data)+(hdr->br_dev_off));
	brlog("br_fstype_off = %d fstype=%s",hdr->br_fstype_off,(hdr->br_data)+(hdr->br_fstype_off));
	brlog("br_mname_off = %d mname=%s seq %d",hdr->br_mname_off,(hdr->br_data)+(hdr->br_mname_off), hdr->br_seqno);
#endif

	return(hdr);

}



br_write_hdr(f, dtype, volpromt, wa, capacity, arcname)
GFILE	**f;			/* archive file dsec */
char	*dtype;			/* devmgmt dtype */
char	*volpromt;		/* devmgmt volpromt */
struct wr_archive_hdr *wa;
long capacity;			/* bytes per vol */
char *arcname;			/* string of the thing we opened for archive */
{
	int type = -1, isfile = 0;
	int extra, size = 0;
	int bytesin = 0, byteswanted = 1024;
	char *bp;
	int	buf_sz = 0;
#define RDSZ (byteswanted - bytesin)

	wa->br_labelit_hdr = (char *) NULL;
	wa->br_lab_len = 0;
	size = wa->br_hdr_len;

#ifdef TRACE
brlog("enter br_write_hdr *f=0x%x",*f);
#endif
	if (!strcmp(dtype, "dpart"))
		type = IS_DPART;
	if (!strcmp(dtype, "file"))
		isfile = 1;
	if (!strncmp(dtype, "dir", 3))
		isfile = 1;

	if (!isfile) {
		switch (type) {

		case IS_DPART:
#ifdef TRACE
brlog("IS_DPART: wa->br_hdr_len=%d",wa->br_hdr_len);
brlog("Opening device O_WRONLY");
#endif
			BEGIN_CRITICAL_REGION;

			(*f) = g_open (arcname, O_WRONLY, 0);

			END_CRITICAL_REGION;

			if ((*f) == NULL) {
				brlog("Open of %s failed %s",arcname,SE);
				return(-1);
			}
			buf_sz = (*f)->_size;

			if (g_flush((*f), 512) < 0) {
				brlog("g_flush failed %s", SE);
				(void)g_close(*f);
				return(-1);
			}
			extra = wa->br_hdr_len - 512;

			if (g_seek((*f), (capacity - 512), 0) < 0) {
				brlog("g_seek failed %s", SE);
				(void)g_close(*f);
				return(-1);
			}
			if (g_write((*f), wa->br_hdr, 512) != 512) {
				brlog("write of bkrs hdr size %d failed %s",
						512,SE);
				(void)g_close(*f);
				return(-1);
			}
			if (extra) {

				if (g_seek((*f), -(wa->br_hdr_len), 1) < 0) {
					brlog("g_seek failed %s", SE);
					(void)g_close(*f);
					return(-1);
				}
				if (g_write((*f), (((int)(wa->br_hdr)) + 512), extra) 
						!= extra) {
					brlog("write bkrs hdr size %d failed %s",
						wa->br_hdr_len, SE );
					(void)g_close(*f);
					return(-1);
				}
			}
			size = 0;

			if (g_seek((*f), 0, 0) < 0) {
				brlog("g_seek failed %s", SE);
				(void)g_close(*f);
				return(-1);
			}
			break;

		default:
#ifdef TRACE
brlog("DEFAULT: bytesin=%d, byteswanted=%d",bytesin, byteswanted);
#endif
			if (volpromt == NULL) {
#ifdef TRACE
brlog("Opening device O_WRONLY");
#endif
				BEGIN_CRITICAL_REGION;

				(*f) = g_open (arcname, O_WRONLY, 0);

				END_CRITICAL_REGION;

				if ((*f) == NULL) {
					brlog("reopen of %s failed %s",arcname,SE);
					return(-1);
				}
				buf_sz = (*f)->_size;

				if (g_flush((*f), 512) < 0) {
					brlog("g_flush failed %s", SE);
					(void)g_close(*f);
					return(-1);
				}
				break;
			}
#ifdef TRACE
brlog("Opening device O_RDONLY");
#endif
			BEGIN_CRITICAL_REGION;

			(*f) = g_open (arcname, O_RDONLY, 0);

			END_CRITICAL_REGION;

			if ((*f) == NULL) {
				brlog("Open of %s failed %s",arcname,SE);
				return(-1);
			}
			buf_sz = (*f)->_size;

			if (g_flush((*f), 512) < 0) {
				brlog("g_flush failed %s", SE);
				(void)g_close(*f);
				return(-1);
			}
			wa->br_labelit_hdr = buf;

			while(bytesin < byteswanted) {
				isfile = -1;
				isfile = g_read((*f), (buf+bytesin), RDSZ);
#ifdef TRACE
brlog("bkhdr read returns %d (*f)->_file=%d",isfile,(*f)->_file);
#endif
				if (isfile < 0) {
					sleep(5);
				}
				if (isfile > 0) {
					if (bytesin == 0) {
						if (!strncmp(buf, "Volcopy", 7)){
#ifdef TRACE
brlog("have Volcopy block");
#endif
							byteswanted += 512;
						}
					}
					bytesin += isfile;
					continue;
				}
				brlog("cannot read %s label ret=%d %s", volpromt, isfile, SE);
				(void)g_close(*f);
				return(-1);
			}
			if (bytesin >= byteswanted) {
				wa->br_lab_len = byteswanted;
			}
/*
 * qtape cannot be written after a read, the following close/open should
 * work for all devices.
*/
			(void) g_close (*f);
#ifdef TRACE
brlog("Reopenning device O_WRONLY");
#endif
			BEGIN_CRITICAL_REGION;

			(*f) = g_open (arcname, O_WRONLY, 0);

			END_CRITICAL_REGION;

			if ((*f) == NULL) {
				brlog("reopen of %s failed %s",arcname,SE);
				return(-1);
			}
			buf_sz = (*f)->_size;

			if (g_flush((*f), 512) < 0) {
				brlog("g_flush failed %s", SE);
				(void)g_close(*f);
				return(-1);
			}
			bp = buf;
			if (wa->br_lab_len > 1024) {
				if (g_write((*f), bp, 512) != 512) {
					brlog("rewrite Volcopy hdr failed %s", SE);
					(void)g_close(*f);
					return(-1);
				}
				bp += 512;
			}
			if (g_write((*f), bp, 1024) != 1024) {
				brlog("rewrite of fs/volname failed %s",SE);
				(void)g_close(*f);
				return(-1);
			}
		}
	}
#ifdef TRACE
brlog("size=%d: wa->br_hdr_len=%d",size,wa->br_hdr_len);
#endif
	if (size) {
		int	ret;

		if ((ret = g_write((*f), wa->br_hdr, wa->br_hdr_len)) != wa->br_hdr_len){
			brlog("write of bkrs hdr size %d failed, ret=%d, %s", wa->br_hdr_len,ret,SE);
			(void)g_close(*f);
			return(-1);
		}
	}
	if (g_flush((*f), buf_sz) < 0) {
		brlog("g_flush failed %s", SE);
		(void)g_close(*f);
		return(-1);
	}
	return(0);
}

br_copy_hdr(f, t, dtype, volpromt, wa, capacity, fname, arcname)
GFILE	**f;			/* backup file desc */
GFILE	**t;			/* archive file desc */
char	*dtype;			/* devmgmt dtype */
char	*volpromt;		/* devmgmt volpromt */
struct wr_archive_hdr *wa;
long capacity;			/* bytes per vol */
char *fname;			/* string of the thing we opened for backup */
char *arcname;			/* string of the thing we opened for archive */
{
	int type = -1, isfile = 0;
	int extra, size = 0;
	int bytesin = 0, byteswanted = 1024;
	char *bp;
	int	buf_sz;
#define RDSZ (byteswanted - bytesin)

	wa->br_labelit_hdr = (char *) NULL;
	wa->br_lab_len = 0;
	size = wa->br_hdr_len;

#ifdef TRACE
brlog("enter br_copy_hdr *f=0x%x, *t=0x%x, dtype=%s", (*f), (*t), dtype);
brlog("volpromt=%s, capacity=%ld, fname=%s, arcname=%s", volpromt, capacity, fname, arcname);
#endif
	if (!strcmp(dtype, "dpart"))
		type = IS_DPART;
	if (!strcmp(dtype, "file"))
		isfile = 1;
	if (!strncmp(dtype, "dir", 3))
		isfile = 1;

#ifdef TRACE
brlog("br_copy_hdr: calling g_set_up");
#endif
	if (g_set_up (fname, O_RDONLY) < 0) {
		brlog("Set up of %s failed %s",fname,SE);
		return(-1);
	}
	if (g_set_up (arcname, O_RDONLY) < 0) {
		brlog("Set up of %s failed %s",arcname,SE);
		return(-1);
	}
	if (isfile) {
		BEGIN_CRITICAL_REGION;
		(*f) = g_init (fname, O_RDONLY, 0, arcname, (O_WRONLY|O_CREAT|O_TRUNC), 0644);
		END_CRITICAL_REGION;

		if ((*f) == NULL) {
			brlog("Init of %s failed %s",arcname,SE);
			return(-1);
		}
		(*t) = (*f) + 1;
		buf_sz = (*t)->_size;

		if (g_flush(*t, 512) < 0) {
			brlog("g_flush failed %s", SE);
			(void)g_wrap_up(*f);
			return(-1);
		}
	}
	else {
		switch (type) {

		case IS_DPART:
#ifdef TRACE
brlog("IS_DPART: wa->br_hdr_len=%d",wa->br_hdr_len);
brlog("Opening device O_WRONLY");
#endif
			BEGIN_CRITICAL_REGION;
			(*f) = g_init (fname, O_RDONLY, 0, arcname, O_WRONLY, 0);
			END_CRITICAL_REGION;

			if ((*f) == NULL) {
				brlog("Init of %s failed %s",arcname,SE);
				return(-1);
			}
			(*t) = (*f) + 1;
			buf_sz = (*t)->_size;

			if (g_flush((*t), 512) < 0) {
				brlog("g_flush failed %s", SE);
				(void)g_wrap_up(*f);
				return(-1);
			}
			extra = wa->br_hdr_len - 512;

			if (g_seek((*t), (capacity - 512), 0) < 0) {
				brlog("g_seek failed %s", SE);
				(void)g_wrap_up(*f);
				return(-1);
			}
			if (g_write((*t), wa->br_hdr, 512) != 512) {
				brlog("write of bkrs hdr size %d failed %s", 512,SE);
				(void)g_wrap_up(*f);
				return(-1);
			}
			if (extra) {

				if (g_seek((*t), -(wa->br_hdr_len), 1) < 0) {
					brlog("g_seek failed %s", SE);
					(void)g_wrap_up(*f);
					return(-1);
				}
				if (g_write((*t), (((int)(wa->br_hdr)) + 512), extra) 
						!= extra) {
					brlog("write bkrs hdr size %d failed %s", wa->br_hdr_len,SE);
					(void)g_wrap_up(*f);
					return(-1);
				}
			}
			size = 0;
			if (g_seek((*t), 0, 0) < 0) {
				brlog("g_seek failed %s", SE);
				(void)g_wrap_up(*f);
				return(-1);
			}
			break;

		default:
#ifdef TRACE
brlog("DEFAULT: bytesin=%d, byteswanted=%d",bytesin, byteswanted);
#endif
			if (volpromt == NULL) {
#ifdef TRACE
brlog("volprompt == NULL: Reopenning device O_WRONLY");
#endif
				BEGIN_CRITICAL_REGION;
				(*f) = g_init (fname, O_RDONLY, 0, arcname, O_WRONLY, 0);
				END_CRITICAL_REGION;

				if ((*f) == NULL) {
					brlog("Init of %s failed %s",arcname,SE);
					return(-1);
				}
				(*t) = (*f) + 1;
				buf_sz = (*t)->_size;

				if (g_flush((*t), 512) < 0) {
					brlog("g_flush failed %s", SE);
					(void)g_wrap_up(*f);
					return(-1);
				}
				break;
			}
#ifdef TRACE
brlog("volpromt = %s: Opening device O_RDONLY", volpromt);
#endif
			BEGIN_CRITICAL_REGION;

			(*f) = g_open (arcname, O_RDONLY, 0);

			END_CRITICAL_REGION;

			if ((*f) == NULL) {
				brlog("Open of %s failed %s",arcname,SE);
				return(-1);
			}
			buf_sz = (*f)->_size;

			if (g_flush((*f), 512) < 0) {
				brlog("g_flush failed %s", SE);
				(void)g_close(*f);
				return(-1);
			}
			wa->br_labelit_hdr = buf;

			while(bytesin < byteswanted) {
				isfile = -1;
				isfile = g_read((*f), (buf+bytesin), RDSZ);
#ifdef TRACE
brlog("bkhdr read returns %d (*f)->_file=%d",isfile,(*f)->_file);
#endif
				if (isfile < 0) {
					sleep(5);
				}
				if (isfile > 0) {
					if (bytesin == 0) {
						if (!strncmp(buf, "Volcopy", 7)){
#ifdef TRACE
brlog("have Volcopy block");
#endif
							byteswanted += 512;
						}
					}
					bytesin += isfile;
					continue;
				}
				brlog("cannot read %s label ret=%d %s", volpromt, isfile, SE);
				(void)g_close(*f);
				return(-1);
			}
			if (bytesin >= byteswanted) {
				wa->br_lab_len = byteswanted;
			}
/*
 * qtape cannot be written after a read, the following close/open should
 * work for all devices.
*/
			(void) g_close (*f);
#ifdef TRACE
brlog("Reopenning device O_WRONLY");
#endif
			BEGIN_CRITICAL_REGION;
			(*f) = g_init (fname, O_RDONLY, 0, arcname, O_WRONLY, 0);
			END_CRITICAL_REGION;

			if ((*f) == NULL) {
				brlog("Init of %s failed %s",arcname,SE);
				return(-1);
			}
			(*t) = (*f) + 1;
			buf_sz = (*t)->_size;

			if (g_flush((*t), 512) < 0) {
				brlog("g_flush failed %s", SE);
				return(-1);
			}
			bp = buf;
#ifdef TRACE
brlog("wa->br_lab_len=%d", wa->br_lab_len);
#endif
			if (wa->br_lab_len > 1024) {
				if (g_write((*t), bp, 512) != 512) {
					brlog("rewrite Volcopy hdr failed %s", SE);
					(void)g_wrap_up(*f);
					return(-1);
				}
				bp += 512;
			}
			if (g_write((*t), bp, 1024) != 1024) {
				brlog("rewrite of fs/volname failed %s",SE);
				(void)g_wrap_up(*f);
				return(-1);
			}
		}
	}
#ifdef TRACE
brlog("size=%d: wa->br_hdr_len=%d",size,wa->br_hdr_len);
#endif
	if (size) {
		int	ret;

		if ((ret = g_write((*t), wa->br_hdr, wa->br_hdr_len)) != wa->br_hdr_len){
			brlog("write of bkrs hdr size %d failed, ret=%d, %s", wa->br_hdr_len,ret,SE);
			(void)g_wrap_up(*f);
			return(-1);
		}
	}
	if (g_flush((*t), buf_sz) < 0) {
		brlog("g_flush failed %s", SE);
		(void)g_wrap_up(*f);
		return(-1);
	}
	return(0);
}
