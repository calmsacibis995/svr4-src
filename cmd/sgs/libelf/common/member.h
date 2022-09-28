/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libelf:common/member.h	1.2"


/* Member handling
 *	Archive members have an ASCII header.  A Member structure
 *	holds internal information.  Because the ASCII file header
 *	may be clobbered, Member holds a private, safe copy.
 *	The ar_name member of m_hdr points at m_name except for string
 *	table names.  Ar_rawname in m_hdr always points at m_raw.
 *
 *	m_raw	The original ar_name with null termination.
 *		E.g., "name/           "
 *
 *	m_name	The processed name, with '/' terminator changed to '\0'.
 *		Unused for string table names.  E.g., "name\0           "
 */


#define ARSZ(m)	(sizeof((struct ar_hdr *)0)->m)


struct	Member
{
	Elf_Arhdr	m_hdr;
	Member		*m_next;
	int		m_err;
	char		m_raw[ARSZ(ar_name)+1];
	char		m_name[ARSZ(ar_name)+1];
};
