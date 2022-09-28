/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)dfsck:dfsck.c	1.5"
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MAXFDS	20		/* max open files */
#define MAXBUF	512		/* pipe buffer size */
#define WAITIME 2	/* # of secs. to wait for death of child in loop */

char *use =
	"Usage:  %s [-options] FileSysA [FSA...] [-[options] FSB [FSB...]]\n";
int p1fs1[2], p2fs1[2], p1fs2[2], p2fs2[2];
int fsout1, fsin1, fsout2, fsin2;
int ftendi1,ftendo1,ftendi2,ftendo2;
int ttyio, ttyi, ttyo;
void sigcatch();
void sigalrm();
unsigned alarm();

main(argc, argv)
char **argv;
{
	extern int sys_nerr, errno;
	extern char *sys_errlist[];

	pid_t pid, pid1, pid2;
	char *save;
	int i, j;
	int status = 0, stat;
	char buf2, c = '0';
	int bd1flg = 0, bd2flg = 0;
	int exit1 = 0, exit2 = 0;
	int one, two, ans = 0, ans1 = 0, ans2 = 0;
	int arg1 = 0, opt2 = 0, arg2 = 0, optsame = 0;


	for (i = 3; i < MAXFDS; i++) close(i);
	if((ttyio = open("/dev/tty",O_RDWR )) == -1) {
		printf("Cannot open tty\n");
		exit(2);
	}

	for ( i = 1; i < argc; i++)
	{	if ( argv[i][0] == '-' )
		{	if ( arg1 && !opt2 ) opt2 = i;
			for ( j = 1; argv[i][j] != '\0'; j++ )
				switch( argv[i][j])
				{	case 'y':
					case 'Y':
					case 'n':
					case 'N':
						opt2? ans2++: ans1++;
						break;
					case 't':
						i++;
						break;
					default:	break;
				}
			if ( (j==1) && opt2 ) optsame++;
		} else if ( !arg1 ) arg1 = i;
		  else if ( opt2 && !arg2) arg2 = i;
	}

	if ( (argc < 2) || !arg1 || (opt2 && !arg2) )
	{	fprintf( stderr, use, argv[0]);
		exit( 1);
	}

	ans = ans1 && (ans2 || optsame);
	if ( !arg2 ) bd2flg++;

	if(!ans && !bd2flg) {		/* identify file systems */
		printf("1 will identify ");
		for ( i = arg1; i < opt2; i++ ) printf( "%s ", argv[i]);
		printf("\n2 will identify ");
		for ( i = arg2; i < argc; i++ ) printf( "%s ", argv[i]);
		printf("\nPrecede every answer with 1 or 2\n");
		printf("as in `1y' for %s\n",argv[arg1]);
	}

			/* pipe set-up */
	if(pipe(p1fs1) == -1)
		printf("Cannot open pipe for first fsck\n");
	if(pipe(p2fs1) == -1)
		printf("Cannot open pipes for first fsck\n");
	ftendi1 = p1fs1[0];
	ftendo1 = p2fs1[1];
	fsout1= p1fs1[1];
	fsin1 = p2fs1[0];

	if(pipe(p1fs2) == -1)
		printf("Cannot open pipe for second fsck\n");
	if(pipe(p2fs2) == -1)
		printf("Cannot open pipes for second fsck\n");
	ftendi2 = p1fs2[0];
	ftendo2 = p2fs2[1];
	fsout2 = p1fs2[1];
	fsin2 = p2fs2[0];

	argv[0] = "fsck";
	if(!bd2flg) {
		argv[0] = "1fsck";
		save = argv[opt2];
		argv[opt2] = 0;
	}

	switch(pid1 = fork()) {
	case 0:			/* child */
		fcntl(fsin1,F_SETFL,fcntl(fsin1,F_GETFL) & ~O_NDELAY);
		fcntl(fsout1,F_SETFL,fcntl(fsout1,F_GETFL) & ~O_NDELAY);
		chgfd(fsin1,0);
		chgfd(fsout1,1);
		close(ftendi1);
		close(ftendo1);
		for(i = 3; i < MAXFDS; i++)	close(i);
		execvp( "/etc/fsck", argv);
		printf("Can't exec 'fsck' on %s\n",argv[arg1]);
		exit(2);
	case (pid_t)-1:		/* unsuccessful fork */
		printf("Couldn't fork on %s\n",argv[arg1]);
		exit(2);
	default:		/* parent */
		close(fsin1);
		close(fsout1);
		break;
	}

	if(!bd2flg) {
		argv[0] = "2fsck";
		argv[opt2] = save;
		if ( optsame )
		{	for ( i = arg2; i < argc; i++ )
				argv[arg1 - arg2 + i] = argv[i];
			argv[arg1 - arg2 + i] = 0;
		} else
		{	for ( i = opt2; i < argc; i++ )
				argv[1 - opt2 + i] = argv[i];
			argv[1 - opt2 + i] = 0;
		}
		switch(pid2 = fork()) {
		case 0:			/* child */
			fcntl(fsin2,F_SETFL,fcntl(fsin2,F_GETFL) & ~O_NDELAY);
			fcntl(fsout2,F_SETFL,fcntl(fsout2,F_GETFL) & ~O_NDELAY);
			chgfd(fsin2,0);
			chgfd(fsout2,1);
			close(ftendi2);
			close(ftendo2);
			for(i = 3; i <MAXFDS; i++)	close(i);
			execvp( "/etc/fsck", argv);
			printf("Can't exec 'fsck' on %s\n",argv[arg2]);
			exit(2);
		case (pid_t)-1:		/* unsuccessful fork */
			printf("Couldn't fork on %s\n",argv[arg2]);
			exit(2);
		default:		/* parent */
			close(fsin2);
			close(fsout2);
			break;
		}
	}
	setdlys();		/* set delays */
	signal(SIGPIPE,sigcatch);
	signal(SIGALRM,sigalrm);
	ans = 0;
	for(;;) {
		alarm(WAITIME);
		if((pid = wait(&status)) != (pid_t)-1) {
			alarm(0);
			if(pid == pid1 || pid == pid2) {
				if(stat = status & 0377) {  /* signal detected */
					if(status & 0200)
						printf("core dumped\n");
					printf("signal %o caught\n",stat);
					pid == pid1? exit1++: exit2++;
				}
				if(((status >> 8) & 0377) >= 0) {	/* child exit */
					status = 0;
					pid == pid1? exit1++: exit2++;
				}
			}
			else {
			   printf("Unknown child fdes\n");
			   exit(2);
			}
		}
		if(pid == (pid_t)-1 && errno != EINTR) {
		   if(!bd2flg && !exit2 && errno != ECHILD) {
			if(errno <= sys_nerr) {
				for(i=0; sys_errlist[errno][i] != '\0'; i++);
				write(ttyio, sys_errlist[errno], i);
				write(ttyio, "\n", 1);
			}
			else
			   perror("dfsck");
			exit(2);
		   }
		}
	
		pread(ftendi1);
		pread(ftendi2);

#ifdef RT		/* Correct for RT TTY package not seeing O_NDELAY */
		alarm ( WAITIME );
		while ((read(ttyi,&buf2,1)) > 0 ) {
			alarm ( 0 );
#else
		while(read(ttyi,&buf2,1)) {
#endif
			if(buf2 =='1' && !one) {
				one++;
				continue;
			}
			if(buf2 == '2' && !two) {
				two++;
				continue;
			}
			if(bd2flg && !one)
				one++;
			if((!bd2flg && !two) && (!bd1flg && !one) && !ans) {
				write(ttyio,"which filesystem? answer '1' or '2'\n",36);
				c = buf2;
				ans++;
			}
			if(one) {
				if(buf2 == '\n' && (c == 'y' || c == 'n')) {
					write(ftendo1,&c,1);
					c = '0';
				}
				write(ftendo1,&buf2,1);
			}
			if(two) {
				if(buf2 == '\n' && (c == 'y' || c == 'n')) {
					write(ftendo2,&c,1);
					c = '0';
				}
				write(ftendo2,&buf2,1);
			}
			if(buf2 == '\n') {
				if (one)   one = 0;
				if (two)   two = 0;
				ans = 0;
				break;
			}
		}
		if(exit1)
			pread(ftendi1);
		if(exit2)
			pread(ftendi2);
		if((exit1 && exit2) || (bd2flg && exit1)) {
			printf(">>> DFSCK DONE <<<\n");
			break;
		}
	}
}

pread(fd)
int fd;
{
	unsigned sz;
	struct stat statb;
	char buf1[MAXBUF];

	fstat(fd,&statb);
	if((sz = statb.st_size) > 0) {
		statb.st_size = 0;
		while(sz >= MAXBUF) {
			read(fd,buf1,MAXBUF);
			write(ttyo,buf1,MAXBUF);
			sz -= MAXBUF;
		}
		if(sz) {
			read(fd,buf1,sz);
			write(ttyo,buf1,sz);
		}
	}
}

chgfd(ofd,nfd)
int ofd,nfd;
{
	if(nfd == ofd) return;
	if(fcntl(nfd,F_GETFD,0) != -1) close(nfd);
	if(fcntl(ofd,F_DUPFD,nfd) == -1) {
		printf("Cannot change file descriptor\n");
		exit(2);
	}
	close(ofd);
	return;
}

void
sigcatch(sig)
{
	signal(SIGPIPE,SIG_IGN);
	write(ttyio,"pipe error: write to wrong pipe(fsck)\n",38);
	signal(SIGPIPE,sigcatch);
	return;
}

void
sigalrm()
{
	signal(SIGALRM,sigalrm);
}

setdlys()
{
	close(0);
	if((ttyi = fcntl(ttyio,F_DUPFD,0)) == -1) {
		printf("Cannot dup tty fdes for ttyi\n");
		exit(2);
	}
	close(1);
	if((ttyo = fcntl(ttyio,F_DUPFD,1)) == -1) {
		printf("Cannot dup tty fdes\n");
		exit(2);
	}
	if((fcntl(ttyi,F_SETFL,O_NDELAY)) == -1) {
		printf("Couldn't set O_NDELAY on ttyi\n");
		exit(2);
	}
	fcntl(ftendo1,F_SETFL,fcntl(ftendo1,F_GETFL) & ~O_NDELAY);
	fcntl(ftendi1,F_SETFL,fcntl(ftendi1,F_GETFL) | O_NDELAY);
	fcntl(ftendo2,F_SETFL,fcntl(ftendo2,F_GETFL) & ~O_NDELAY);
	fcntl(ftendi2,F_SETFL,fcntl(ftendi2,F_GETFL) | O_NDELAY);
}
