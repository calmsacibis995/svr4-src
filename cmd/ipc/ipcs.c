/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ipc:/ipcs.c	1.12.7.3"

/*
 * ipcs - IPC status
 *
 * Examine and print certain things about
 * message queues, semaphores and shared memory.
 *
 * As of SVR4, IPC information is obtained via msgctl, semctl and shmctl
 * to the extent possible.  /dev/kmem is used only to obtain configuration
 * information and to determine the IPC identifiers present in the system.
 * This change ensures that the information in each msgid_ds, semid_ds or
 * shmid_ds data structure that we obtain is complete and consistent.
 * For example, the shm_nattch field of a shmid_ds data structure is
 * only guaranteed to be meaningful when obtained via shmctl; when read
 * directly from /dev/kmem, it may contain garbage.
 * If the user supplies an alternate corefile (using -C), no attempt is
 * made to obtain information using msgctl/semctl/shmctl.
 */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/vnode.h> 
#include <sys/param.h>
#include <sys/var.h>
#include <sys/fs/xnamnode.h> 
#include <sys/sd.h>
#include <a.out.h>
#include <fcntl.h>
#include <time.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define		SYS5	0
#define		SYS3	1

#define	TIME	0
#define	MSG	1
#define	SEM	2
#define	SHM	3
#define	MSGINFO	4
#define	SEMINFO	5
#define	SHMINFO	6
#if u3b2 || u3b || u3b15 || i386
#define XNAMNODE 7
#define VAR	8
#define SDTAB	9
#endif

/*
 * Given an index into an IPC table (message table, shared memory
 * table or semaphore table) determine the corresponding IPC
 * identifier (msgid, shmid or semid).  This requires knowledge of
 * the table size, the corresponding ipc_perm structure (for the
 * sequence number contained therein) and the undocumented method
 * by which the kernel assigns new identifiers.
 */
#define IPC_ID(tblsize, index, permp)	((index) + (tblsize)*(permp)->seq)

struct nlist nl[] = {		/* name list entries for IPC facilities */
#if u3b || u3b15 || u3b2 || i386
	{"time", 0, 0, 0, C_EXT, 0},
	{"msgque", 0, 0, 0, C_EXT, 0},
	{"sema", 0, 0, 0, C_EXT, 0},
	{"shmem", 0, 0, 0, C_EXT, 0},
	{"msginfo", 0, 0, 0, C_EXT, 0},
	{"seminfo", 0, 0, 0, C_EXT, 0},
	{"shminfo", 0, 0, 0, C_EXT, 0},
	{"xnamtable", 0, 0, 0, C_EXT, 0},
	{"v", 0, 0, 0, C_EXT, 0},
	{"sdtab", 0, 0, 0, C_EXT, 0},
#else
#ifdef vax
	{"_time", 0, 0, 0, C_EXT, 0},
	{"_msgque", 0, 0, 0, C_EXT, 0},
	{"_sema", 0, 0, 0, C_EXT, 0},
	{"_shmem", 0, 0, 0, C_EXT, 0},
	{"_msginfo", 0, 0, 0, C_EXT, 0},
	{"_seminfo", 0, 0, 0, C_EXT, 0},
	{"_shminfo", 0, 0, 0, C_EXT, 0},
#else
	{"_time", C_EXT, 0},
	{"_msgque", C_EXT, 0},
	{"_sema", C_EXT, 0},
	{"_shmem", C_EXT, 0},
	{"_msginfo", C_EXT, 0},
	{"_seminfo", C_EXT, 0},
	{"_shminfo", C_EXT, 0},
#endif
#endif
	{NULL}
};
char	chdr[] = "T     ID     KEY        MODE       OWNER    GROUP",
				/* common header format */
	chdr2[] = "  CREATOR   CGROUP",
				/* c option header format */
	*name = "/stand/unix",	/* name list file */
	*mem = "/dev/kmem",	/* memory file */
	opts[] = "abcmopqstC:N:X";/* allowable options for getopt */
extern char	*optarg;	/* arg pointer for getopt */
int		bflg,		/* biggest size:
					segsz on m; qbytes on q; nsems on s */
		cflg,		/* creator's login and group names */
		mflg,		/* shared memory status */
		oflg,		/* outstanding data:
					nattch on m; cbytes, qnum on q */
		pflg,		/* process id's: lrpid, lspid on q;
					cpid, lpid on m */
		qflg,		/* message queue status */
		sflg,		/* semaphore status */
		tflg,		/* times: atime, ctime, dtime on m;
					ctime, rtime, stime on q;
					ctime, otime on s */
		Cflg,		/* user supplied corefile */
		Nflg,		/* user supplied namelist */
		Xflg,		/* print XENIX IPC also */

		err;		/* option error count */
