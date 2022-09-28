/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)fmli:sys/coproc.c	1.18"

#include	<ctype.h>
#include	<stdio.h>
#include	<fcntl.h>
#include	<string.h>
#include	<signal.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include 	<errno.h>	/* EFT abs k16 */
#include	"wish.h"
#include	"terror.h"
#include	"var_arrays.h"
#include	"eval.h"
#include	"moremacros.h"
#include	"sizes.h"

typedef struct {
	char *id;
	char *w_path;
	char *r_path;
	FILE *writefp;
	char *expect;
	char *send;
	char **refs;
} COREC;

#define FREE(STR) if (STR) free(STR)

static COREC *Cotable = NULL;
static int Cur_corec = -1;
static FILE *Readfp, *Writefp;
int Coproc_active = 0;
extern char Semaphore[];	/* path of FIFO for process synchronization */
extern pid_t Fmli_pid;		/* process ID of FMLI. EFT k16 abs */

int
cosend(argc, argv, instr, outstr, errstr)
int argc;
char **argv;
IOSTRUCT *instr;
IOSTRUCT *outstr;
IOSTRUCT *errstr;
{
	register int i, blocking;
	void (*osig)();
	char *exp;
	char buf[BUFSIZ];
	char **args;

	args = argv;
	if (strcmp(args[1], "-n") == 0) {	/* if no-blocking mode */
		args++;
		blocking = FALSE;
	}
	else
		blocking = TRUE;
	if ((i = findtab(args[1])) == -1)	/* look-up coproc in proc tab */
		return(NULL);
	make_cur(i);				/* make the coproc current */
	osig = sigset(SIGPIPE, SIG_IGN);	/* ignore interrupts */
	for (i = 2; (args[i]) && (i < argc); i++) {
		if (i > 2)
			putc(' ', Writefp);
		fputs(args[i], Writefp);
	}
	putc('\n', Writefp);
	if (Cotable[Cur_corec].send) {
		fputs(Cotable[Cur_corec].send, Writefp);
		putc('\n', Writefp);
	}
	fflush(Writefp);
	(void) sigset(SIGPIPE, osig); /* was signal()   abs */
	if (blocking) {		/* if blocking on a response from coproc */
		if (!(exp = Cotable[Cur_corec].expect)) {
			fgets(buf, BUFSIZ, Readfp);
			putastr(buf, outstr);
		}
		else {
			while (fgets(buf, BUFSIZ, Readfp)) {
				if (strncmp(buf, exp, strlen(exp)) == 0)
					break;
				putastr(buf, outstr);
			}
		}
	}
	return(SUCCESS);
}

findtab(str)
char *str;
{
	int i;
	int	lcv;

	lcv = array_len(Cotable);
	for (i = 0; i < lcv; i++)
		if (strcmp(Cotable[i].id, str) == 0)
			return(i);
	return(-1);
}

make_cur(i)
int i;
{
	if (Cur_corec == i)
		return;
	if (Cur_corec != -1)
		fclose(Readfp);
	Cur_corec = i;
	if (i == -1)
		return;
	if (!(Cotable[Cur_corec].writefp) && ((Cotable[Cur_corec].writefp = fopen(Cotable[Cur_corec].w_path, "w")) == NULL))
		warn(NOPEN, "the write pipe");
	Writefp = Cotable[Cur_corec].writefp;
	if ((Readfp = fopen(Cotable[Cur_corec].r_path, "r")) == NULL)
		warn(NOPEN, "the read pipe");
	return;
}

