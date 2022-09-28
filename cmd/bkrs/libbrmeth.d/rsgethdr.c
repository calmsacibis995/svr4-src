/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)bkrs:libbrmeth.d/rsgethdr.c	1.8.3.1"

#include <limits.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <backup.h>
#include <bktypes.h>
#include <bkrs.h>
#include <brarc.h>
#include <devmgmt.h>
#include "libadmIO.h"

#define LABELSIZ 6

extern long	atol();
extern int	bklevels;
extern int	br_read_hdr();
extern int	close();
extern int	g_close();
extern GFILE	*g_open();
extern int	sprintf();
extern int	strfind();
extern int	strncmp();
extern char	*strcpy();
extern char	*strcat();

int
rsgethdr( ddevice, dchar, vol, ai, flags, fdpt )
char *ddevice, *dchar, *vol;
register struct archive_info *ai;
int	flags;
GFILE	**fdpt;
{
	short volprompt = 0, isdpart = 0;
	long capacity = 0l;
	char *type = NULL, dev[PATH_MAX+1], *d=ddevice, result[BKLABEL_SZ + 1];
	int	i, options;
	GFILE	*f;

#ifdef TRACE
	brlog( "rsgethdr ddevice=%s dchar=%s vol=%s fdp=%x",
		ddevice, dchar, vol, fdpt );
#endif

	options = DM_AUTO;
	if( flags & BR_LABEL_CHECK ) options |= DM_CHKLBL;
	if( !(flags & BR_PROMPT_ALLWD) ) options |= DM_BATCH;

#ifdef TRACE
	brlog( "rsgethdr(): Calling getvol(), options=0x%x", options );
#endif

	if( i = getvol( ddevice, dchar, vol, result, BKLABEL_SZ,
		options, (char *)0 ) ) {
#ifdef TRACE
		brlog( "rsgethdr(): getvol() return=%d", i );
#endif
		return( i );
	}

	if( (flags & BR_PROMPT_ALLWD) && strfind(dchar, "volume=" ) >= 0)
		volprompt++;

	if( (i = strfind(dchar, "type=")) >= 0 )
		/* skip past "type=" */
		type = dchar + (i + 5);

	if(type) {
		if(!(strncmp(type, "dpart", 5))) {
			isdpart++;
			i = strfind(dchar, "capacity=");
			if(i >= 0) {
				capacity = atol(dchar + (i + 9));
			}
			if(capacity <= 0)
				return(-1);
		} else if( !strncmp(type, "dir", 3) ) {
			(void) sprintf( dev, "%s/%s", ddevice, vol );
			d = dev;
		}
	}

	capacity = isdpart? (capacity << 9): 0l;

#ifdef TRACE
    brlog( "rsgethdr(): g_open() d=%s", d );
#endif
    f = g_open(d, O_RDONLY, 0);
    if(f == NULL)
		return(-2);

    i = br_read_hdr(&f, ai, capacity, d);

#ifdef TRACE
    brlog( "rsgethdr(): br_read_hdr() ret=%d f->_file=%d d=%s",
		i, f->_file, d );
#endif

    if(fdpt)
		(*fdpt) = f;
    else
		(void) g_close(f);

    return( i <= 0? -3: 0 );
}
