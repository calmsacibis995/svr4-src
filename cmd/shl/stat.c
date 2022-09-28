/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)shl:stat.c	1.4.2.1"
#include	"defs.h"

static struct sxtblock status;

all_layers(flag)
	int flag;
{
	int i;

	status.input = 0;
	status.output = 0;

	if (ioctl(cntl_chan_fd, SXTIOCSTAT, &status) == SYSERROR)
		fprintf(stderr, "ioctl SXTIOCSTAT failed\n");

	for (i = 1; i <= max_index; ++i)
		if (layers[i])
			stat_layer(i, flag);
}


one_layer(name, flag)
	char *name;
	int flag;
{
	int i;

	if (i = lookup_layer(name))
		stat_layer(i, flag);
}


stat_layer(i, flag)
	int i;
	int flag;
{
	printf("%s (%d)", layers[i]->name, layers[i]->p_grp);

	if (status.input & (1 << i))
		printf(" blocked on input\n");
	else if (status.output & (1 << i))
		printf(" blocked on output\n");
	else
		printf(" executing or awaiting input\n");

	fflush(stdout);

	if (flag)
	{
		char buff[50];

		sprintf(buff, "%d", layers[i]->p_grp);
		system(buff);
		printf("\n");
	}
}

