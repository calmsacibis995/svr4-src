/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1984, 1986, 1987, 1988, 1989 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)libns:nsblock.c	1.7.5.1"
#include <stdio.h>
#include <string.h>
#include <tiuser.h>
#include <nsaddr.h>
#include "nslog.h"
#include "nsdb.h"
#include "stdns.h"
#include "nserve.h"

static int	Mflag = FALSE;	/* did we malloc space	*/
static int	getqd();
static int	putqd();
static int	getrlist();
static int	putrlist();
static struct header	*getheader();
/*
 *	btoreq converts an ascii block in canonical format into
 *	a request structure.  It returns a pointer to that structure
 *	or NULL if it fails.  btoreq allocates the request structure,
 *	which should be freed by the caller when through.
 */
struct request	*
btoreq(block,size)
char	*block;
int	size;
{
	struct request *rp; /* pointer to request */
	struct header	*hp;
	place_p	pp;

	LOG2(L_TRACE, "(%5d) enter: btoreq\n", Logstamp);
	if ((rp = (struct request *) calloc(1,sizeof(struct request))) == NULL) {
		LOG3(L_ALL,"(%5d) btoreq: malloc(%d) FAILED\n",
			Logstamp, sizeof(struct request));
		LOG2(L_TRACE, "(%5d) leave: btoreq\n", Logstamp);
		return(NULL);
	}

	pp = setplace(block,size);

	if ((rp->rq_head = getheader(pp)) == NULL) {
		LOG2(L_BLOCK,"(%5d) btoreq: getheader failed\n",Logstamp);
		free(rp);free(pp);
		LOG2(L_TRACE, "(%5d) leave: btoreq\n", Logstamp);
		return(NULL);
	}

	hp = rp->rq_head;
	/* now get the other sections	*/

	if (getqd(pp,hp->h_qdcnt,&(rp->rq_qd)) == FAILURE) {
		LOG2(L_BLOCK,"(%5d) btoreq: getqd failed\n",Logstamp);
		free(rp);free(pp);
		LOG2(L_TRACE, "(%5d) leave: btoreq\n", Logstamp);
		return(NULL);
	}
	if (getrlist(pp,hp->h_ancnt,&(rp->rq_an)) == FAILURE) {
		LOG2(L_BLOCK,"(%5d) btoreq: getrlist failed on an\n",Logstamp);
		free(rp);free(pp);
		LOG2(L_TRACE, "(%5d) leave: btoreq\n", Logstamp);
		return(NULL);
	}
	if (getrlist(pp,hp->h_nscnt,&(rp->rq_ns)) == FAILURE) {
		LOG2(L_BLOCK,"(%5d) btoreq: getrlist failed on ns\n",Logstamp);
		free(rp);free(pp);
		LOG2(L_TRACE, "(%5d) leave: btoreq\n", Logstamp);
		return(NULL);
	}
	if (getrlist(pp,hp->h_arcnt,&(rp->rq_ar)) == FAILURE) {
		LOG2(L_BLOCK,"(%5d) btoreq: getrlist failed on ar\n",Logstamp);
		free(rp);free(pp);
		LOG2(L_TRACE, "(%5d) leave: btoreq\n", Logstamp);
		return(NULL);
	}
	free(pp);
	LOG2(L_TRACE, "(%5d) leave: btoreq\n", Logstamp);
	return(rp);
}
static struct header *
getheader(pp)
place_p	pp;
{
	struct header *hp;

	LOG2(L_TRACE, "(%5d) enter: getheader\n", Logstamp);
	if ((hp = (struct header *) malloc(sizeof(struct header))) == NULL) {
		LOG3(L_ALL,"(%5d) getheader: malloc(%d) FAILED\n",
			Logstamp,sizeof(struct header));
		LOG2(L_TRACE, "(%5d) leave: getheader\n", Logstamp);
		return(NULL);
	}
	hp->h_version = getlong(pp);
	hp->h_flags   = getlong(pp);
	hp->h_opcode  = getlong(pp);
	hp->h_rcode   = getlong(pp);
	hp->h_qdcnt   = getlong(pp);
	hp->h_ancnt   = getlong(pp);
	hp->h_nscnt   = getlong(pp);
	hp->h_arcnt   = getlong(pp);
	hp->h_dname   = getstr(pp,NULL,NULL);
	LOG2(L_TRACE, "(%5d) leave: getheader\n", Logstamp);
	return(hp);
}
static int
getqd(pp,count,qpp)
place_p pp;
long	count;
struct question	***qpp;
{
	int	i;
	struct question	*qra;
	struct question	**qp;