extern int	optind;		/* option index for getopt */

main(argc, argv)
int	argc;	/* arg count */
char	**argv;	/* arg vector */
{
	register int	i,	/* loop control */
			md,	/* memory file file descriptor */
			o,	/* option flag */
			n,	/* table size */
			id;	/* IPC identifier */
	time_t		time;	/* date in memory file */
	struct shmid_ds	mds;	/* shared memory data structure */
	struct shminfo shminfo;	/* shared memory information structure */
	struct msqid_ds	qds;	/* message queue data structure */
	struct msginfo msginfo;	/* message information structure */
	struct semid_ds	sds;	/* semaphore data structure */
	struct seminfo seminfo;	/* semaphore information structure */
	struct var v;		/* tunable parameters for kernel */
	struct xnamnode xnamnode;	/* in-core node for IFNAM files */
	struct xnamnode *pxnamnode; /* in-core node pointer for IFNAM files */
	/* head of table of in-core nodes for IFNAM files */
	struct xnamnode *xnamtable[XNAMTABLESIZE];
	struct ipc_perm ipcperm;	/* simulated permissions for XENIX */
	struct xsd sd;		/* XENIX shared data structure */
	struct xsd *psd;	/* pointer to XENIX shared data structure */
	void hp();
	void tp();
	void reade();
	void lseeke();

	/* Go through the options and set flags. */
	while ((o = getopt(argc, argv, opts)) != EOF)
		switch (o) {
		case 'a':
			bflg = cflg = oflg = pflg = tflg = 1;
			break;
		case 'b':
			bflg = 1;
			break;
		case 'c':
			cflg = 1;
			break;
		case 'C':
			mem = optarg;
			Cflg = 1;
			break;
		case 'm':
			mflg = 1;
			break;
		case 'N':
			name = optarg;
			Nflg = 1;
			break;
		case 'o':
			oflg = 1;
			break;
		case 'p':
			pflg = 1;
			break;
		case 'q':
			qflg = 1;
			break;
		case 's':
			sflg = 1;
			break;
		case 't':
			tflg = 1;
			break;
		case 'X':
      			Xflg = 1;
			break;
		case '?':
			err++;
			break;
		}
	if (err || (optind < argc)) {
		fprintf(stderr,
			"usage:  ipcs [-abcmopqstX] [-C corefile] [-N namelist]\n");
		exit(1);
	}

	/*
	 * If the user supplied either the corefile or namelist then
	 * reset the uid/gid to the user invoking this command.
	 */
	if (Cflg || Nflg) {
		setuid(getuid());
		setgid(getgid());
	}

	if ((mflg + qflg + sflg) == 0)
		mflg = qflg = sflg = 1;

	/* Check out namelist and memory files. */
	nlist(name, nl);
	if (nl[TIME].n_value == 0) {
		fprintf(stderr, "ipcs:  no namelist\n");
		exit(1);
	}
	if ((md = open(mem, O_RDONLY)) < 0) {
		fprintf(stderr, "ipcs:  no memory file\n");
		exit(1);
	}
	lseeke(md, (long)nl[TIME].n_value, 0);
	reade(md, &time, sizeof(time));
	printf("IPC status from %s as of %s", mem, ctime(&time));

        if (Xflg) {
                if(nl[XNAMNODE].n_value) {
			lseeke(md, nl[XNAMNODE].n_value, 0);
			reade(md, xnamtable, sizeof(xnamtable));
		}
		lseeke(md, nl[VAR].n_value, 0);
		reade(md, &v, sizeof(v));
	}

	/* Print Message Queue status report. */
	if (qflg) {
		if (nl[MSG].n_value) {
			lseeke(md, (long)nl[MSGINFO].n_value, 0);
			reade(md, &msginfo, sizeof(msginfo));
			lseeke(md, (long)nl[MSG].n_value, 0);
			printf("%s%s%s%s%s%s\nMessage Queues:\n", chdr,
				cflg ? chdr2 : "",
				oflg ? " CBYTES  QNUM" : "",
				bflg ? " QBYTES" : "",
				pflg ? " LSPID LRPID" : "",
				tflg ? "   STIME    RTIME    CTIME " : "");
			n = msginfo.msgmni;
		} else {
			printf("Message Queue facility not in system.\n");
			n = 0;
		}
		for (i = 0; i < n; i++) {
			reade(md, &qds, sizeof(qds));
			if ((qds.msg_perm.mode & IPC_ALLOC) == 0)
				continue;
			id = IPC_ID(n, i, &qds.msg_perm);
			if (!Cflg && msgctl(id, IPC_STAT, &qds) < 0)
				continue;
			hp('q', "SRrw-rw-rw-", &qds.msg_perm, id,SYS5,SYS5);
			if (oflg)
				printf("%7u%6u", qds.msg_cbytes, qds.msg_qnum);
			if (bflg)
				printf("%7u", qds.msg_qbytes);
			if (pflg)
				printf("%6u%6u", qds.msg_lspid, qds.msg_lrpid);
			if (tflg) {
				tp(qds.msg_stime);
				tp(qds.msg_rtime);
				tp(qds.msg_ctime);
			}
			printf("\n");
		}
	}

	/* Print Shared Memory status report. */
	if (mflg) {
		if (nl[SHM].n_value) {
			lseeke(md, (long)nl[SHMINFO].n_value, 0);
			reade(md, &shminfo, sizeof(shminfo));
			lseeke(md, (long)nl[SHM].n_value, 0);
			if (oflg || bflg || tflg || !qflg || !nl[MSG].n_value)
				printf("%s%s%s%s%s%s\n", chdr,
					cflg ? chdr2 : "",
					oflg ? " NATTCH" : "",
					bflg ? "  SEGSZ" : "",
					pflg ? "  CPID  LPID" : "",
					tflg ? "   ATIME    DTIME    CTIME " : "");
			printf("Shared Memory:\n");
			n = shminfo.shmmni;
		} else {
			printf("Shared Memory facility not in system.\n");
			n = 0;
		}
		for (i = 0; i < n; i++) {
			reade(md, &mds, sizeof(mds));
			if ((mds.shm_perm.mode & IPC_ALLOC) == 0)
				continue;
			id = IPC_ID(n, i, &mds.shm_perm);
			if (!Cflg && shmctl(id, IPC_STAT, &mds) < 0)
				continue;
			hp('m', "DCrw-rw-rw-", &mds.shm_perm, id,SYS5,SYS5);
			if (oflg)
				printf("%7u", mds.shm_nattch);
			if (bflg)
				printf("%7d", mds.shm_segsz);
			if (pflg)
				printf("%6u%6u", mds.shm_cpid, mds.shm_lpid);
			if (tflg) {
				tp(mds.shm_atime);
				tp(mds.shm_dtime);
				tp(mds.shm_ctime);
			}
			printf("\n");
		}
		if (Xflg) {
			/* handle XENIX system 3 shared data */
			if ((pxnamnode = xnamtable[1]) != 0) {
                                if((oflg || bflg || tflg || !qflg ||
                                        !nl[MSG].n_value) && !nl[SHM].n_value)
                                        printf("%s%s%s%s\n", chdr,
                                                cflg ? chdr2 : "",
                                                oflg ? " NATTCH" : "",
                                                bflg ? "   SEGSZ" : "");
                                printf("XENIX Shared Memory (3.0):\n");
			} else if (!nl[XNAMNODE].n_value){
                                printf("XENIX Shared Memory (3.0) facility not\
in system.\n");
			}
			while (pxnamnode)
			{
                                lseeke(md, pxnamnode, 0);
                                reade(md, &xnamnode, sizeof(xnamnode));
                                ipcperm.cuid = ipcperm.uid = xnamnode.x_uid;
                                ipcperm.cgid = ipcperm.gid = xnamnode.x_gid;
                                ipcperm.mode = xnamnode.x_mode;
                                psd = (struct xsd *) xnamnode.x_un.xsd;
                                lseeke(md, psd, 0);
                                reade(md, &sd, sizeof(sd));
                                if (sd.x_flags & SDI_CLEAR)
                                        ipcperm.mode |= 01000;
                                hp('m', "DCrw-rw-rw-", &ipcperm, 0, v.v_xsdsegs
* v.v_xsdslots, SYS3);
                                if(oflg)
                                        printf("%7u", xnamnode.x_vnode.v_count);
                                if(bflg)
                                        printf("%8u", sd.x_len + 1);
                                printf("\n");
                                pxnamnode = xnamnode.x_next;
			}
		}

	}

	/* Print Semaphore facility status. */
	if (sflg) {
		if (nl[SEM].n_value) {
			lseeke(md, (long)nl[SEMINFO].n_value, 0);
			reade(md, &seminfo, sizeof(seminfo));
			lseeke(md, (long)nl[SEM].n_value, 0);
			if (bflg || tflg || (!qflg || !nl[MSG].n_value) &&
				(!mflg || !nl[SHM].n_value))
				printf("%s%s%s%s\n", chdr,
					cflg ? chdr2 : "",
					bflg ? " NSEMS" : "",
					tflg ? "   OTIME    CTIME " : "");
			printf("Semaphores:\n");
			n = seminfo.semmni;
		} else {
			printf("Semaphore facility not in system.\n");
			n = 0;
		}
		for (i = 0; i < n; i++) {
			reade(md, &sds, sizeof(sds));
			if ((sds.sem_perm.mode & IPC_ALLOC) == 0)
				continue;
			id = IPC_ID(n, i, &sds.sem_perm);
			if (!Cflg && semctl(id, 0, IPC_STAT, &sds) < 0)
				continue;
			hp('s', "--ra-ra-ra-", &sds.sem_perm, id, SYS5, SYS5);
			if (bflg)
				printf("%6u", sds.sem_nsems);
			if (tflg) {
				tp(sds.sem_otime);
				tp(sds.sem_ctime);
			}
			printf("\n");
		}
                if(Xflg) {
                        if ((pxnamnode = xnamtable[0]) != 0) {
                        /* handle system 3 semaphores */
                                if((bflg || tflg || (!qflg || !nl[MSG].n_value)
                                        && (!mflg || (!nl[SHM].n_value &&
                                        !nl[SDTAB].n_value))) &&
                                        !nl[SEM].n_value)
                                                printf("%s%s%s\n", chdr,
                                                        cflg ? chdr2 : "",
                                                        bflg ? " NSEMS" : "");
                                printf("XENIX Semaphores (3.0):\n");
                        } else if (!nl[XNAMNODE].n_value)
                                printf("XENIX Semaphore (3.0) facility  not \
in system.\n");
			while (pxnamnode)
			{
                                lseeke(md, pxnamnode, 0);
                                reade(md, &xnamnode, sizeof(xnamnode));
                                ipcperm.cuid = ipcperm.uid = xnamnode.x_uid;
                                ipcperm.cgid = ipcperm.gid = xnamnode.x_gid;
                                ipcperm.mode = xnamnode.x_mode;
                                ipcperm.key = 0;
                                hp('s',"--ra-ra-ra-",&ipcperm,0,0, SYS3);
                                if (bflg)
                                        printf("%6u", 1);
                                printf("\n");
                                pxnamnode = xnamnode.x_next;
			}
		}
	}
	exit(0);
	/*NOTREACHED*/
}

