/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-fs:rfs/rf_canon.c	1.3"

#include "sys/types.h"
#include "sys/stream.h"
#include "sys/param.h"
#include "sys/rf_messg.h"
#include "sys/nserve.h"
#include "sys/rf_cirmgr.h"
#include "sys/cmn_err.h"
#include "sys/dirent.h"
#include "rf_canon.h"
#include "sys/sysmacros.h"
#include "sys/vnode.h"
#include "sys/errno.h"

/* imports */
extern int	stoi();
extern int	strlen();

/*
 * Convert from canonical to local representation.  Return length of the
 * local representation, or 0 for bad format or data.
 */
int
rf_fcanon(fmt, from, end, to)
	register char		*fmt;		/* conversion format */
	register caddr_t	from;		/* canonical data */
	register caddr_t	end;		/* of canonical data */
	caddr_t			to;		/* local data */
{
	long			ltmp;		/* intermediate scalar values */
	register caddr_t	cptr;		/* cursor into ltmp */
	caddr_t			tptr = to;	/* cursor into local data */
	char			*lfmt;		/* addressable copy of fmt */
	register int		is_string;	/* flag "c0" format */

	while (*fmt) {
		if (from >= end) {
			goto fail;
		}
		switch (*fmt++) {
		case 's':
			tptr = SALIGN(tptr);
			from = LALIGN(from);
			if (from >= end) {
				goto fail;
			}
			cptr = (caddr_t	)&ltmp;
			*cptr++ = hibyte(hiword(*from));
			*cptr++ = lobyte(hiword(*from));
			*cptr++ = hibyte(loword(*from));
			*cptr = lobyte(loword(*from));
			*(short *)tptr = ltmp;
			tptr = SNEXT(tptr);
			from = LNEXT(from);
			continue;
		case 'i':
			tptr = IALIGN(tptr);
			from = LALIGN(from);
			if (from >= end) {
				goto fail;
			}
			cptr = (caddr_t	) & ltmp;
			*cptr++ = hibyte(hiword(*from));
			*cptr++ = lobyte(hiword(*from));
			*cptr++ = hibyte(loword(*from));
			*cptr = lobyte(loword(*from));
			*(int *)tptr = ltmp;
			tptr = INEXT(tptr);
			from = LNEXT(from);
			continue;
		case 'l':
			tptr = LALIGN(tptr);
			from = LALIGN(from);
			if (from >= end) {
				goto fail;
			}
			ltmp = *(long *)from;
			*tptr++ = hibyte(hiword(ltmp));
			*tptr++ = lobyte(hiword(ltmp));
			*tptr++ = hibyte(loword(ltmp));
			*tptr++ = lobyte(loword(ltmp));
			from = LNEXT(from);
			continue;
		case 'b':
			*tptr++ = *from++;
			continue;
		case 'c':
			from = LALIGN(from);
			if (from >= end) {
				goto fail;
			}

			/*
			 * We use the format-encoded length, and ignore
			 * that in the data, for character arrays.
			 */

			lfmt = fmt;
			ltmp = stoi(&lfmt);
			fmt = lfmt;
			if (!ltmp) {
				ltmp = *(long *)from;
				cptr = (caddr_t	)&ltmp;
				*cptr++ = hibyte(hiword(*from));
				*cptr++ = lobyte(hiword(*from));
				*cptr++ = hibyte(loword(*from));
				*cptr = lobyte(loword(*from));
				is_string = 1;
			} else {
				is_string = 0;
			}

			/*
			 * Character array and string sanity checks:
			 *
			 * 1 - encoded end of array/string must not exceed
			 *     end of data and
			 * 2 - must not exceed MAXNAMELEN
			 *
			 * Checks for strings only:
			 *
			 * 3 - must be null terminated
			 * 4 - length must equal that specified in data.
	 		 */

			from = LNEXT(from);
			if (from + ltmp > end || ltmp > MAXNAMELEN ||
			  is_string &&
			   (*(from + ltmp - 1) != '\0' ||
			    ltmp != strlen(from) + 1)) {
				goto fail;
			}
			while (ltmp--) {
				if (from >= end) {
					goto fail;
				}
				*tptr++ = *from++;
			}
			continue;
		default:
			goto fail;

		}
	}
	return tptr - to;
fail:
	cmn_err(CE_WARN, "rf_fcanon:  bad format or data");
	return 0;
}

