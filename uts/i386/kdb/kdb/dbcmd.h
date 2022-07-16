/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kdb:kdb/dbcmd.h	1.3.1.1"

/*
 * The debugger's fixed command table structure.
 */
struct cmdentry {
    char    *name;          /* command's textual name */
    void    (*func)();	    /* function to call to handle this command */
    int	    arg;	    /* argument to func */
    uchar   stackcheck;     /* stack bounds check, see STACKCHK(d,u) below  */
    uchar   parmcnt;        /* number of checked stack parameters for cmd   */
    uchar   parmtypes[3];   /* allowed type mask for each parm, 3 max       */
};

/*
 * Suffix table structure.
 */
struct suffix_entry {
    char    name[4];        /* suffix's textual name */
    db_as_t as;		    /* address space to assign */
    int	    size;	    /* operand size to use */
    int     brk;	    /* breakpoint type */
    int	    flags;	    /* special flags */
};

/* Values for suffix_entry flags */
#define SFX_NUM	 0x01	    /* suffix takes a (hex) numeric argument */
#define SFX_RSET 0x02	    /* numeric argument is register set number */

/*
 * Parameter type checking masks.
 *  These can be used singly or in combination to specify the allowed types
 *  of stack parameters for each command.
 */
#define T_NUMBER        0x01
#define T_NAME          0x02
#define T_STRING        0x04

/*
 * Stack depth checking masks.
 *  The stackcheck member of struct cmdentry specifies the lower and upper
 *  bounds of stack growth during the command's execution.  The predefined
 *  values S_1_0, S_2_0, and S_0_1 are the only ones currently needed, but
 *  for new commands, arbitrary checks can be constructed with STACKCHK().
 */

#define S_DOWNMASK      0x0f
#define S_UPMASK        0xf0
#define S_DOWNSHIFT     0
#define S_UPSHIFT       4

#define S_DOWN(x) (((unsigned)(x)&S_DOWNMASK)>>S_DOWNSHIFT)
#define S_UP(x)   (((unsigned)(x)&S_UPMASK)>>S_UPSHIFT)

#define STACKCHK(d,u)   ((((d)<<S_DOWNSHIFT)&S_DOWNMASK) | \
					(((u)<<S_UPSHIFT)&S_UPMASK))
#define S_1_0           STACKCHK(1,0)
#define S_2_0           STACKCHK(2,0)
#define S_0_1           STACKCHK(0,1)
