/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:cmd/mbusutils/mb2.d/cci.d/410fw.d/utils.c	1.3"

#include <stdio.h>

#ifdef MBIIU
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/ics.h>
#include <sys/mb2taiusr.h>
#include <ctype.h>
#endif

#include "common.h"

		int ic_fd;

/****************************************************************************/
/*                                                                          */
/*                     SYP MDP INITIALIZATION       				        */
/*                     ----------------------				                */
/*                                                                          */
/****************************************************************************/

#ifdef MBIIU
mdp_init()

{
	ic_fd = open("/dev/ics", O_RDWR);
	if (ic_fd == -1) {
		return(-1);
	}
	return(0);
}

#endif

/****************************************************************************/
/*                                                                          */
/*                     INITIALIZATION 			    		  		        */
/*                     --------------						                */
/*                                                                          */
/****************************************************************************/

init()

{

#ifdef MBIIU
	return(mdp_init());
#endif

}

/****************************************************************************/
/*                                                                          */
/*                     GET INTERCONNECT       						        */
/*                     ----------------						                */
/*                                                                          */
/****************************************************************************/

unsigned char get_ic(slot, reg)
unsigned char slot;
unsigned short reg;

{
	unsigned char value;
	unsigned short status;
	
#ifdef MBIIU
	status = ics_read(ic_fd, slot, reg, &value, 1);
#else
	value = rq$get$interconnect(slot, reg, &status);
#endif

	if (status != 0) 
		return(0xff);

	return(value);
} 

/****************************************************************************/
/*                                                                          */
/*                     PUT INTERCONNECT       						        */
/*                     ----------------						                */
/*                                                                          */
/****************************************************************************/

put_ic(slot, reg, value)
unsigned char slot;
unsigned short reg;
unsigned char value;

{
	unsigned short status;
	
#ifdef MBIIU
	status = ics_write(ic_fd, slot, reg, &value,1);
#else
	rq$set$interconnect(value, slot, reg, &status);
#endif

	if (status != 0) 
		return(-1);

	return(0);
}

/****************************************************************************/
/*                                                                          */
/*                     FIND RECORD		       						        */
/*                     -----------							                */
/*                                                                          */
/****************************************************************************/

find_rec(slot, rec_type)

unsigned char slot;
unsigned char rec_type;

{
	
#ifdef MBIIU
	return(ics_find_rec(ic_fd, slot, rec_type));
#endif

}

/****************************************************************************/
/*                                                                          */
/*                     			DO SLEEP 				      		        */
/*                     			--------		        			        */
/*                                                                          */
/****************************************************************************/

void do_sleep(count)

unsigned char count;

{
	unsigned short status;

#ifdef MBIIU
	sleep(count);
#else
	rq$sleep(count, &status);
#endif
}

/****************************************************************************/
/*                                                                          */
/*                     			EXECUTE 				      		        */
/*                     			--------		        			        */
/*                                                                          */
/****************************************************************************/

execute(name_p)

char *name_p;

{

#ifdef MBIIU
	return(system(name_p));
#endif

}

/****************************************************************************/
/*                                                                          */
/*                     GET SLOT ID       		    					    */
/*                     -----------							                */
/*                                                                          */
/****************************************************************************/

get_slot_id()		
{
	
	int reg;
	unsigned char value;
	unsigned short status;

#ifdef MBIIU
	reg = ics_find_rec(ic_fd, MY_SLOT, PSB_CONTROL_REC);
	if (reg < 0) {
		return(-1);
	}

	status = ics_read(ic_fd, MY_SLOT, (reg + ICS_SLOT_ID_OFFSET), &value, 1);
	value = value >> 3;
	return((int)(value)); 

#else
	
	return(rq$get$host$id(&status));

#endif

}
