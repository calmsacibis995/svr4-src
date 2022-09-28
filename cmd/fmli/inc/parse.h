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

#ident	"@(#)fmli:inc/parse.h	1.1"

#ifndef MALLOC
#include	<malloc.h>
#endif

#define MAXATTS 10
#define dumpstr(a, b) fputs((b) ? (b) : "", (a)); fputs("\n", (a))
#define nicestr(a) (a) ? (a) : ""
#define LOWER(a) (isupper(a) ? _tolower(a) : (a))
#define readnum(fp, num, buf) fgets((buf), STR_SIZE, (fp)); *(num) = atol(buf)
#define readsave(fp, str, buf) *(str) = fgets((buf), STR_SIZE, (fp)); buf[strlen(buf) - 1] = '\0';  *(str) && *(buf) ? (*(str) = strsave(buf)) : (*(str) = NULL)
#define readput(fp, str) fgets((str), STR_SIZE, (fp)); (str)[strlen((str)) - 1] = '\0'
#define dumpnum(fp, num) fprintf((fp), "%d\n", (num))
#define FREE 0
#define NOFREE 1
#define REPLY 1
#define ATTHEAD 1
#define STATHEAD 2
#define MAILHEAD 4
#define CONHEAD 8
#define NOFUNCS 4
#define STACK_SIZE 5
#define DATESIZE 32
#define ADDON 0
#define FILL 1
#define INBOX "/IN_BOX"
#undef STR_SIZE
#define STR_SIZE 256
#define ADDRSIZE 50
#define WORD 1
#define KEYWORD 2
#define NOTKEY 4
#define PDONE 3
#define SOMETHING 1
#define RECEIPT   2
#define BUSINESS  4
#define URGENT    8
#define PRIORITY  16
#define SKIP 	1
#define NOSKIP	2
#define STATUS 1
#define READ_HEADER	0
#define NUM_CHECK	1
#define UNPACK		2
#define OEH_BAD		0x01
#define PART_BAD	0x02
#define OBJ_UNK		0x04
#define ENC_BAD		0x08
#define OEH_NOT		0x10
#define OBJ_OEU		0x20

struct oeh {
	char *num;
	char *type;
	char *name;
	char *encrytest;
	int count;
	char *file;
};

struct addr {
	char info[STR_SIZE];
	char addr[ADDRSIZE];
	int mask;
	struct addr *next;
};

struct ucmfhead {
	char *file;
	int status;
	struct addr *ufrom;
	struct addr *from;
	struct addr *cc;
	long conlen;
	char *phone;
	char *contype;
	int defopt;
	char *enc;
	char *date;
	char *expire;
	char *import;
	char *kwd;
	int curatt;
	int mset;
	char *mts;
	struct addr *bcc;
	char *origdate;
	char *subj;
	struct addr *replyto;
	char *replyid;
	struct addr *sender;
	char *sens;
	int noatts;
	int flags;
	struct oeh *atts[MAXATTS];
	struct addr *to;
};
#define RP_SET 1
#define RP_UNSET 2
#define RP_USE 3
