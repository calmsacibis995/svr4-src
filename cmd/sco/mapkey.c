/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sco:mapkey.c	1.1"
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/at_ansi.h>
#include <sys/kd.h>
#include <stdio.h>



main( Narg, ppCarg )
int Narg;
char **ppCarg;
{
	static struct key_dflt mykeymap;
	extern int optind;
	FILE *pF;
	int arg, base,i,j, kdfd, pervt, kern;
	char *dflt;

#define DFLT "/usr/lib/keyboard/keys ";	

	base = 10; 
	kern = pervt = 0;
	dflt=DFLT;

	while( EOF != (arg = getopt( Narg, ppCarg, "Vdox")) ){
		switch(arg){
		case 'd':
			kern = 1;
			break;
		case 'o':
			base = 8;
			break;
		case 'x':
			base = 16;
			break;
		case 'V':
			pervt = 1;
			break;
		default:
			fprintf(stderr,"unknown option -%c", arg);
			fprintf(stderr,"usage: %s [-doxV] [file]\n",ppCarg[0]);
			return 1;
			break;
		}
	}
	if (ioctl(0, KIOCINFO, 0) < 0 ) {
		fprintf(stderr,"mapkey can only be run from a virtual terminal\n");
		fprintf(stderr,"on a graphics workstation.\n");
		fprintf(stderr,"usage: %s [-doxV] [file]\n",ppCarg[0]);
		exit(2);
	}
	kdfd = 0;
	if( kern ){
		if( optind < Narg ){
			fprintf(stderr,"unknown option -%c", arg);
			fprintf(stderr,"usage: %s [-doxV] [file]\n",ppCarg[0]);
			exit(1);
		}
		if (pervt) {
			if (ioctl(kdfd,GIO_KEYMAP,&mykeymap.key_map) < 0) {
				perror("keymap: error in GIO_KEYMAP ioctl");
				exit (1);
			}
		} else {
			mykeymap.key_direction = KD_DFLTGET;
			if (ioctl(kdfd, KDDFLTKEYMAP, &mykeymap) == -1)
			{
				perror("keymap: error in KDDFLTKEYMAP ioctl");
				exit (1);
			}
		}
		fprintmap(stdout, base, &mykeymap.key_map);
	} else {
		if(geteuid() != 0 ) {
			fprintf(stderr,"not super-user\n");
			return(1);
		}
		if( optind < Narg ){
			if( 0 == (pF = fopen(ppCarg[optind],"r"))){
				fprintf(stderr,"can't open %s\n",ppCarg[optind]);
				exit(1);
			}
		} else{ 
			if( 0 == (pF = fopen( "/usr/lib/keyboard/keys" ,"r"))){
				fprintf(stderr,"can't open %s\n", dflt );
				exit(1);
			}
		}
		fparsemap(pF,&mykeymap.key_map);
		fclose(pF);
		if (pervt) {
		   if (ioctl(kdfd,PIO_KEYMAP,&mykeymap.key_map) == -1) {
		 	fprintf(stderr,"mapkey: error in KDDFLTKEYMAP ioctl");
			exit (1);
		   }
		} else {
		   mykeymap.key_direction = KD_DFLTSET;
		   if (ioctl(kdfd,KDDFLTKEYMAP,&mykeymap) == -1) {
			fprintf(stderr,"mapkey: error in KDDFLTKEYMAP ioctl");
			exit (1);
		   }
		}
	} 
	return (0);
}
