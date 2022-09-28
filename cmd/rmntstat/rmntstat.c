/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)rmntstat:rmntstat.c	1.4.10.1"
#include <stdio.h>
#include <sys/fcntl.h> 
#include <sys/types.h>
#include <sys/nserve.h>
#include <sys/stat.h>
#include <sys/rf_sys.h>

extern char *malloc();

main(argc, argv)
int    argc;
char **argv;
{
	extern void read_advtab();
	extern void pr_info();
	extern void exit();
	extern void perror();
	int i;
	int num_args = argc;
	int maxclients, numclients;	
	struct client *clientp;	/* buffer for kernel client info */
	int maxadvs, numadvs;
	char *k_advp;		/* buffer for kernel resource names */
	char *advtabp;		/* buffer for info from /etc/dfs/sharetab */
	int headerflag = 1;	/* reset if -h option is given, or after the 
				 * header is printed 
				 */

	if (geteuid() != 0) {
		fprintf(stderr, "%s: must be super-user\n", argv[0]);
		exit(1);
	}
	if (argc != 1 && strcmp(argv[1], "-h") == 0) {
		headerflag = 0;
		num_args--;
	}
	if (num_args > 2) {
		fprintf(stderr, "%s: usage: %s [-h] [resource]\n", argv[0],
		    argv[0]);
		exit(1);
	}

	if ((maxclients = rfsys(RF_TUNEABLE, T_NSRMOUNT)) <= 0) {
		perror(argv[0]);
		exit(1);
	}

	if ((clientp = (struct client *)malloc(maxclients *
	    (sizeof(struct client)))) == NULL) {
		fprintf(stderr, "%s:  memory allocation failed\n", argv[0]);
		exit(1);
	}

	/* read /etc/dfs/sharetab into advtabp, setting advtabp to NULL if error */
	read_advtab(&advtabp);

	/* the following handles a request about a particular resource */
	if (num_args == 2) {
		if ((numclients = rfsys(RF_CLIENTS, argv[argc-1],
		    clientp)) < 0) {
			perror(argv[0]);
			exit(1);
		}
		pr_info(argv[argc-1], clientp, numclients, headerflag, advtabp);
		exit(0);
	}

	/* the following prints info about all resources */
	if ((maxadvs = rfsys(RF_TUNEABLE, T_NADVERTISE)) < 0) {
		perror(argv[0]);
		exit(1);
	}
	if ((k_advp = malloc(maxadvs * RFS_NMSZ)) == NULL) {
		fprintf(stderr, "%s:  memory allocation failed\n", argv[0]);
		exit(1);
	}
	if ((numadvs = rfsys(RF_RESOURCES, k_advp)) < 0) {
		perror(argv[0]);
		exit(1);
	}

	for (i = 0; i < numadvs; i++) {
		if ((numclients = rfsys(RF_CLIENTS, k_advp, clientp)) < 0) {
			perror(argv[0]);
			exit(1);
		}
		pr_info(k_advp, clientp, numclients, headerflag, advtabp);
		if (headerflag)
			headerflag = 0;
		k_advp += RFS_NMSZ;
	}
	exit(0);
}

void
pr_info(res, clientp, numclients, headerflag, advtabp)
	char res[RFS_NMSZ];
	struct client *clientp;
	int numclients;
	int headerflag;
	char *advtabp;
{
	extern char *pathname();
	int i;
	char path[32];

	if (headerflag) 
		printf("RESOURCE       PATH                             HOSTNAMES\n");

	printf("%-14s", res);
	strcpy(path, pathname(res,advtabp));
	printf(" %-32s", path);

	if (numclients == 0) {
		printf("\n");
		return;
	}
	printf(" %s", clientp[0].cl_node);
	for (i = 1; i < numclients; i++)
		printf(",%s", clientp[i].cl_node);
	printf("\n");
}

void
read_advtab(advtabpp)
	char **advtabpp;
{
	int fd;
	struct stat sbuf;

	if ((stat("/etc/dfs/sharetab", &sbuf) == -1)
	|| ((*advtabpp = malloc(sbuf.st_size + 1)) == NULL)
	|| ((fd = open("/etc/dfs/sharetab", O_RDONLY)) == -1)
	|| (read(fd, *advtabpp, sbuf.st_size) != sbuf.st_size)) {
		printf("rmntstat: warning: cannot get information from /etc/dfs/sharetab; pathnames will not be printed\n");
		*advtabpp = NULL;
	} else {
		*(*advtabpp + sbuf.st_size) = '\0';
	}
}

char *
pathname(res, advtabp)
	char *res;
	char *advtabp;
{
	extern char *gettok();
	char rname[15], path[32], fstype[10], perm[10];

	if (*advtabp == NULL)
		return("unknown");

	strcpy(path ,gettok(advtabp, " \t"));
	strcpy(rname ,gettok(NULL, " \t"));
	strcpy(fstype ,gettok(NULL, " \t"));
	strcpy(perm ,gettok(NULL, " \t"));

	if ((path[0] == NULL)||(rname[0] == NULL)||(fstype[0] == NULL)
	||(perm[0] == NULL))
		return("unknown");

	while (strcmp(fstype, "rfs") || strcmp(rname, res)) {
		if (gettok(NULL, "\n") == NULL) 
			return("unknown");
		else {
		        strcpy (path ,gettok(NULL, " \t"));
		   	strcpy (rname ,gettok(NULL, " \t"));
		   	strcpy (fstype ,gettok(NULL, " \t"));
	    	   	strcpy (perm ,gettok(NULL, " \t"));
         		if ((path[0] == NULL)||(rname[0] == NULL)
			||(fstype[0] == NULL)||(perm[0] == NULL))
				return("unknown");
		}
	}

	return(path);
}



char *
gettok(string, sep)
char  *string;
char  *sep;
{
	register char	*ptr;
	static	 char	*savept;
	static	 char	buf[512];
	char	 t;

	char	*strpbrk();

	ptr = (string == NULL)? savept: string;

	ptr = ptr + strspn(ptr, sep);

	if (*ptr == '\0')
		return(NULL);

	if ((savept = strpbrk(ptr, sep)) == NULL)
		savept = ptr + strlen(ptr);

	t = *savept;
	*savept = '\0';
	strncpy(buf, ptr, 512);
	*savept = t;
	buf[511] = '\0';
	if (*savept != '\0')
		savept ++;
	return(buf);
}