/*
**	lseeke - lseek with error exit
*/

void
lseeke(f, o, w)
int	f,	/* fd */
	w;	/* whence */
long	o;	/* offset */
{
#ifdef vax
	o &= 0x3fffffff;
#endif

	if (lseek(f, o, w) == -1) {
		perror("ipcs:  seek error");
		exit(1);
	}
}

/*
**	reade - read with error exit
*/

void
reade(f, b, s)
int	f;	/* fd */
size_t	s;	/* size */
void	*b;	/* buffer address */
{
	if (read(f, (char *)b, s) != s) {
		perror("ipcs:  read error");
		exit(1);
	}
}

/*
**	hp - common header print
*/

void
hp(type, modesp, permp, slot, slots, sys3)
char				type,	/* facility type */
				*modesp;/* ptr to mode replacement characters */
register struct ipc_perm	*permp;	/* ptr to permission structure */
int				slot,	/* facility slot number */
                                slots;	/* # of facility slots */
int				sys3;	/* system 5 vs. system 3 */
{
	register int		i;	/* loop control */
	register struct group	*g;	/* ptr to group group entry */
	register struct passwd	*u;	/* ptr to user passwd entry */

        if (sys3){
		printf("%c%s%s", type, "    x	  ", "xenix    ");
	}
	else {
		printf("%c%7d%s%#8.8x ", type, slot,
			permp->key ? " " : " 0x", permp->key);
	}
	for (i = 02000; i; modesp++, i >>= 1)
		printf("%c", ((int)permp->mode & i) ? *modesp : '-');
	if ((u = getpwuid(permp->uid)) == NULL)
		printf("%9d", permp->uid);
	else
		printf("%9.8s", u->pw_name);
	if ((g = getgrgid(permp->gid)) == NULL)
		printf("%9d", permp->gid);
	else
		printf("%9.8s", g->gr_name);
	if (cflg) {
		if ((u = getpwuid(permp->cuid)) == NULL)
			printf("%9d", permp->cuid);
		else
			printf("%9.8s", u->pw_name);
		if ((g = getgrgid(permp->cgid)) == NULL)
			printf("%9d", permp->cgid);
		else
			printf("%9.8s", g->gr_name);
	}
}

/*
**	tp - time entry printer
*/

void
tp(time)
time_t	time;	/* time to be displayed */
{
	register struct tm *t;	/* ptr to converted time */

	if (time) {
		t = localtime(&time);
		printf(" %2d:%2.2d:%2.2d", t->tm_hour, t->tm_min, t->tm_sec);
	} else
		printf(" no-entry");
}
