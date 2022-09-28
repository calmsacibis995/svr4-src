/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)conv-cmd:ttyconv.c	1.5.1.1"

/*
 * ttyconv	- this program will perform the following:
 *		- it will convert /etc/gettydefs to /etc/ttydefs
 *		- it will convert all getty entries in /etc/inittab
 *		  to ttymon run under SAC
 *
 *		- requirements:
 *			- /usr must be mounted
 *			- must be run in single user mode
 */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/mkdev.h>
#include <sys/stat.h>
#include <sac.h>

#define	VERSION		"# VERSION=1\n"

#define	GREP_INITTAB	"grep getty /etc/inittab"
#define CHG_CONSOLE	"sed '/^co:/s/\\/.*$/\\/usr\\/lib\\/saf\\/ttymon -g -p \"%s\" -m ldterm -d \\/dev\\/console -l %s/' /etc/inittab > _tmpinittab"
#define RMGETTY		"sed '/getty/d' /etc/inittab > _tmpinittab"
#define	LISTSVC		"/usr/sbin/pmadm -L -t ttymon 2>/dev/null|cut -d: -f1,3,6"
#define	SVCREMOVE	"/usr/sbin/pmadm -r -p %s -s %s 2>/dev/null"

#define PMTEST		"/usr/sbin/sacadm -L -p %s >/dev/null 2>&1"

#define	PMADD \
"/usr/sbin/sacadm -a -p %s -t ttymon -c /usr/lib/saf/ttymon -n 3 -v `/usr/sbin/ttyadm -V`"

#define	SVCADD	\
"/usr/sbin/pmadm -a -p %s -s %s -i root -v `/usr/sbin/ttyadm -V` %s %s -m \"`/usr/sbin/ttyadm %s -p \\\"%s\\\" -d %s -s /usr/bin/login -l %s`\" 2>/dev/null"

#define	MVTMP		"mv _tmpinittab /etc/inittab"

#define		MAXMAJOR	20
#define		MAXMINOR	10

struct	getty {
	int	g_exist;		/* device already under ttymon 	*/
	major_t	g_major;		/* device major #		*/
	minor_t	g_minor;		/* device minor #		*/

	char	*g_svctag;		/* port/service tag		*/
	char	g_flags[5];		/* flags			*/
	char	*g_options;		/* options			*/
	char	g_odevice[30];		/* device name in inittab	*/
	char	g_device[30];		/* device name to be used	*/
	char	*g_ttylabel;		/* ttylabel			*/
	char	*g_prompt;		/* prompt message		*/
	char	g_comments[BUFSIZ];	/* comments			*/

	char	g_pmtag[15];		/* port monitor tag		*/
};

struct pm_buf {
	char pmtag[15];		/* the pmtag name 	*/
	char svctag[15];	/* the svctag name	*/
	char device[30];	/* device name		*/
};

struct	pm_buf	ports[MAXMAJOR][MAXMINOR];

char	*find_prompt();
char	*nexttok();
static	void	initialize_ports();
static	void	conv_defs();

main()
{
	FILE	*gfp;
	char	buf[BUFSIZ];
	char	cmd[BUFSIZ];
	struct	getty	getty;
	major_t	M;		/* device Major */
	minor_t m;		/* .. and minor */
	int	sys_ret;
	char	*infile = "/etc/gettydefs";
	char	*outfile = "/etc/ttydefs";

	/* convert gettydefs to ttydefs */
	conv_defs(infile, outfile);

	/* initialize ports array */
	initialize_ports();

	/* get all getty entries in inittab */
	if (( gfp = popen(GREP_INITTAB, "r")) == NULL) {
		perror("popen failed");
		exit(1);
	}

	/* convert one entry at a time */
	while (fgets(buf, BUFSIZ-1, gfp) != NULL) {
		buf[strlen(buf)-1] = '\0';
		if (parse(buf, &getty) != 0) {
			continue;
		}
		M = getty.g_major;
		m = getty.g_minor;
		if ((M == 0) && (m == 0)) 	{ /* the console */
			if (strcmp(getty.g_prompt, "login: ") == 0) {
				(void)sprintf(cmd,CHG_CONSOLE, 
					"Console Login: ",
					getty.g_ttylabel);
			}
			else {
				(void)sprintf(cmd,CHG_CONSOLE, 
					getty.g_prompt,
					getty.g_ttylabel);
			}
			system(cmd);
			system(MVTMP);
		}
		else if (getty.g_exist) {
			(void)sprintf(cmd,SVCREMOVE, 
				ports[(int)M][(int)m].pmtag, ports[M][m].svctag);
			system(cmd);
			(void)sprintf(cmd, SVCADD,
				ports[(int)M][(int)m].pmtag,
				ports[(int)M][(int)m].svctag,
				getty.g_flags,
				getty.g_comments,
				getty.g_options,
				getty.g_prompt,
				ports[(int)M][(int)m].device,
				getty.g_ttylabel);
			system(cmd);
		}
		else {
			(void)sprintf(cmd, PMTEST, getty.g_pmtag);
			sys_ret = system(cmd);
			if (( sys_ret>>8) == E_NOEXIST) {
				(void)sprintf(cmd, PMADD, getty.g_pmtag);
				system(cmd);
			}
			(void)sprintf(cmd, SVCADD,
				getty.g_pmtag,
				getty.g_svctag,
				getty.g_flags,
				getty.g_comments,
				getty.g_options,
				getty.g_prompt,
				getty.g_device,
				getty.g_ttylabel);
			system(cmd);
		}
	}
	system(RMGETTY);
	system(MVTMP);
}

