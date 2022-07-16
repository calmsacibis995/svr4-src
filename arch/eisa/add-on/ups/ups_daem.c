/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)eisa:add-on/ups/ups_daem.c	1.3"
/* 
 *	Copyright (C) Ing. C. Olivetti & C. S.p.a., 1989.
 *
 *	daemon process for UPS support.
 *	called at init 2 time		:  "ups_daem 15 powertest".
 *	called at init 0 time		:  "ups_daem 15 powerfail".
 *
 */

#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stream.h>
#include <sys/stropts.h>
#include <signal.h>
#include <time.h>

#include "fpan.h"

#define POLL_TIME	10	/* time of polling of status register	*/
#define GAP_TIME	20	/* min. time interval of AC powerfail	*/
				/* that cause a shutdown; for less than	*/
				/* GAP_TIME sec of AC fail, no shutdown	*/

/* local action to be performed states */
#define	ACTION_FAIL	1	/* power failed. go directly to UPS */
#define	ACTION_TEST	2	/* check status occasionally and report changes */
#define	ACTION_OFF	3	/* disconnect power immediately */

char daemon_name[] = "UPS DAEMON";
char abort_string[] = "UPS DAEMON ABORTED";

char * status_messages[] = {
	"",	"",		/* first bit no state change report allowed */
	"UPS present",
	"UPS absent",
	"Battery charge level low",
	"Battery charge level restored",
	"AC power fail detected",
	"AC power restored",
	"High temperature detected",
	"Normal operating temperature restored"
};

/* for each bit in new_state that is different, print a status message */

report_state_changes(old_state, new_state)
unsigned old_state, new_state;
{
	register int i;
	register unsigned changed = old_state ^ new_state;
	register unsigned mask = new_state;
	char buf[80];

	for (i=1; i < 5; i++) {	/* first bit not used */
		if (changed & (1<<i) ) {
			if (mask & (1<<i) ) 
				write_log( status_messages[(i+i)] );
			else
				write_log( status_messages[(i+i) +1] );
		}
	}
	
	if (changed & BAT_LEVEL) {
		if (mask & BAT_LEVEL) {

			printf("\n\t\t\t***** WARNING *****\n");
			printf("UPS battery voltage is low:\nstarting now, ");
			printf("the system may not be able to sustain a graceful\n");
			printf("shutdown in case of powerfail.\n");
		}
	}
}

/* name stamp and time stamp each message, then */
/* write to standard output and to logger. */
/* sorry, I don't have the Streams programming manual to code */
/* the right stuff for streams error logging */

write_log(str)
char * str;
{
	FILE	*fp;
	time_t	clock;
	char *message, buf[256];

	time(&clock);

	sprintf(buf,"%s: %s %s", daemon_name, str, ctime(&clock) );

	if ((fp = fopen("/usr/adm/upslog", "a")) != NULL)
	{
		fprintf(fp,"%s",buf);
		fclose(fp);
	}
	printf("%s",buf);
}

usage() {
	printf("%s: bad invocation. UPS support not activated\n",daemon_name);
	exit(-1);
}

main(argc, argv)
int argc;
char ** argv;
{
	struct	strioctl	ioctbuf;
	unsigned char		upsstatus;
	int			fd, res;
	int			minutes_good;
	int			action_class;
	unsigned char		current_state;

	if (argc != 3) usage();

	if ( (minutes_good=atoi(argv[1])) < 0) usage();

	if (strcmp(argv[2],"powerfail") == 0)
		action_class = ACTION_FAIL;

	else if (strcmp(argv[2],"powertest") == 0)
		action_class = ACTION_TEST;

	else
		usage();

	/*
	 * better to set permissions to ---x------;
	 * in any case test if super-user
	 */
	if (getuid() != 0)
	{
		printf("%s: Permission denied\n",daemon_name);
		exit(-1);
	}

#ifndef TEST
	current_state = UPS_PRESENT;
	report_state_changes(current_state,upsstatus);
	current_state = upsstatus;

	upsstatus = UPS_PRESENT | BAT_LEVEL | HIGH_TEMP;
printf("now the other changes\n");
	report_state_changes(current_state,upsstatus);
	current_state = upsstatus;
printf("now the other changes\n");

	report_state_changes(current_state,upsstatus);
	current_state = upsstatus;
	
exit(0);
#endif
	if ((fd = open("/dev/front_panel", O_RDWR)) < 0)
	{
		perror(abort_string);
		exit(-1);
	}

	ioctbuf.ic_cmd = FPAN_STATUS;
	ioctbuf.ic_timout = 0;
	ioctbuf.ic_dp = (char *) &upsstatus;
	ioctbuf.ic_len = 1; /*sizeof(upsstatus);*/

	if ((res = ioctl(fd, I_STR, &ioctbuf)) < 0)
	{
		perror(abort_string);
		close(fd);
		exit(-1);
	}

	if (!(upsstatus & UPS_PRESENT))
	{
		printf("%s: UPS not present\n",daemon_name);
		close(fd);
		exit(0);
	}

    if (action_class == ACTION_FAIL) {

	/*
	 * powerfail:  set shutdown complete and die.
	 */

	write_log("UPS shutdown complete");

	ioctbuf.ic_cmd = FPAN_UPSSUP;
	ioctbuf.ic_timout = 0;
	ioctbuf.ic_dp = (char *) &upsstatus;
	ioctbuf.ic_len = 1; /*sizeof(upsstatus);*/
	if ((res = ioctl(fd, I_STR, &ioctbuf)) < 0)
		perror(abort_string);
	/* send shutdown message on the front_panel */
	ioctbuf.ic_cmd = FPAN_SHTCMP;
	if ((res = ioctl(fd, I_STR, &ioctbuf)) < 0)
		perror(abort_string);
	close(fd);

	sync(); sync();
	for (;;) ;

    }

    else {	/* action_class == ACTION_TEST */
	write_log("UPS support started.");
	current_state = UPS_PRESENT;

loop:
	/*
	 * loop until not powerfail
	 */
	do {
		sleep(POLL_TIME);

		if ((res = ioctl(fd, I_STR, &ioctbuf)) < 0)
		{
			perror(abort_string);
			close(fd);
			exit(-1);
		}
		
		report_state_changes(current_state,upsstatus);
		current_state = upsstatus;
	}
	while(!(upsstatus & PW_FAIL));

	sleep(GAP_TIME);

	if ( !((ioctl(fd, I_STR, &ioctbuf)) < 0) 
	     && !(upsstatus & PW_FAIL)
	     && !(upsstatus & BAT_LEVEL) ) {
		goto loop;	/* AC power is returned */
	}

	write_log("UPS shutdown in progress");

	ioctbuf.ic_cmd = FPAN_UPSSUP;
	ioctbuf.ic_timout = 0;
	ioctbuf.ic_dp = (char *) &upsstatus;
	ioctbuf.ic_len = 1; /*sizeof(upsstatus);*/
	if ((res = ioctl(fd, I_STR, &ioctbuf)) < 0)
		perror(abort_string);
	/* send shutdown message on the front_panel */
	ioctbuf.ic_cmd = FPAN_SHTPRG;
	if ((res = ioctl(fd, I_STR, &ioctbuf)) < 0)
		perror(abort_string);
	close(fd);

	/*
	 *	send powerfail signal to init.
	 *	init in turn will exec all "inittab" entries
	 *	with their mode fields set to "powerfail".
	 *
	 */
	kill(1, SIGPWR);
	/*DEBUG
	 *	that is stop to respawning new gettys and wait until
	 *	powerfail end.
	system("init 0");
	 */
    } /* end action_class == ACTION_TEST */

}