	LOG2(L_TRACE, "(%5d) enter: getqd\n", Logstamp);
	if (count == 0) {
		*qpp = (struct question **) NULL;
		LOG2(L_TRACE, "(%5d) leave: getqd\n", Logstamp);
		return(SUCCESS);
	}
	if (*qpp == NULL)
		if ((*qpp = (struct question **)
		     calloc(count,sizeof(struct question *))) == NULL) {
			LOG4(L_ALL,"(%5d) getqd: *qpp = calloc(%d,%d) failed\n",
				Logstamp,count,sizeof(struct question *));
			LOG2(L_TRACE, "(%5d) leave: getqd\n", Logstamp);
			return(FAILURE);
		}

	if ((qra = (struct question *)calloc(count,sizeof(struct question)))
	    == NULL) {
		LOG4(L_ALL,"(%5d) getqd: qra = calloc(%d,%d) failed\n",
			Logstamp,count,sizeof(struct question));
		LOG2(L_TRACE, "(%5d) leave: getqd\n", Logstamp);
		return(FAILURE);
	}

	for (i=0, qp = *qpp; i < count; i++, qp++, qra++) {
		*qp = qra;
		qra->q_name = getstr(pp,NULL,NULL);
		qra->q_type = getlong(pp);
	}
	LOG2(L_TRACE, "(%5d) leave: getqd\n", Logstamp);
	return(SUCCESS);
}
static int
getrlist(pp,count,rpp)
place_p	pp;
long	count;
struct res_rec	***rpp;
{
	int	i;
	struct res_rec	*rra;
	struct res_rec	**rp;

	LOG2(L_TRACE, "(%5d) enter: getrlist\n", Logstamp);
	if (count == 0) {
		*rpp = (struct res_rec **) NULL;
		LOG2(L_TRACE, "(%5d) leave: getrlist\n", Logstamp);
		return(SUCCESS);
	}
	if (*rpp == NULL)
		if ((*rpp = (struct res_rec **)
		     calloc(count,sizeof(struct res_rec *))) == NULL) {
			LOG4(L_ALL,"(%5d) getrlist: *rpp = calloc(%d,%d) failed\n",
				Logstamp,count,sizeof(struct res_rec *));
			LOG2(L_TRACE, "(%5d) leave: getrlist\n", Logstamp);
			return(FAILURE);
		}

	if ((rra = (struct res_rec *)calloc(count,sizeof(struct res_rec)))
	    == NULL) {
		LOG4(L_ALL,"(%5d) getrlist: rra = calloc(%d,%d) failed\n",
			Logstamp,count,sizeof(struct question));
		LOG2(L_TRACE, "(%5d) leave: getrlist\n", Logstamp);
		return(FAILURE);
	}
	for (i=0, rp = *rpp; i < count; i++, rp++, rra++) {
		*rp = rra;
		if (getrrec(pp,rra) != SUCCESS)
			return(FAILURE);
	}
	LOG2(L_TRACE, "(%5d) leave: getrlist\n", Logstamp);
	return(SUCCESS);
}
int
getrrec(pp,rp)
place_p	pp;
struct res_rec	*rp;
{