/*
 * parse(bp, gp) - parse a line from inittab
 *		 - return 0 if successful, -1 if failed
 */
parse(bp, gp)
char	*bp;
struct	getty *gp;	/* a ptr to getty structure */
{
	char	*p, *q;
	char	cmd[BUFSIZ];
	int	space;
	struct	stat	sbuf;
	extern	int	stat();

	p = bp;

	/* get the id field and it will be used as the service tag */
	p = (char *)nexttok(bp,":");
	gp->g_svctag = p;

	/* skip the next field */
	p = (char *)nexttok(NULL,":");

	p = (char *)nexttok(NULL,":");
	if (strcmp(p,"respawn") == 0) 
		strcpy(gp->g_flags, "-f u");  /* port is enabled  */
	else 
		strcpy(gp->g_flags, "-f xu"); /* port is disabled */
	p = p + strlen(p) + 1;

	/* skip the getty command */
	p = (char *)strtok(p," \t");
	p = p + strlen(p) + 1;

	/* get options to the getty/uugetty */
	for (space=1,q=p; *q; q++) {
		if (isspace(*q)) {
			space = 1;
			continue;
		}
		if (*q == '-') {
			switch(*++q) {
			case 'r':
				/*
				 * -r is only used in uugetty
				 * so we set the -b flag
				 */
				*q = 'b';
				break;
			case 'h':
				break;
			case 't':
				while (isspace(*++q));
				while (!isspace(*++q));
				break;
			default:
				break;
			}
			space = 1;
		}
		else {
			if (space) 
				break;
			space = 0;
		}
	}
	if (p == q)
		gp->g_options = NULL;
	else {
		gp->g_options = p;
		*(q-1) = '\0';
	}

	/* get device name */
	p = (char *)strtok(q," \t");
	if (*p == '/')
		(void)sprintf(gp->g_odevice, "%s",p);
	else
		(void)sprintf(gp->g_odevice, "/dev/%s",p);
	if (strncmp(p, "/dev/", 5) == 0)
		p = p + 5;
	if (strncmp(p, "tty", 3) == 0) {
		p = p + 3;
		(void)sprintf(gp->g_device,"/dev/term/%s",p);
		if (stat(gp->g_odevice, &sbuf) < 0) {  
			/* no device with old name */
			if (stat(gp->g_device, &sbuf) < 0){ 
				/* no device with new name */
				return(-1);
			}
		}
		else {  /* device with old name exists */
			if (stat(gp->g_device, &sbuf) < 0){ 
				/* 
				 * no device with new name 
				 * mv old name to new name
				 */
				if (stat("/dev/term", &sbuf) < 0){ 
					system("mkdir /dev/term");
				}
				(void)sprintf(cmd, "mv %s %s", 
					gp->g_odevice, gp->g_device);
				system(cmd);
			}
		}
	}
	else
		(void)sprintf(gp->g_device,"/dev/%s",p);

	/* get speed label */
	p = (char *)strtok(NULL," \t#");
	gp->g_ttylabel = p;
	p = p + strlen(p) + 1;

	/* get comments if any */
	while (p && (*p != '\0') && (*p != '#'))
		p++;
	if ((p) && (*p == '#'))
		(void)sprintf(gp->g_comments, "-y \"%s\"",p+1);
	else
		*gp->g_comments = '\0';

	/* find out the major and minor of the device */
	if (stat(gp->g_device, &sbuf) < 0) {
		perror("stat on device failed");
		return(-1);
	}
	gp->g_minor = minor(sbuf.st_rdev);
	gp->g_major = major(sbuf.st_rdev);
	if (*ports[(int)gp->g_major][(int)gp->g_minor].pmtag == '\0')
		gp->g_exist = 0;
	else
		gp->g_exist = 1;
	(void)sprintf(gp->g_pmtag,"ttymon%d", gp->g_major);

	gp->g_prompt = find_prompt(gp->g_ttylabel);

	return(0);
}

/*
 * find_prompt(ttylabel) - find the corresponding prompt in /etc/gettydefs
 *			 - if the file does not exist or ttylabel not found
 *			   return the default "login: "
 */