int
cocreate(argc, args, instr, outstr, errstr)
int argc;
char **args;
IOSTRUCT *instr;
IOSTRUCT *outstr;
IOSTRUCT *errstr;
{
	COREC tmp;
	int i, c;
	int gobble = FALSE;
	char *ref;
	extern char *optarg, *filename();
	extern int optind;

	if (!Cotable)
		Cotable = (COREC *) array_create(sizeof(COREC), 1);
	ref = tmp.id = tmp.r_path = tmp.w_path = tmp.expect = tmp.send = NULL;
	tmp.refs = NULL;
	tmp.writefp = NULL;
	optind = 1;
	while ((c = getopt(argc, args, "R:i:ge:w:r:s:")) != EOF)
		switch (c) {
		case 'R':
			ref = optarg;
			break;
		case 'i':
			tmp.id = strsave(optarg);
			break;
		case 'g':
			gobble = TRUE;
			break;
		case 'e':
			tmp.expect = strsave(optarg);
			break;
		case 'w':
			tmp.w_path = strsave(optarg);
			break;
		case 'r':
			tmp.r_path = strsave(optarg);
			break;
		case 's':
			tmp.send = strsave(optarg);
			break;
		default:
			break;
		}
	if (!tmp.id)
		tmp.id = strsave(filename(args[optind]));
	if ((i = findtab(tmp.id)) != -1) {
		make_cur(i);
		if (ref && Cotable[i].refs) {
			register int j;
			int	lcv;

			lcv = array_len(Cotable[i].refs);
			for (j = 0; (j < lcv) && (strcmp(Cotable[i].refs[j], ref)); j++)
				;
			if (j >= lcv) {
				ref = strsave(ref);
				Cotable[i].refs = (char **) array_append(Cotable[i].refs, &ref);
			}
		}
		return(SUCCESS);
	}
	else {
		if (Coproc_active == 0)		/* set up fifo for Semaphore */
			mknod(Semaphore, 010666, 0);
		Coproc_active++;
	}
	if (ref) {
		tmp.refs = (char **) array_create(sizeof(char *), 1);
		ref = strsave(ref);
		tmp.refs = (char **) array_append(tmp.refs, &ref);
	}
	if (!tmp.r_path) {
		char path[PATHSIZ];

		sprintf(path, "/tmp/r%s.%ld", tmp.id, Fmli_pid);
		tmp.r_path = strsave(path);
	}
	if (!tmp.w_path) {
		char path[PATHSIZ];

		sprintf(path, "/tmp/w%s.%ld", tmp.id, Fmli_pid);
		tmp.w_path = strsave(path);
	}
	if (mknod(tmp.r_path, 010666, 0) == -1)
#ifdef _DEBUG4
		_debug4(stderr, "Could not do mknod");
#endif
		;
	if (mknod(tmp.w_path, 010666, 0) == -1)
#ifdef _DEBUG4
		_debug4(stderr, "Could not do mknod");
#endif
		;
	switch(fork()) {
	case -1:
		fatal(NOFORK, NULL);
	case 0: {
		int fd;

#ifdef _DEBUG
		_debug(stderr, "in child\n");
#endif
		if ((fd = open(tmp.w_path, O_RDONLY|O_NDELAY)) == -1) 
			child_fatal(NOPEN, nil);
		fcntl(fd, F_SETFL, 0);
		close(0);
		dup(fd);
		close(fd);
		if ((fd = open(tmp.r_path, O_WRONLY)) == -1) 
			child_fatal(NOPEN, nil);
		close(1);
		dup(fd);
		close(fd);
		freopen("/dev/null", "w", stderr);
		execvp(args[optind], args + optind);
#ifdef _DEBUG
		_debug(stderr, "couldnt exec\n");
#endif
		child_fatal(NOFORK, NULL);
		}
	default:
		break;
	}
	Cotable = (COREC *) array_append(Cotable, &tmp);
	make_cur(array_len(Cotable) - 1);
	if (gobble) {
		char buf[BUFSIZ];

		fgets(buf, BUFSIZ, Readfp);
	}
	return(SUCCESS);
}

int
codestroy(argc, args, instr, outstr, errstr)
int argc;
char **args;
IOSTRUCT *instr;
IOSTRUCT *outstr;
IOSTRUCT *errstr;
{
	int i;
	int hold;
	char *ref;
	int	lcv;

	ref = NULL;
	if (strcmp(args[1], "-R") == 0)
		ref = args[2];
/*	if ((i = findtab(args[ref ? 3 : 1])) == -1) amdahl compatibility.. */
	if (ref)
	    i = findtab(args[3]);
	else
	    i = findtab(args[1]);
	if ( i == -1)
		return(FAIL);
	if (Cotable[i].refs && ref) {
		register int j;

		lcv = array_len(Cotable[i].refs);
		for (j = 0; (j < lcv) && (strcmp(Cotable[i].refs[j], ref)); j++)
			;
		if (j < lcv)
			array_delete(Cotable[i].refs, j);
	}
	if ((int)array_len(Cotable[i].refs) > 0)
		return(SUCCESS);
	hold = cosend(ref ? argc - 2 : argc, ref ? args + 2 : args, instr, outstr);
	make_cur(-1);
	if (Cotable[i].writefp)
		fclose(Cotable[i].writefp);
	unlink(Cotable[i].w_path);
	unlink(Cotable[i].r_path);
	FREE(Cotable[i].expect);
	FREE(Cotable[i].w_path);
	FREE(Cotable[i].r_path);
	FREE(Cotable[i].send);
	FREE(Cotable[i].id);
	array_delete(Cotable, i);
	if (--Coproc_active == 0)
		unlink(Semaphore);	/* remove fifo */
	return(hold);
}

int
cocheck(argc, argv, instr, outstr, errstr)
int argc;
char **argv;
IOSTRUCT *instr;
IOSTRUCT *outstr;
IOSTRUCT *errstr;
{
	register int i;
	struct stat sbuf;

	if ((i = findtab(argv[1])) == -1)  	/* look-up coproc in proc tab */
		return(FAIL);
	make_cur(i);				/* make the coproc current */
	if (fstat(fileno(Readfp), &sbuf) < 0)
		return(FAIL);			/* can't stat file */
	if (sbuf.st_size > 0)
		return(SUCCESS);
	else
		return(FAIL);
}

int
coreceive(argc, argv, instr, outstr, errstr)
int argc;
char **argv;
IOSTRUCT *instr;
IOSTRUCT *outstr;
IOSTRUCT *errstr;
{
	register int i;
	register char *exp;
	struct stat sbuf;
	char buf[BUFSIZ];

	if ((i = findtab(argv[1])) == -1)	/* look-up coproc in proc tab */
		return(FAIL);
	make_cur(i);				/* make the coproc current */
	if ((fstat(fileno(Readfp), &sbuf) < 0) || (sbuf.st_size == 0))
		return(FAIL);
	if (!(exp = Cotable[Cur_corec].expect)) {
		fgets(buf, BUFSIZ, Readfp);
		putastr(buf, outstr);
	}
	else {
		while (fgets(buf, BUFSIZ, Readfp)) {
			if (strncmp(buf, exp, strlen(exp)) == 0)
				break;
			putastr(buf, outstr);
		}
	}
	return(SUCCESS);
}