	LOG2(L_TRACE, "(%5d) enter: getrrec\n", Logstamp);
	getstr(pp,rp->rr_name,NAMSIZ);
	rp->rr_type = getlong(pp);
	switch (rp->rr_type) {
	case RN:
		if ((rp->rr_rn = (struct rn *) malloc(sizeof(struct rn))) == NULL) {
			LOG3(L_ALL,"(%5d) getrrec: malloc(%d) failed\n",
				Logstamp,sizeof(struct rn));
			LOG2(L_TRACE, "(%5d) leave: getrrec\n", Logstamp);
			return(FAILURE);
		}
		rp->rr_owner = getstr(pp,NULL,NULL);
		rp->rr_desc = getstr(pp,NULL,NULL);
		rp->rr_path = getstr(pp,NULL,NULL);
		rp->rr_flag = getlong(pp);
		break;
	case DOM:
		if ((rp->rr_dom = (struct domain *) malloc(sizeof(struct domain)))
		    == NULL) {
			LOG3(L_ALL,"(%5d) getrrec: domain malloc(%d) failed\n",
				Logstamp,sizeof(struct domain));
			LOG2(L_TRACE, "(%5d) leave: getrrec\n", Logstamp);
			return(FAILURE);
		}
		break;
	default: /* all the rest fall in this category	*/
		rp->rr_data = getstr(pp,NULL,NULL);
		break;
	}
	LOG2(L_TRACE, "(%5d) leave: getrrec\n", Logstamp);
	return(SUCCESS);
}
/*
 *	reqtob writes a request structure into a block.
 *	If block == NULL, reqtob allocates enough space.
 *	reqtob returns a pointer to the block or NULL,
 *	if the operation fails.
 */
char	*
reqtob(rp,block,size)
struct request	*rp;
char		*block;
int		*size;
{
	int	rsize;	/* real size of current block	*/
	place_p	pp;
	struct header	*hp;

	LOG2(L_TRACE, "(%5d) enter: reqtob\n", Logstamp);
	if (block == NULL) {	/* allocate some space	*/
		if ((block = malloc(DBLKSIZ)) == NULL) {
			LOG3(L_ALL,"(%5d) reqtob: malloc(%d) failed\n",
				Logstamp,DBLKSIZ);
			LOG2(L_TRACE, "(%5d) leave: reqtob\n", Logstamp);
			return(NULL);
		}
		rsize = *size = DBLKSIZ;
		Mflag = TRUE;
	}
	else
		rsize = *size;

	pp = setplace(block,rsize);

	*size = 0;	/* set for error case, resets anyway if ok	*/
	hp = rp->rq_head;
	if (putheader(pp,hp) == FAILURE) {
		LOG2(L_BLOCK,"(%5d) reqtob: putheader failed\n",Logstamp);
		if (Mflag) free(block);
		free(pp);
		LOG2(L_TRACE, "(%5d) leave: reqtob\n", Logstamp);
		return(NULL);
	}

	/* now get the other sections	*/

	if (putqd(pp,hp->h_qdcnt,rp->rq_qd) == FAILURE) {
		LOG2(L_BLOCK,"(%5d) reqtob: putqd failed\n",Logstamp);
		if (Mflag) free(block);
		free(pp);
		LOG2(L_TRACE, "(%5d) leave: reqtob\n", Logstamp);
		return(NULL);
	}
	if (putrlist(pp,hp->h_ancnt,rp->rq_an) == FAILURE) {
		LOG2(L_BLOCK,"(%5d) reqtob: putrlist failed on an\n",Logstamp);
		if (Mflag) free(block);
		free(pp);
		LOG2(L_TRACE, "(%5d) leave: reqtob\n", Logstamp);
		return(NULL);
	}
	if (putrlist(pp,hp->h_nscnt,rp->rq_ns) == FAILURE) {
		LOG2(L_BLOCK,"(%5d) reqtob: putrlist failed on ns\n",Logstamp);
		if (Mflag) free(block);
		free(pp);
		LOG2(L_TRACE, "(%5d) leave: reqtob\n", Logstamp);
		return(NULL);
	}
	if (putrlist(pp,hp->h_arcnt,rp->rq_ar) == FAILURE) {
		LOG2(L_BLOCK,"(%5d) reqtob: putrlist failed on ar\n",Logstamp);
		if (Mflag) free(block);
		free(pp);
		LOG2(L_TRACE, "(%5d) leave: reqtob\n", Logstamp);
		return(NULL);
	}

	*size = pp->p_ptr - pp->p_start;
	free(pp);
	LOG2(L_TRACE, "(%5d) leave: reqtob\n", Logstamp);
	return(pp->p_start);
}
int
putheader(pp,hp)
place_p	pp;
struct header	*hp;
{
	LOG2(L_TRACE, "(%5d) enter: putheader\n", Logstamp);
	/* check size first	*/
	if (overbyte(pp,c_sizeof(hp->h_dname)+8*L_SIZE) &&
	    explace(pp,0) == FAILURE) {
		LOG2(L_TRACE, "(%5d) leave: putheader\n", Logstamp);
		return(FAILURE);
	}