/*
 * Convert from local to canonical representation.
 * Return length of canonical representation, 0 for format error.
 *
 * from == to iff inplace copy is okay, iff the data to be converted
 * is all longs.  to may, in general, have to be twice the length of
 * the local representation to allow for worst-case expansion.
 */
int
rf_tcanon(fmt, from, to)
	register char		*fmt;		/* conversion format */
	register caddr_t	from;		/* canonical data */
	register caddr_t	to;		/* local data */
{
	long			ltmp;		/* intermediate scalar values */
	caddr_t			tptr;		/* cursor into converted data */
	char			*lfmt;		/* addressable copy of fmt */

	tptr = to;
	while (*fmt) {
		switch (*fmt++) {
		case 's':
			tptr = LALIGN(tptr);
			from = SALIGN(from);
			ltmp = *(short *)from;
			*tptr++ = hibyte(hiword(ltmp));
			*tptr++ = lobyte(hiword(ltmp));
			*tptr++ = hibyte(loword(ltmp));
			*tptr++ = lobyte(loword(ltmp));
			from = SNEXT(from);
			continue;
		case 'i':
			tptr = LALIGN(tptr);
			from = IALIGN(from);
			ltmp = *(int *)from;
			*tptr++ = hibyte(hiword(ltmp));
			*tptr++ = lobyte(hiword(ltmp));
			*tptr++ = hibyte(loword(ltmp));
			*tptr++ = lobyte(loword(ltmp));
			from = INEXT(from);
			continue;
		case 'l':
			tptr = LALIGN(tptr);
			from = LALIGN(from);
			ltmp = *(long *)from;
			*tptr++ = hibyte(hiword(ltmp));
			*tptr++ = lobyte(hiword(ltmp));
			*tptr++ = hibyte(loword(ltmp));
			*tptr++ = lobyte(loword(ltmp));
			from = LNEXT(from);
			continue;

		case 'b':
			*tptr++ = *from++;
			continue;
		case 'c':
			lfmt = fmt;
			ltmp = stoi(&lfmt);
			fmt = lfmt;
			if (!ltmp) {
				ltmp = strlen(from) + 1;
			}
			tptr = LALIGN(tptr);
			*tptr++ = hibyte(hiword(ltmp));
			*tptr++ = lobyte(hiword(ltmp));
			*tptr++ = hibyte(loword(ltmp));
			*tptr++ = lobyte(loword(ltmp));
			while (ltmp--) {
				*tptr++ = *from++;
			}
			tptr = LALIGN(tptr);
			continue;
		default:
			return 0;
		}
	}
	return tptr - to;
}

/*
 * Convert RFS headers to canonical format.
 */
void
rf_hdrtcanon(bp, version)
	register mblk_t	*bp;
	register int version;	/* version of RFS protocol */
{
	register caddr_t	head;
	register long		type;
	register char		*fmt;

	/* request or response header */
	switch (type = RF_COM(bp)->co_type) {
	case RF_REQ_MSG:
		head = (caddr_t)RF_REQ(bp);
		break;
	case RF_RESP_MSG:
	case RF_NACK_MSG:
		head = (caddr_t)RF_RESP(bp);
		break;
	default:
		cmn_err(CE_PANIC, "rf_hdrtcanon: bad type\n");
		/* NOTREACHED */
	}

	/* rf_message_t and rf_common_t header */
	(void)rf_tcanon(MSGCOM_FMT, (caddr_t)bp->b_rptr, (caddr_t)bp->b_rptr);

	switch (version) {
	case RFS1DOT0:
		fmt = COMMV1_FMT;
		break;
	case RFS2DOT0:
		if (type == RF_REQ_MSG) {
			fmt = REQV2_FMT;
		} else {
			fmt = RESPV2_FMT;
		}
		break;
	default:
		cmn_err(CE_PANIC, "rf_hdrtcanon: bad version\n");
		/* NOTREACHED */
	}
	(void)rf_tcanon(fmt, head, head);
}

