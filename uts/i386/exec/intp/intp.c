/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-exe:intp/intp.c	1.3"

#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/signal.h"
#include "sys/cred.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/file.h"
#include "sys/buf.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/fstyp.h"
#include "sys/acct.h"
#include "sys/sysinfo.h"
#include "sys/reg.h"
#include "sys/var.h"
#include "sys/immu.h"
#include "sys/proc.h"
#include "sys/tuneable.h"
#include "sys/tty.h"
#include "sys/cmn_err.h"
#include "sys/debug.h"
#include "sys/rf_messg.h"
#include "sys/conf.h"
#include "sys/uio.h"
#include "sys/pathname.h"
#include "sys/disp.h"
#include "sys/exec.h"
#include "sys/kmem.h"

#ifndef i386
short intpmagic = 0x2321;		/* magic number for cunix */
#else
short intpmagic = 0x2123;
#endif
	
STATIC int getintphead();		/* cracks open a '#!' line */

#define	INTPSZ	256			/* max size of '#!' line allowed */

struct intpdata {			/* holds interpreter name & argument data */

	char	*line1p;		/* points to dyn. alloc. buf of size INTPSZ */
	int	intp_ssz;		/* size of compacted pathname & arg */
	char	*intp_name;		/* points to name part */
	char	*intp_arg;		/* points to arg part */
};



/*
 * Crack open a '#!' line.
 */
STATIC int
getintphead(idatap, ehdp)
	register struct intpdata *idatap;
	exhda_t *ehdp;
{
	register int error;
	register char *cp, *linep;
	int rdsz;
	int ssz = 0;

	/*
	 * Read the entire line and confirm that it starts with '#!'.
	 */
	rdsz = INTPSZ;
	if (rdsz > ehdp->vnsize)
		rdsz = ehdp->vnsize;
	if ((error = exhd_getmap(ehdp, (off_t) 0, rdsz,
			EXHD_COPY, idatap->line1p)) != 0)
		return error;
	linep = idatap->line1p;
	if (linep[0] != '#' || linep[1] != '!')
		return ENOEXEC;
	/*
	 * Blank all white space and find the newline.
	 */
	cp = &linep[2];
	linep += rdsz;
	for (; cp < linep && *cp != '\n'; cp++)
		if (*cp == '\t')
			*cp = ' ';
	if (cp >= linep)
		return E2BIG;

	ASSERT(*cp == '\n');
	*cp = '\0';

	/*
	 * Locate the beginning and end of the interpreter name.
	 * In addition to the name, one additional argument may
	 * optionally be included here, to be prepended to the
	 * arguments provided on the command line.  Thus, for
	 * example, you can say
	 *
	 * 	#! /usr/bin/awk -f
	 */
	for (cp = &idatap->line1p[2]; *cp == ' '; cp++)
		;
	if (*cp == '\0')
		return ENOEXEC;
	idatap->intp_name = cp;
	while (*cp && *cp != ' ') {
		ssz++;
		cp++;
	}
	ssz++;
	if (*cp == '\0')
		idatap->intp_arg = NULL;
	else {
		*cp++ = '\0';
		while (*cp == ' ')
			cp++;
		if (*cp == '\0')
			idatap->intp_arg = NULL;
		else {
			idatap->intp_arg = cp;
			while (*cp && *cp != ' ') {
				ssz++;
				cp++;
			}
			*cp = '\0';
			ssz++;
		}
	}
	idatap->intp_ssz = ssz;
	return 0;
}

int
intpexec(vp, args, level, execsz, ehdp, setid)
	struct vnode *vp;
	struct uarg *args;
	int level;
	long *execsz;
	exhda_t *ehdp;
	int setid;
{
	vnode_t *nvp;
	int num, error = 0;
	char devfd[14];
	int fd = -1;
	struct intpdata idata;
	struct pathname intppn;


	if (level) 	/* Can't recurse */
		return(ENOEXEC);

	/* allocate buffer for interpreter pathname */
	idata.line1p = (char *)kmem_alloc(INTPSZ,  KM_SLEEP);

	if ((error = getintphead(&idata, ehdp)) != 0)
		goto bad;
	/*
	 * Look the new vnode up.
	 */
	if (error = pn_get(idata.intp_name, UIO_SYSSPACE, &intppn))
		goto bad;

	if (error = lookuppn(&intppn, FOLLOW, NULLVPP, &nvp)) {
		pn_free(&intppn);
		goto bad;
	}
	pn_free(&intppn);

	num = 1;
	if (idata.intp_arg)
		num++;
	args->prefixc = num;
	args->prefixp = &idata.intp_name;
	args->prefixsize = idata.intp_ssz;
	
	if (setid) { /* close security hole */
		strcpy(devfd, "/dev/fd/");
		if (error = execopen(&vp, &fd)) {
			VN_RELE(nvp);
			goto bad;
		}
		numtos(fd, &devfd[8]);
		args->fname = devfd;
	}
	
	error = gexec(&nvp, args, ++level, execsz);
	VN_RELE(nvp);

	if (error)
		goto bad;

	/* before returning, free buffer allocated for interpreter pathname */
	kmem_free(idata.line1p, INTPSZ);

	return 0;

bad:
	/* free buffer allocated for interpreter pathname */
	kmem_free(idata.line1p, INTPSZ);

	if (fd != -1)
		(void)execclose(fd);
	return error;
}

intpcore()
{
#ifdef DEBUG
	printf("intpcore()\n");
#endif
	return 0;
}
