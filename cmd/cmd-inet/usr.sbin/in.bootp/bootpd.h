/*
 *	Copyrighted as an unpublished work.
 *      (c) Copyright 1990 INTERACTIVE Systems Corporation
 *      All Rights Reserved.
 *
 *      Copyright 1987, 1988, 1989 Lachman Associates, Incorporated (LAI)
 *      All Rights Reserved.
 *
 *      System V STREAMS TCP was jointly developed by Lachman
 *      Associates and Convergent Technologies.
 */

/*
 * bootpd.h -- common header file for all the modules of the bootpd program.
 *
 *
 * Copyright 1988 by Carnegie Mellon.
 *
 * Permission to use, copy, modify, and distribute this program for any
 * purpose and without fee is hereby granted, provided that this copyright
 * and permission notice appear on all copies and supporting documentation,
 * the name of Carnegie Mellon not be used in advertising or publicity
 * pertaining to distribution of the program without specific prior
 * permission, and notice be given in supporting documentation that copying
 * and distribution is by permission of Carnegie Mellon and Stanford
 * University.  Carnegie Mellon makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */


#ifndef TRUE
#define TRUE	1
#endif
#ifndef FALSE
#define FALSE	0
#endif

#ifndef SIGUSR1
#define SIGUSR1			 30	/* From 4.3 <signal.h> */
#endif

#define MAXHTYPES		  7	/* Number of htypes defined */
#define MAXHADDRLEN		  6	/* Max hw address length in bytes */
#define MAXSTRINGLEN		 80	/* Max string length */

/*
 * Return the length in bytes of a hardware address of the given type.
 */
#define haddrlength(type)	(maphaddrlen[(int) (type)])

/*
 * Return pointer to static string which gives full network error message.
 */
#define get_network_errmsg get_errmsg


/*
 * Variables shared among modules.
 */

extern int debug;
extern char *bootptab;
extern unsigned maphaddrlen[MAXHTYPES + 1];

extern hash_tbl *hwhashtable;
extern hash_tbl *iphashtable;
extern hash_tbl *nmhashtable;
extern unsigned char vm_cmu[4];
extern unsigned char vm_rfc1048[4];


/*
 * Functions shared among modules
 */

extern void report();
extern char *get_errmsg();
extern char *haddrtoa();
extern int readtab();



/*
 * Nice typedefs. . .
 */

typedef int boolean;
typedef unsigned char byte;


/*
 * Data structure used to hold an arbitrary-lengthed list of IP addresses.
 * The list may be shared among multiple hosts by setting the linkcount
 * appropriately.
 */

struct in_addr_list {
    unsigned		linkcount, addrcount;
    struct in_addr	addr[1];		/* Dynamically extended */
};


/*
 * Data structures used to hold shared strings and shared binary data.
 * The linkcount must be set appropriately.
 */

struct shared_string {
    unsigned		linkcount;
    char		string[1];		/* Dynamically extended */
};

struct shared_bindata {
    unsigned		linkcount, length;
    byte		data[1];		/* Dynamically extended */
};


/*
 * Flag structure which indicates which symbols have been defined for a
 * given host.  This information is used to determine which data should or
 * should not be reported in the bootp packet vendor info field.
 */

struct flag {
    unsigned	bootfile	:1,
		bootsize	:1,
		bootsize_auto	:1,
		cookie_server	:1,
		domain_server	:1,
		gateway		:1,
		generic		:1,
		haddr		:1,
		homedir		:1,
		htype		:1,
		impress_server	:1,
		iaddr		:1,
		log_server	:1,
		lpr_server	:1,
		name_server	:1,
		name_switch	:1,
		rlp_server	:1,
		send_name	:1,
		subnet_mask	:1,
		time_offset	:1,
		timeoff_auto	:1,
		time_server	:1,
		vendor_magic	:1,
		vm_auto		:1;
};



/*
 * The flags structure contains TRUE flags for all the fields which
 * are considered valid, regardless of whether they were explicitly
 * specified or indirectly inferred from another entry.
 *
 * The gateway and the various server fields all point to a shared list of
 * IP addresses.
 *
 * The hostname, home directory, and bootfile are all shared strings.
 *
 * The generic data field is a shared binary data structure.  It is used to
 * hold future RFC1048 vendor data until bootpd is updated to understand it.
 *
 * The vm_cookie field specifies the four-octet vendor magic cookie to use
 * if it is desired to always send the same response to a given host.
 *
 * Hopefully, the rest is self-explanatory.
 */

struct host {
    struct flag		    flags;		/* ALL valid fields */
    struct in_addr_list	    *cookie_server,
			    *domain_server,
			    *gateway,
			    *impress_server,
			    *log_server,
			    *lpr_server,
			    *name_server,
			    *rlp_server,
			    *time_server;
    struct shared_string    *hostname,
			    *homedir,
			    *bootfile;
    struct shared_bindata   *generic;
    byte		    vm_cookie[4],
			    htype,  /* RFC826 says this should be 16-bits but
				       RFC951 only allocates 1 byte. . . */
			    haddr[MAXHADDRLEN];
    long		    time_offset;
    unsigned int	    bootsize;
    struct in_addr	    iaddr,
			    subnet_mask;
};