/*
 * Convert rf_message_t and rf_common_t headers in RFS message to
 * canonical format.  Return length of converted data or zero for
 * failure.
 */
int
rf_mcfcanon(bp)
	mblk_t			*bp;
{
	register caddr_t	rptr = (caddr_t)bp->b_rptr;

	return rf_fcanon(MSGCOM_FMT, rptr, rptr + RF_MCSZ, rptr);
}

/*
 * Convert rf_request_t or rf_response_t headers from canonical format.
 * Assumes rf_message_t and rf_common_t headers in RFS message have
 * been decanonized if necessary.
 *
 * Return bytes converted for success, 0 for bad header or data.
 */
int
rf_rhfcanon(bp, gp)
	register mblk_t		*bp;
	register gdp_t		*gp;
{
	register int		version = gp->version;
	caddr_t			head;
	caddr_t			end;
	register long		type;
	register caddr_t	fmt;

	switch (type = RF_COM(bp)->co_type) {
	case RF_REQ_MSG:
		head = (caddr_t)RF_REQ(bp);
		break;
	case RF_RESP_MSG:
	case RF_NACK_MSG:
		head = (caddr_t)RF_RESP(bp);
		break;
	default:
		return 0;
	}

	end = head;

	switch (version) {
	case RFS1DOT0:
		fmt = COMMV1_FMT;
		end += RFV1_MINRESP;	/* equal in size to RFV1_MINREQ */
		break;
	case RFS2DOT0:
		if (type == RF_REQ_MSG) {
			fmt = REQV2_FMT;
			end += RFV2_MINREQ;
		} else {
			fmt = RESPV2_FMT;
			end += RFV2_MINRESP;
		}
		break;
	default:
		cmn_err(CE_PANIC, "rf_hdrfcanon: bad version\n");
		/* NOTREACHED */
	}

	end -= RF_MCSZ;

	return rf_fcanon(fmt, head, end, head);
}

/*
 * Convert directory entries to canonical form.
 * to must allow for worst-case expansion.
 */
int
rf_dentcanon(count, from, to)
	register long		count;
	register caddr_t	from;
	register caddr_t	to;

{
	register int		tlen = 0;
	register int		tcc;
	struct dirent		*dir;
	register caddr_t	tmp = to;

	while (count > 0) {
		dir = (struct dirent *)from;
		tcc = rf_tcanon(DIRENT_FMT, from, tmp);
		tcc = (tcc + 3) & ~3;
		tmp += tcc;
		tlen += tcc;
		from += dir->d_reclen;
		count -= dir->d_reclen;
	}
	return tlen;
}

/*
 * This routine is called to convert directory entries from canonical form
 * to local form.  Does in-place conversion in data, returning
 * the cumulative total of directory entry lengths, or zero for failure.
 */
int
rf_denfcanon(count, data, end)
	register long		count;
	caddr_t			data;
	register caddr_t	end;
{
	register caddr_t	from;
	register caddr_t	to;
	register int		tlen;
	register int		tcc;
	struct dirent		*dir;

	tlen = 0;
	from = to = data;
	while (count > 0) {
		tcc = 4 * sizeof(long) +
		  ((strlen(from + 4 * sizeof(long)) + 1 + 3) & ~3);
		if (!rf_fcanon(DIRENT_FMT, from, end, to)) {
			return 0;
		}
		dir = (struct dirent *)to;
		to += dir->d_reclen;
		tlen += dir->d_reclen;
		from += tcc;
		count -= tcc;
	}
	return tlen;
}