	if (putlong(pp,(long)NSVERSION)    == FAILURE ||
	    putlong(pp,(long)hp->h_flags)  == FAILURE ||
	    putlong(pp,(long)hp->h_opcode) == FAILURE ||
	    putlong(pp,(long)hp->h_rcode)  == FAILURE ||
	    putlong(pp,(long)hp->h_qdcnt)  == FAILURE ||
	    putlong(pp,(long)hp->h_ancnt)  == FAILURE ||
	    putlong(pp,(long)hp->h_nscnt)  == FAILURE ||
	    putlong(pp,(long)hp->h_arcnt)  == FAILURE ||
	    putstr(pp,hp->h_dname)        == FAILURE) {
		LOG2(L_TRACE, "(%5d) leave: putheader\n", Logstamp);
		return(FAILURE);
	}

	LOG2(L_TRACE, "(%5d) leave: putheader\n", Logstamp);
	return(SUCCESS);
}
static int
putqd(pp,count,qp)
place_p pp;
int	count;
struct question	**qp;
{
	int	i;

	LOG2(L_TRACE, "(%5d) enter: putqd\n", Logstamp);
	/* put each query into block	*/
	for (i=0; i < count; i++, qp++) {
		/* size is strlen + room for NULL + type + class */
		if (overbyte(pp,c_sizeof((*qp)->q_name)+L_SIZE) &&
		    explace(pp,0) == FAILURE) {
			LOG2(L_TRACE, "(%5d) enter: putqd\n", Logstamp);
			return(FAILURE);
		}

		putstr(pp,(*qp)->q_name);
		putlong(pp,(long)(*qp)->q_type);
	}
	LOG2(L_TRACE, "(%5d) enter: putqd\n", Logstamp);
	return(SUCCESS);
}
static int
putrlist(pp,count,rp)
place_p	pp;
int	count;
struct res_rec	**rp;
{
	int	i;

	LOG2(L_TRACE, "(%5d) enter: putrlist\n", Logstamp);
	for (i=0; i < count; i++, rp++)
		if (putrrec(pp,*rp) != SUCCESS) {
			LOG2(L_TRACE, "(%5d) leave: putrlist\n", Logstamp);
			return(FAILURE);
		}

	LOG2(L_TRACE, "(%5d) leave: putrlist\n", Logstamp);
	return(SUCCESS);
}
int
putrrec(pp,rp)
place_p	pp;
struct res_rec	*rp;
{

	LOG2(L_TRACE, "(%5d) enter: putrrec\n", Logstamp);
	if (overbyte(pp,c_sizeof(rp->rr_name)+L_SIZE) && explace(pp,0) == FAILURE) {
		LOG2(L_TRACE, "(%5d) leave: putrrec\n", Logstamp);
		return(FAILURE);
	}

	putstr(pp,rp->rr_name);
	putlong(pp,(long)rp->rr_type);
	switch (rp->rr_type) {
	case RN:
		if (overbyte(pp,c_sizeof(rp->rr_owner) +
		    c_sizeof(rp->rr_desc) +
		    c_sizeof(rp->rr_path) + L_SIZE) &&
		    explace(pp,0) == FAILURE) {
			LOG2(L_TRACE, "(%5d) leave: putrrec\n", Logstamp);
			return(FAILURE);
		}
			
		putstr(pp,rp->rr_owner);
		putstr(pp,rp->rr_desc);
		putstr(pp,rp->rr_path);
		putlong(pp,(long)rp->rr_flag);
		break;
	case DOM:
		/* nothing else needed for domain	*/
		break;
	default:
		if (overbyte(pp,c_sizeof(rp->rr_data)) &&
		    explace(pp,0) == FAILURE)
			return(FAILURE);

		putstr(pp,rp->rr_data);
		break;
	}
	LOG2(L_TRACE, "(%5d) leave: putrrec\n", Logstamp);
	return(SUCCESS);
}
