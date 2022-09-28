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
#ident	"@(#)fmli:sys/chgenv.c	1.1"

#include	<stdio.h>
#include	<fcntl.h>

#define LOOKING		1
#define SKIPPING	2

char *
chgenv(file, name, val)
char	*file;
char	*name;
char	*val;
{
	char	inbuf[BUFSIZ];
	char	outbuf[BUFSIZ];
	register char	*p;
	char	*index;
	register int	c;
	register int	state;
	register FILE	*infp;
	register FILE	*outfp;
	int		len;
	char	*strnsave();
	char	*backslash();
	FILE	*tempfile();

	if ((outfp = tempfile(NULL, "w+")) == NULL)
		return NULL;
	setbuf(outfp, outbuf);
	if (val) {
		fputs(name, outfp);
		putc('=', outfp);
		len = 2 * strlen(val);
		fputs(p = backslash(strnsave(val, len), len), outfp);
		free(p);
		putc('\n', outfp);
	}
	if ((infp = fopen(file, "r+"))) {
		setbuf(infp, inbuf);
		state = LOOKING;
		index = name;
		for (c = getc(infp); c != EOF; c = getc(infp)) {
			if (state == SKIPPING) {
				if (c == '\n') {
					state = LOOKING;
					index = name;
				}
				continue;
			}
			if (state == LOOKING) {
				/* if we are in name */
				if (*index) {
					if (c == *index) {
						index++;
						continue;
					}
				}
				/* found name, look for "=" */
				else if (c == '=') {
					state = SKIPPING;
					continue;
				}
				/* failure, copy line to outfile */
				for (p = name; p < index; p++)
					putc(*p, outfp);
				state = 0;
			}
			if (c == '\n') {
				state = LOOKING;
				index = name;
			}
			putc(c, outfp);
		}
		fclose(infp);
	}
	{
		register int	fd;
		register int	n;

		if ((fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0640)) >= 0) {
			fseek(outfp, 0L, 0);
			while ((n = fread(inbuf, 1, sizeof(inbuf), outfp)) > 0)
				write(fd, inbuf, n);
			close(fd);
		}
		else
			val = NULL;
	}
	fclose(outfp);
	return val;
}
