/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:mapscrn.c	1.1"
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <string.h>
#include <sys/at_ansi.h>
#include <sys/kd.h>

main(argc, argv)
int	argc;
char	**argv;
{
	extern int	optind;
	extern char	*optarg;
	int	ch, errflag = 0;
	int	cnt, kern, pervt, error;
	scrnmap_t	oscrnmap;
	char	*dflt, *basename = argv[0];
	struct scrn_dflt screen;
	FILE *fd;

#define DFLT "/usr/lib/console/screens"

	dflt = DFLT;
	fflush(stdout);
	kern = 0;
	pervt = 1; /* per VT by default */
	while ((ch = getopt(argc, argv, "dg")) != EOF) {
		switch (ch) {
		case 'd':
			kern = 1;
			break;
		case 'g':
			pervt = 0; /* global change */
			break;
		default:
			fprintf(stderr, "usage: %s [-dg] [file]\n", basename);
			exit(1);
			break;
		}
	}
	if (ioctl(0, KIOCINFO, 0) < 0 ) {
		fprintf(stderr,"Not the console driver\n");
		exit(1);
	}
	if ( kern ) {
		if (pervt) {
		   if (ioctl(0, GIO_SCRNMAP, &screen.scrn_map) < 0) {
			fprintf(stderr,"Error executing GIO_SCRNMAP ioctl\n");
			exit(1);
		   }
		   fscrnprint(stdout, &screen);
		} else {
		   screen.scrn_direction = KD_DFLTGET;
		   if (ioctl(0, KDDFLTSCRNMAP, &screen) < 0) {
			fprintf(stderr,"Error executing KDDFLTSCRNMAP ioctl\n");
			exit(1);
		   }
		   fscrnprint(stdout, &screen);
		}
	} else {
		if (pervt) {
		   if (ioctl(0, GIO_SCRNMAP, &screen.scrn_map) < 0) {
			fprintf(stderr,"Error executing GIO_SCRNMAP ioctl\n");
			exit(1);
		   }
		} else {
		   screen.scrn_direction = KD_DFLTGET;
		   if (ioctl(0, KDDFLTSCRNMAP, &screen) < 0) {
			fprintf(stderr,"Error executing KDDFLTSCRNMAP ioctl\n");
			exit(1);
		   }
		}
		if ( optind < argc ) {
			if( 0 == (fd =fopen(argv[optind],"r"))){
				fprintf(stderr,"error opening %s\n",argv[optind]);
				exit(1);
			}
		}else {
			if( 0 == (fd = fopen("/usr/lib/console/screens","r"))){
				fprintf(stderr,"error opening %s\n", DFLT);
				exit(1);
			}
		}
		fsrcnpar( fd, &screen, &error);
		fclose(fd);
		if (error < 0) {
			fprintf(stderr,"error input file\n");
			exit(1);
		}
		if (pervt) {
		   if (ioctl(0, PIO_SCRNMAP, &screen.scrn_map) < 0) {
			fprintf(stderr,"Error executing PIO_SCRNMAP ioctl\n");
			exit(1);
		   }
		} else {
		   screen.scrn_direction = KD_DFLTSET;
		   if (ioctl(0, KDDFLTSCRNMAP, &screen) < 0) {
			fprintf(stderr,"Error executing KDDFLTSCRNMAP ioctl\n");
			exit(1);
		   }
		}

	}
	exit(0);
}

char *ascnames[] = {
"nul","soh","stx","etx","eot","enq","ack","bel",
"'\\b'","'\\t'","'\\n'","vt","'\\f'","'\\r'","so","si",
"dle","dc1","dc2","dc3","dc4","nak","syn","etb",
"can","em","sub","esc","fs","gs","rs","ns",
"' '","'!'","'\"'","'#'","'$'","'%'","'&'","'\\''",
"'('","')'","'*'","'+'","','","'-'","'.'","'/'",
"'0'","'1'","'2'","'3'","'4'","'5'","'6'","'7'",
"'8'","'9'","':'","';'","'<'","'='","'>'","'?'",
"'@'","'A'","'B'","'C'","'D'","'E'","'F'","'G'",
"'H'","'I'","'J'","'K'","'L'","'M'","'N'","'O'",
"'P'","'Q'","'R'","'S'","'T'","'U'","'V'","'W'",
"'X'","'Y'","'Z'","'['","'\\\\'","']'","'^'","'_'",
"'`'","'a'","'b'","'c'","'d'","'e'","'f'","'g'",
"'h'","'i'","'j'","'k'","'l'","'m'","'n'","'o'",
"'p'","'q'","'r'","'s'","'t'","'u'","'v'","'w'",
"'x'","'y'","'z'","'{'","'|'","'}'","'~'","del"
};

fscrnprint(fd, oscreen)
struct scrn_dflt *oscreen;
FILE *fd;
{
	int i, j, cnt, n_char;
	fprintf(fd,"Font values:\n");
	for ( i=0, cnt = 0; cnt < NUM_ASCII ; i++ ) {
		for ( j=0; j<8; j++ )  {
			n_char = oscreen->scrn_map[cnt++];
			if ( (n_char > 126) || ( n_char < 8 ) ||
			   ((n_char < 32 ) && ( n_char > 13 )) || (n_char == 11))
				if ( n_char < 126 ) {
					if ( n_char < 8 )
						fprintf(fd,"\'\\00%o\'\t",n_char);
					else
						fprintf(fd,"\'\\0%o\'\t", n_char);
				} else 

					fprintf(fd,"\'\\%o\'\t", n_char);
			else
				fprintf(fd,"%s\t", ascnames[cnt-1]);
		}
		fprintf(fd, "\n" );
	}
}

fsrcnpar( fd, oscreen, err)
struct scrn_dflt *oscreen;
FILE *fd;
int *err;
{
	int val, cnt;
	char *c, buf;

	*err = 1;
	cnt = 0;
	c = &buf;
	while ( '\n' != (*c = getc(fd))){
	};
	while (!feof( fd )){
		if( cnt <  NUM_ASCII ) {
			fscanval(fd, &val);
			if( val >= 0 ) {
				oscreen->scrn_map[cnt++] = val;
			}
			else 
				if(val == -2 ) {
					*err = -1;
					return;
				}
		} else 
			return;
	} 
}

fscanval(pF, val)
FILE *pF;
int *val;
{
	int i, val1;
	char *c, cc, buf[6], *bp;
	
	bp =&buf[0];
	c = &cc;
	*bp++ = '\''; 
	*val = -1 ;
	*c = getc(pF);
	while ( ' ' == *c || '\t' == *c || '\n' == *c ) {
		if( feof( pF )){
			*val = -4;
			return;
		}
		*c = getc(pF);
	}
	if ( *c == '\'' ) {
		if(( *c = getc(pF)) == '\\' ) {
			*c = getc(pF);
			if(isdigit(*c) ) {
				*val = atoi(c);
				while ( (*c = getc(pF)) != '\'' ) {
					if ( !(isdigit(*c)) ) {
						*val = -2;	
						return;
					}
					*val = *val * 8 + atoi(c);
				}
				return;
			} 
			if(( *c != '\"') )
				*bp++ = '\\';
		}
		*bp++ = *c;
		while ( (*c = getc(pF)) != '\'' ) {
			*bp++ = *c;
		}
		*bp = *c;
		for(i=0; i < 128;i++) {
			if( !(strncmp(buf, ascnames[i], strlen(ascnames[i])))) {
				*val = i;
				return;
			}
		}
		*val = -2;	
		return;
		
	} else {  
		*val = -3;	
		return;
	}
}
