/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucbtroff:troff.d/devaps/build.c	1.1.1.1"

/*******************************************************************

		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 


#include <stdio.h>


#define	LIBDIR	"charlib"				/* files found in *fontdir/LIBDIR */


/*
 *
 * These variables are used to save and later restore the post-processor's
 * environment.
 *
 */


int		ohpos;
int		ovpos;
int		ofont;
int		osize;

int		env = 0;						/* nonzero if environment is saved */


extern int		hpos;
extern int		vpos;
extern int		font;
extern int		size;

extern char		*fontdir;
extern char		devname[];
extern FILE		*tf;

#define oput(n)	putc(n, tf)


/*****************************************************************************/


newfile(name, size)


	char	*name;						/* start reading from this file */
	int		size;						/* may use it to get the file */


{


	FILE	*fp;
	FILE	*charfile();


	/********************************************************************
	 *																	*
	 *		Used when we want to take the post-processor's input from	*
	 *	a different file for a while. Added to handle problems with the	*
	 *	new logos, but it could easily be used to define other special	*
	 *	characters.														*
	 *																	*
	 ********************************************************************/


	if ( env || (fp = charfile(name, size)) == NULL )
		return(1);

	save_env();
	nconv(fp);
	restore_env();
	fclose(fp);

	return(0);

}	/* End of newfile */


/*****************************************************************************/


FILE *charfile(name, size)


	char	*name;						/* start reading from this file */
	int		size;						/* size of the character to print */


{


	char	path[100];					/* file pathname put here */
	FILE	*fp;						/* file pointer for *path */


	/********************************************************************
	 *																	*
	 *		First tries to open file *name.size in the right directory,	*
	 *	and if it can't then it tries *name. Returns the file pointer	*
	 *	or NULL if either file can't be opened.							*
	 *																	*
	 ********************************************************************/


	sprintf(path, "%s/dev%s/%s/%s.%d", fontdir, devname, LIBDIR, name, size);

	if ( (fp = fopen(path, "r")) == NULL )  {
		sprintf(path, "%s/dev%s/%s/%s", fontdir, devname, LIBDIR, name);
		fp = fopen(path, "r");
	}	/* End if */

	return(fp);

}	/* End of charfile */


/*****************************************************************************/


save_env()


{


	/********************************************************************
	 *																	*
	 *		Before we start reading from a different file we'll want	*
	 *	to save the values of the variables that will be needed to get	*
	 *	back to where we were.											*
	 *																	*
	 ********************************************************************/


	hflush();

	ohpos = hpos;
	ovpos = vpos;
	ofont = font;
	osize = size;

	env = 1;

}	/* End of save_env */


/*****************************************************************************/


restore_env()


{


	/********************************************************************
	 *																	*
	 *		Hopefully does everything needed to get the post-processor	*
	 *	back to where it was before the input was diverted.				*
	 *																	*
	 ********************************************************************/


	hgoto(ohpos);
	vgoto(ovpos);

	setfont(ofont);
	setsize(osize);

	env = 0;

}	/* End of restore_env */


/*****************************************************************************/


nconv(fp)


	FILE	*fp;						/* new file we're reading from */


{


	int		ch;							/* first character of the command */
	int		c, n;						/* used in reading chars from *fp */
	char	str[100];					/* don't really need this much room */


	/********************************************************************
	 *																	*
	 *		A restricted and slightly modified version of the conv()	*
	 *	routine found in all of troff's post-processors. It should only	*
	 *	be used to interpret the special character building files found	*
	 *	in directory *fontdir/LIBDIR.									*
	 *																	*
	 ********************************************************************/


	while ( (ch = getc(fp)) != EOF )  {

		switch ( ch )  {

			case 'w':					/* just ignore these guys */
			case ' ':
			case '\n':
			case 0:
					break;

			case 'c':					/* single ASCII character */
					put1(getc(fp));
					break;

			case 'C':					/* special character */
					fscanf(fp, "%s", str);
					put1s(str);
					break;

			case 'h':					/* relative horizontal motion */
					fscanf(fp, "%d", &n);
					hmot(n);
					break;

			case 'v':
					fscanf(fp, "%d", &n);
					vmot(n);
					break;

			case 'x':					/* device control - font change only */
					fscanf(fp, "%s", str);
					if ( str[0] == 'f' )  {
						fscanf(fp, "%d %s", &n, str);
						loadfont(n, str, "");
					}	/* End if */
					break;

			case 's':					/* set point size */
					fscanf(fp, "%d", &n);
					setsize(t_size(n));
					break;

			case 'f':					/* use this font */
					fscanf(fp, "%s", str);
					setfont(t_font(str));
					break;

			case 'b':
					fscanf(fp, "%d", &n);
					oput(n);
					break;

			case 'W':
					fscanf(fp, "%d", &n);
					putint(n);
					break;


			case '#':					/* comment */
					while ( (c = getc(fp)) != '\n'  &&  c != EOF ) ;
					break;

		}	/* End switch */

	}	/* End while */

}	/* End of nconv */


/*****************************************************************************/


