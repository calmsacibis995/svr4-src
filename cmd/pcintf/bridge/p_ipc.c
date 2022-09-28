/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcintf:bridge/p_ipc.c	1.1"
#include	"system.h"
#include	"sccs.h"
SCCSID(@(#)p_ipc.c	3.13	LCC);	/* Modified: 15:52:56 9/6/89 */

/****************************************************************************

	Copyright (c) 1985 Locus Computing Corporation.
	All rights reserved.
	This is an unpublished work containing CONFIDENTIAL INFORMATION
	that is the property of Locus Computing Corporation.
	Any unauthorized use, duplication or disclosure is prohibited.

****************************************************************************/

#ifndef NOIPC
#include "pci_types.h"
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/errno.h>
#include "flip.h"

#define SUCCESS 0
#define FAIL (-1)

long tmpLong;
short tmp;
extern int errno;

/*
 * Extended Protocol Header for Locus Computing Corporation PC-Interface
 * Bridge. The information for doing IPC messages is contained in the
 * hdr.text field of the incoming packet. Overlay the following
 * structures to get at the information.
 */

struct pci_msgbuf {
	long	mtype;		/* message type */
	char	mtext[1];	/* message text */
};

struct pci_ipc_perm {
	ushort	uid;		/* owner's user id */
	ushort	gid;		/* owner's group id */
	ushort	cuid;		/* creator's user id */
	ushort	cgid;		/* creator's group id */
	ushort	mode;		/* access modes */
	ushort	seq;		/* slut usage sequence number */
	long	key;		/* key  (key_t) */
};

struct pci_msqid_ds {
	struct pci_ipc_perm	msg_perm;	/* operation perm struct */
	long			msg_stime;	/* last msgsnd time */
	long			msg_rtime;	/* last msgrcv time */
	long			msg_ctime;	/* last change time */
	ushort			msg_cbytes;	/* current # bytes on q */
	ushort			msg_qnum;	/* # of messages on q */
	ushort			msg_qbytes;	/* max # of bytes on q */
	ushort			msg_lspid;	/* pid of last msgsnd */
	ushort			msg_lrpid;	/* pid of last msgrcv */
};

struct pci_semid_ds {
	struct pci_ipc_perm	sem_perm;	/* operation perm struct */
	long			sem_otime;	/* last semop time */
	long			sem_ctime;	/* last change time */
	ushort			sem_nsems;	/* # of semaphores in set */
};


struct msgget_type {
	key_t			key;
	long			flag;
};

struct msgsnd_type {
        long			id;
	long			size;
	long			flag;
	struct pci_msgbuf	message;
};

struct msgrcv_type {
	long	 		id;
	long			size;
	long			flag;
	long			type;
	struct pci_msgbuf	message;
};

struct msgctl_type {
	long			id;
	long			command;
	struct pci_msqid_ds	buffer;
}; 

struct semget_type {
	key_t			key;
	long			number;
	long			flag;
};

struct semop_type {
	long			id;
	long			number;
	struct pci_sembuf	(*sops)[];
};

struct semctl_type {
	long			id;
	long			number;
	long			command;
	union {
	    long		val;
	    struct pci_semid_ds	dsbuf;
	    ushort		dsarr[1];
	} dsret;
};

/*
 * get a message queue id 
 */

int 
p_msgget( data , out )
char *data;
struct output *out;
{
	int rc;
	struct msgget_type *buf;

	buf = (struct msgget_type *)data;
#if	HOW_TO_FLIP != NOFLIP
	lflipm(buf->key,tmpLong,LSFLIP);
	lflipm(buf->flag,tmpLong, LSFLIP);
#endif
	log("msgget parameters: key %ld  flag %x\n", buf->key,buf->flag);
	do
	    rc = msgget( (key_t)buf->key, (int)buf->flag ); 
	while (rc == -1 && errno == EINTR);
	out->hdr.res = (rc >= 0) ? SUCCESS : errno;
	out->hdr.fdsc = rc; /* return msg queue id */
	/* has UNIX run out of msg queue IDs? */
	if (rc < 0 && ((int)buf->flag & IPC_CREAT) && errno == ENOSPC)
	    serious("Cannot create message queue ID, errno = %d\n", errno);
	log("MSGGET Returned %d\n",rc);
}

/*
 * Send a message
 */

int
p_msgsnd( data , out )
char *data;
struct output *out;
{
    register int rc;
    struct msgsnd_type *buf;
    register int i;

	buf = (struct msgsnd_type *) data;
#if	HOW_TO_FLIP != NOFLIP
	lflipm(buf->id,tmpLong,LSFLIP);
	lflipm(buf->flag,tmpLong,LSFLIP);
	lflipm(buf->size,tmpLong,LSFLIP);
	lflipm(buf->message.mtype,tmpLong,LSFLIP);
#endif
	log("msgsnd paramters:\n");
	log("buf->id %d  buf->size %d  buf->flag %d\n",
	    buf->id ,buf->size ,buf->flag);
	log("          message: %s \n",buf->message.mtext); 

	rc = msgsnd( (int)buf->id, (struct msgbuf *)&buf->message,
		     (int)buf->size, (int)buf->flag);
	out->hdr.res = (rc >= 0) ? SUCCESS : errno;
	out->hdr.fdsc = rc; 
}

static char *msgsnd_buf = NULL;
static char *msgsnd_bp = NULL;
static int   msgsnd_len = 0;

int
p_msgsnd_new(data, data_len, out)
char *data;
int data_len;
struct output *out;
{
	struct msgsnd_type *buf;

	buf = (struct msgsnd_type *) data;
	msgsnd_len = buf->size;
#if	HOW_TO_FLIP != NOFLIP
	lflipm(msgsnd_len, tmpLong, LSFLIP);
#endif
	msgsnd_len += sizeof (struct msgsnd_type);

	log("msgsnd_new: data_len = %d, size = %d\n", data_len, msgsnd_len);
	if (msgsnd_buf != NULL) {
		free(msgsnd_buf);
		msgsnd_buf = NULL;
	}
	if (msgsnd_len < data_len) {
		log("msgsnd_new: Too much data\n");
		out->hdr.res = E2BIG;
		return;
	}
	if ((msgsnd_buf = (char *)malloc(msgsnd_len)) == NULL) {
		log("msgsnd_new: Couldn't allocate buffer\n");
		out->hdr.res = EINVAL;
		return;
	}
	memcpy(msgsnd_buf, data, data_len);
	msgsnd_bp = &msgsnd_buf[data_len];
	out->hdr.res = SUCCESS;
}

int
p_msgsnd_ext(data, data_len, out)
char *data;
int data_len;
struct output *out;
{
	log("msgsnd_ext: data_len = %d\n", data_len);
	if (msgsnd_len < (msgsnd_bp - msgsnd_buf) + data_len) {
		log("msgsnd_ext: Too much data\n");
		out->hdr.res = E2BIG;
		return;
	}
	memcpy(msgsnd_bp, data, data_len);
	msgsnd_bp += data_len;
	out->hdr.res = SUCCESS;
}

int
p_msgsnd_end(data, data_len, out)
char *data;
int data_len;
struct output *out;
{
	log("msgsnd_end: data_len = %d\n", data_len);
	if (msgsnd_len < (msgsnd_bp - msgsnd_buf) + data_len) {
		log("msgsnd_ext: Too much data\n");
		out->hdr.res = E2BIG;
		return;
	}
	memcpy(msgsnd_bp, data, data_len);
	msgsnd_bp += data_len;
	p_msgsnd(msgsnd_buf, out);
	free(msgsnd_buf);
	msgsnd_buf = NULL;
}


/*
 * Receive a message
 */

int
p_msgrcv( data , out)
char *data;
struct output *out;
{

	int rc;
	register struct msgrcv_type *buf;
	register int i;
	struct pci_msgbuf *mbp;
	char *op;

	buf = (struct msgrcv_type *)data;
#if	HOW_TO_FLIP != NOFLIP
	lflipm(buf->id,tmpLong,LSFLIP);
	lflipm(buf->size,tmpLong,LSFLIP);  
	lflipm(buf->type,tmpLong,LSFLIP);
	lflipm(buf->flag,tmpLong,LSFLIP);
#endif
	log("msgrcv parameters: id %d  type %ld  size %d  flag %d\n",
	     buf->id,buf->type,buf->size, buf->flag);

	if (buf->size > MAX_OUTPUT) {
		log("msgrcv: Too large a packet\n");
		out->hdr.res = E2BIG;
		return;
	}
	op = &out->text[-4];
	mbp = (struct pci_msgbuf *)op;
	rc = msgrcv( (int)buf->id, (struct msgbuf *)mbp, 
		     (int)buf->size, (long)buf->type, (int)buf->flag);
	if ( rc != -1) 
	{
             log("(type %ld) : %s\n",buf->message.mtype,buf->message.mtext);
             out->hdr.t_cnt = rc;
	     out->hdr.f_size = mbp->mtype;
             out->hdr.res =  SUCCESS; 
         }
	 else out->hdr.res =  errno;    
	 out->hdr.fdsc = rc; 
	 log("MSGRCV Returned %d\n",out->hdr.fdsc);
}

/*
 * issue a message queue control operation
 */

int 
p_msgctl( data , out )
char *data;
struct output *out;
{
    int i, rc;
    register struct msgctl_type *buf;
    register char *cptr;
    struct msqid_ds msgstat;

	buf = (struct msgctl_type *) data;
#if	HOW_TO_FLIP != NOFLIP
	lflipm(buf->id,tmpLong,LSFLIP);
	lflipm(buf->command,tmpLong,LSFLIP);
	if (buf->command == IPC_SET)
	{
	    sflipm(buf->buffer.msg_perm.uid,tmp,SFLIP); 
	    sflipm(buf->buffer.msg_perm.gid,tmp,SFLIP); 
	    sflipm(buf->buffer.msg_perm.cuid,tmp,SFLIP); 
    	    sflipm(buf->buffer.msg_perm.cgid,tmp,SFLIP); 
	    sflipm(buf->buffer.msg_perm.mode,tmp,SFLIP); 
	    sflipm(buf->buffer.msg_perm.seq,tmp,SFLIP); 
	    lflipm(buf->buffer.msg_perm.key,tmpLong,LSFLIP);
	    sflipm(buf->buffer.msg_cbytes,tmp,SFLIP); 
	    sflipm(buf->buffer.msg_qnum,tmp,SFLIP); 
	    sflipm(buf->buffer.msg_qbytes,tmp,SFLIP); 
	    sflipm(buf->buffer.msg_lspid,tmp,SFLIP); 
	    sflipm(buf->buffer.msg_lrpid,tmp,SFLIP); 
	    lflipm(buf->buffer.msg_stime,tmpLong,LSFLIP);
	    lflipm(buf->buffer.msg_rtime,tmpLong,LSFLIP);
	    lflipm(buf->buffer.msg_ctime,tmpLong,LSFLIP);
	}
#endif
	msgstat.msg_perm.uid = buf->buffer.msg_perm.uid; 
	msgstat.msg_perm.gid = buf->buffer.msg_perm.gid; 
	msgstat.msg_perm.cuid = buf->buffer.msg_perm.cuid; 
    	msgstat.msg_perm.cgid = buf->buffer.msg_perm.cgid; 
	msgstat.msg_perm.mode = buf->buffer.msg_perm.mode; 
	msgstat.msg_perm.seq = buf->buffer.msg_perm.seq; 
	msgstat.msg_perm.key = buf->buffer.msg_perm.key;
	msgstat.msg_cbytes = buf->buffer.msg_cbytes; 
	msgstat.msg_qnum = buf->buffer.msg_qnum; 
	msgstat.msg_qbytes = buf->buffer.msg_qbytes; 
	msgstat.msg_lspid = buf->buffer.msg_lspid; 
	msgstat.msg_lrpid = buf->buffer.msg_lrpid; 
	msgstat.msg_stime = buf->buffer.msg_stime;
	msgstat.msg_rtime = buf->buffer.msg_rtime;
	msgstat.msg_ctime = buf->buffer.msg_ctime;
	log("msgctl parameters: id %d  command %d\n", buf->id,buf->command);
	do
	    rc = msgctl( (int)buf->id, (int)buf->command, &msgstat);
	while (rc == -1 && errno == EINTR);
	log("MSGCTL Returned %d\n",rc);
	if ( rc != -1)
	{
	    if (buf->command == IPC_STAT)
	    {
		buf->buffer.msg_perm.uid = msgstat.msg_perm.uid;
		buf->buffer.msg_perm.gid = msgstat.msg_perm.gid;
		buf->buffer.msg_perm.cuid = msgstat.msg_perm.cuid;
		buf->buffer.msg_perm.cgid = msgstat.msg_perm.cgid;
		buf->buffer.msg_perm.mode = msgstat.msg_perm.mode;
		buf->buffer.msg_perm.seq = msgstat.msg_perm.seq;
		buf->buffer.msg_perm.key = msgstat.msg_perm.key;
		buf->buffer.msg_cbytes = msgstat.msg_cbytes;
		buf->buffer.msg_qnum = msgstat.msg_qnum;
		buf->buffer.msg_qbytes = msgstat.msg_qbytes;
		buf->buffer.msg_lspid = msgstat.msg_lspid;
		buf->buffer.msg_lrpid = msgstat.msg_lrpid;
		buf->buffer.msg_stime = msgstat.msg_stime;
		buf->buffer.msg_rtime = msgstat.msg_rtime;
		buf->buffer.msg_ctime = msgstat.msg_ctime;

	        log("(msgctl returned) uid %u  gid %u  cuid %u\n",
		     buf->buffer.msg_perm.uid, buf->buffer.msg_perm.gid,
		     buf->buffer.msg_perm.cuid);
	        log("    cgid %u  mode %u  seq %u  key %ld\n",
	             buf->buffer.msg_perm.cgid, buf->buffer.msg_perm.mode,
	             buf->buffer.msg_perm.seq, buf->buffer.msg_perm.key);
	        log("    msg_cbytes %u msg_qnum %u\n",
		     buf->buffer.msg_cbytes, buf->buffer.msg_qnum);
	        log("    msg_qbytes %u msg_lspid %u msg_lrpid %u msg_stime %ld\n",
	             buf->buffer.msg_qbytes,buf->buffer.msg_lspid,
		     buf->buffer.msg_lrpid, buf->buffer.msg_stime);
 	        log("    msg_rtime %ld  msg_ctime %ld\n", 
	             buf->buffer.msg_rtime, buf->buffer.msg_ctime);
	        out->hdr.t_cnt = sizeof(struct pci_msqid_ds);
#if	HOW_TO_FLIP != NOFLIP
	        sflipm(buf->buffer.msg_perm.uid,tmp,SFLIP); 
	        sflipm(buf->buffer.msg_perm.gid,tmp,SFLIP); 
	        sflipm(buf->buffer.msg_perm.cuid,tmp,SFLIP); 
	        sflipm(buf->buffer.msg_perm.cgid,tmp,SFLIP); 
	        sflipm(buf->buffer.msg_perm.mode,tmp,SFLIP); 
	        sflipm(buf->buffer.msg_perm.seq,tmp,SFLIP); 
	        lflipm(buf->buffer.msg_perm.key,tmpLong,LSFLIP);
	        sflipm(buf->buffer.msg_cbytes,tmp,SFLIP); 
	        sflipm(buf->buffer.msg_qnum,tmp,SFLIP); 
	        sflipm(buf->buffer.msg_qbytes,tmp,SFLIP); 
	        sflipm(buf->buffer.msg_lspid,tmp,SFLIP); 
	        sflipm(buf->buffer.msg_lrpid,tmp,SFLIP); 
	        lflipm(buf->buffer.msg_stime,tmpLong,LSFLIP);
	        lflipm(buf->buffer.msg_rtime,tmpLong,LSFLIP);
	        lflipm(buf->buffer.msg_ctime,tmpLong,LSFLIP);
#endif
	        cptr = (char *)&buf->buffer;
	        for (i = 0; i < sizeof(struct pci_msqid_ds); i++)
		    out->text[i] = *cptr++;
	    }
	    else		/* Command NOT IPC_STAT */
		out->hdr.t_cnt = 0;
	    out->hdr.res = SUCCESS;
	}
	else
	    out->hdr.res = errno;
	out->hdr.fdsc = rc; 
}

/*
 * get a semaphore id
 */

int 
p_semget( data , out )
char *data;
struct output *out;
{
    register int rc;
    register struct semget_type *buf;

	buf = (struct semget_type *)data;
#if	HOW_TO_FLIP != NOFLIP
	lflipm(buf->key,tmpLong,LSFLIP);
	lflipm(buf->number,tmpLong, LSFLIP);
	lflipm(buf->flag,tmpLong, LSFLIP);
#endif
	log("semget parameters: key %ld  number %d  flag %x\n", buf->key,
	    buf->number, buf->flag);
	do
	    rc = semget( (key_t)buf->key, (int)buf->number, (int)buf->flag ); 
	while (rc == -1 && errno == EINTR);
	out->hdr.res = (rc >= 0) ? SUCCESS : errno;
	out->hdr.fdsc = rc; /* return semaphore id */
	/* has UNIX run out of semaphores? */
	if (rc < 0 && ((int)buf->flag & IPC_CREAT) && errno == ENOSPC)
	    serious("Cannot create semaphore, errno = %d\n", errno);
	log("SEMGET Returned %d\n",rc);
}

/*
 * perform semaphore operation
 */

int 
p_semop( data , out )
char *data;
struct output *out;
{
    int rc;
    register i;
    register struct semop_type *buf;
    struct sembuf *sbp;

	buf = (struct semop_type *)data;
	sbp = (struct sembuf *)&data[8];

#if	HOW_TO_FLIP != NOFLIP
	lflipm(buf->id,tmpLong, LSFLIP);
	lflipm(buf->number,tmpLong, LSFLIP);
	for (i = 0; i < buf->number; i++) {
	    sflipm(sbp->sem_num, tmp, SFLIP);
	    sflipm(sbp->sem_op, tmp, SFLIP);
	    sflipm(sbp->sem_flg, tmp, SFLIP);
	    sbp++;
	}
	sbp = (struct sembuf *)&data[8];
#endif
	log("semop parameters: id %d  number %d\n", buf->id, buf->number);
	for (i = 0; i < buf->number; i++) {
	    log("  sbp->sem_num   %d\n",sbp->sem_num);
	    log("  sbp->sem_op    %d\n",sbp->sem_op);
	    log("  sbp->sem_flag  %d\n",sbp->sem_flg);
	    sbp++;
	}
	sbp = (struct sembuf *)&data[8];
	do
	    rc = semop( (int)buf->id, sbp, (unsigned)buf->number ); 
	while (rc == -1 && errno == EINTR);
	out->hdr.res = (rc >= 0) ? SUCCESS : errno;
	out->hdr.fdsc = rc;  
	log("SEMOP Returned %d\n",rc);
}

/*
 * Issue a semaphore control operation
 */

int
p_semctl( data, out)
char *data;
struct output *out;
{
    int i, rc;
    register struct semctl_type *buf;
    struct semid_ds semstat;
    struct pci_semid_ds *ostat;
    register char *cptr;

	buf = (struct semctl_type *) data;
#if	HOW_TO_FLIP != NOFLIP
	lflipm(buf->id,tmpLong, LSFLIP);
	lflipm(buf->number,tmpLong, LSFLIP);
	lflipm(buf->command,tmpLong, LSFLIP);
	switch (buf->command) {
	     case SETVAL:
		lflipm(buf->dsret.val,tmpLong,LSFLIP);
		break;
	     case SETALL:
		for (i = 0; i < buf->number; i++)
		    sflipm(buf->dsret.dsarr[i], tmp, SFLIP);
		break;
	     case IPC_SET:
		sflipm(buf->dsret.dsbuf.sem_perm.uid,tmp,SFLIP); 
		sflipm(buf->dsret.dsbuf.sem_perm.gid,tmp,SFLIP); 
		sflipm(buf->dsret.dsbuf.sem_perm.cuid,tmp,SFLIP); 
		sflipm(buf->dsret.dsbuf.sem_perm.cgid,tmp,SFLIP); 
		sflipm(buf->dsret.dsbuf.sem_perm.mode,tmp,SFLIP); 
		sflipm(buf->dsret.dsbuf.sem_perm.seq,tmp,SFLIP); 
		lflipm(buf->dsret.dsbuf.sem_perm.key,tmpLong,LSFLIP);
		break;
	}
#endif
	cptr = (char *)&buf->dsret.dsbuf;
	if (buf->command == IPC_SET) {
		semstat.sem_perm.uid = buf->dsret.dsbuf.sem_perm.uid; 
		semstat.sem_perm.gid = buf->dsret.dsbuf.sem_perm.gid; 
		semstat.sem_perm.cuid = buf->dsret.dsbuf.sem_perm.cuid; 
		semstat.sem_perm.cgid = buf->dsret.dsbuf.sem_perm.cgid; 
		semstat.sem_perm.mode = buf->dsret.dsbuf.sem_perm.mode; 
		semstat.sem_perm.seq = buf->dsret.dsbuf.sem_perm.seq; 
		semstat.sem_perm.key = buf->dsret.dsbuf.sem_perm.key;
		cptr = (char *)&semstat;
	} else if (buf->command == IPC_STAT)
		cptr = (char *)&semstat;
	else if (buf->command == SETVAL)
		cptr = (char *)buf->dsret.val;
	log("semctl parameters:  id %d  number %d  command %d\n",
	    buf->id, buf->number, buf->command);
	do
	    rc = semctl((int)buf->id, (int)buf->number, (int)buf->command, cptr);
	while (rc == -1 && errno == EINTR);
	if (rc != -1) {
	    cptr = (char *)&buf->dsret.dsbuf;
	    if (buf->command == IPC_STAT) {
		ostat = (struct pci_semid_ds *)out->text;
		ostat->sem_perm.uid = semstat.sem_perm.uid;
		ostat->sem_perm.gid = semstat.sem_perm.gid;
		ostat->sem_perm.cuid = semstat.sem_perm.cuid;
		ostat->sem_perm.cgid = semstat.sem_perm.cgid;
		ostat->sem_perm.mode = semstat.sem_perm.mode;
		ostat->sem_perm.seq = semstat.sem_perm.seq;
		ostat->sem_perm.key = semstat.sem_perm.key;
		ostat->sem_nsems = semstat.sem_nsems;
		ostat->sem_otime = semstat.sem_otime;
		ostat->sem_ctime = semstat.sem_ctime;
		out->hdr.t_cnt = sizeof(struct pci_semid_ds);
#if	HOW_TO_FLIP != NOFLIP
		sflipm(ostat->sem_perm.uid,tmp,SFLIP); 
		sflipm(ostat->sem_perm.gid,tmp,SFLIP); 
		sflipm(ostat->sem_perm.cuid,tmp,SFLIP); 
		sflipm(ostat->sem_perm.cgid,tmp,SFLIP); 
		sflipm(ostat->sem_perm.mode,tmp,SFLIP); 
		sflipm(ostat->sem_perm.seq,tmp,SFLIP); 
		lflipm(ostat->sem_perm.key,tmpLong,LSFLIP);
		sflipm(ostat->sem_nsems,tmp,SFLIP);
		lflipm(ostat->sem_otime,tmpLong,LSFLIP);
		lflipm(ostat->sem_ctime,tmpLong,LSFLIP);
#endif
	    } else if (buf->command == GETALL) {
		for (i = 0; i < buf->number * sizeof(ushort); i++)
		    out->text[i] = *cptr++;
		out->hdr.t_cnt = buf->number * sizeof (ushort);
	    } else	/* Doesn't return anything */
		out->hdr.t_cnt = 0;
	    out->hdr.res = SUCCESS;
	}
	else
	    out->hdr.res = errno;
        out->hdr.fdsc = rc;
}

#endif /* NOIPC */