char	*
find_prompt(ttylabel)
char	*ttylabel;
{
	static	FILE	*fp = NULL;
	static	char	buf[BUFSIZ];
	static	char	oldlabel[16] = { '\0' };
	static	int	openfailed = 0;
	char	*p;

	if (strcmp(ttylabel,oldlabel) == 0)
		return(buf);
	else
		strcpy(oldlabel,ttylabel);

	if (fp == NULL) {
		if (openfailed || (fp = fopen("/etc/gettydefs", "r")) == NULL) {
			openfailed = 1;
			strcpy(buf, "login: ");
			return(buf);
		}
	}
	rewind(fp);
	while (fgets(buf,BUFSIZ,fp) != NULL) {
		p = buf;
		if (*p != '#') {
			p = (char *)nexttok(p,"#");
			if (strncmp(p,ttylabel,strlen(ttylabel)) == 0) {
				p = (char *)nexttok(NULL,"#");
				p = (char *)nexttok(NULL,"#");
				p = (char *)nexttok(NULL,"#");
				if (p != NULL) {
					strcpy(buf,p);
					return(buf);
				}
				else
					break;
			}
		}
	}
	strcpy(buf, "login: ");
	return(buf);
}

char *
nexttok(str, delim)
char *str;
register char *delim;
{
	static char *savep;	/* the remembered string */
	register char *p;	/* pointer to start of token */
	register char *ep;	/* pointer to end of token */

	p = (str == NULL) ? savep : str ;
	if (p == NULL)
		return(NULL);
	ep = (char *)strpbrk(p, delim);
	if (ep == NULL) {
		savep = NULL;
		return(p);
	}
	savep = ep + 1;
	*ep = '\0';
	return(p);
}

/*
 * initialize_ports() - initialize the ports array
 */
static	void
initialize_ports()
{
	int	i,j;
	major_t	M;		/* Major and */
	minor_t m;		/* ..  minor */
	char	buf[BUFSIZ];
	FILE	*pfp;
	struct	stat	sbuf;
	char	*p, *s, *d;	/* ptr to pmtag, svctag, and device */

	for (i=0; i<MAXMAJOR; i++) 
		for (j=0; j<MAXMINOR; j++) {
			*ports[i][j].pmtag = '\0';
		}
	if (( pfp = popen(LISTSVC, "r")) == NULL) {
		perror("popen failed");
		exit(1);
	}
	while (fgets(buf, BUFSIZ, pfp) != NULL) {
		buf[strlen(buf)-1] = '\0';
		p = (char *)nexttok(buf,":");
		s = (char *)nexttok(NULL,":");
		d = (char *)nexttok(NULL,":");
		if (stat(d,&sbuf) < 0) {
			continue;
		}
		m = minor(sbuf.st_rdev);
		M = major(sbuf.st_rdev);
		strcpy(ports[(int)M][(int)m].pmtag, p);
		strcpy(ports[(int)M][(int)m].svctag, s);
		strcpy(ports[(int)M][(int)m].device, d);
#ifdef	DEBUG
printf("M = %d, m = %d, pmtag = %s, svctag = %s\n", 
			M,m,ports[M][m].pmtag,ports[M][m].svctag);
#endif
	}
}

/*
 * conv_defs(infile, outfile)	- convert /etc/gettydefs to /etc/ttydefs
 */
static	void
conv_defs(infile, outfile) 
char	*infile, *outfile;
{
	FILE	*gfp, *tfp;
	char	buf[BUFSIZ];
	char	*ptr;
	int	field;

	if ((gfp = fopen(infile, "r+")) == NULL) {
		perror("warning - open /etc/gettydefs failed");
		return;
	}
	if ((tfp = fopen(outfile, "w+")) == NULL) {
		perror("open /etc/ttydefs failed");
		return;
	}
	fputs(VERSION, tfp);
	while (fgets(buf,BUFSIZ,gfp) != NULL) {
		if (buf[0] == '#') { /* a line of comments */
			fputs(buf, tfp);
			continue;
		}
		for (ptr=buf, field=1; *ptr; ptr++) {
			switch(*ptr) {
			case '\r':
			case '\n': 
				if (ptr != buf)
					putc(*ptr, tfp);
				break;
			case '#':
				field++;
				*ptr = ' ';
				putc(':',tfp);
				break;
			case 'B':
				if ((field == 2) || (field == 3)) {
					if (isspace(*(ptr-1)) && 
					    isdigit(*(ptr+1)))
						break;
				}
			default:
				if ((field == 2) || (field == 3)) {
					if (isupper(*ptr))
						*ptr = tolower(*ptr);
				}
				else if (field == 4) {
					break;
				}
				putc(*ptr, tfp);
				break;
			} /* end switch */
		}
	}
}
