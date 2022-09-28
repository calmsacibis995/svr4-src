/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)acomp:common/debug.h	55.1"
/* debug.h */


/* Definitions to support debugging output. */


extern int db_linelevel;	/* line number debug level */
extern int db_symlevel;		/* symbol info debug level */
extern int db_curline;		/* current debugger line */

extern void db_s_file();
extern void db_e_file();
extern void db_begf();
extern void db_s_fcode();
extern void db_e_fcode();
extern void db_endf();
extern void db_s_block();
extern void db_e_block();
extern void db_symbol();
extern void db_lineno();
extern void db_sue();
#ifdef ELF_OBJ
extern void db_sy_clear();
#endif

/* There's a tie-in between the way the "cc" command passes
** the -ds, -dl flags, how main.c sets the db_linelevel and
** db_symlevel flags, and how debug.c/elfdebug.c use them.
*/
#define	DB_LEVEL2	0	/* No -ds, -dl */
#define	DB_LEVEL0	1	/* -ds, -dl set */
#define	DB_LEVEL1	2	/* Not supported by "cc". */

/* These select different behavior if function-at-a-time
** compilation.
*/
#ifdef FAT_ACOMP

#define	DB_S_FILE(s)	cg_q_str(db_s_file, s)
#define	DB_E_FILE()	cg_q_call(db_e_file)
#define	DB_BEGF(sid)	cg_q_sid(db_begf, sid)
#define	DB_S_FCODE()	cg_q_call(db_s_fcode)
#define	DB_E_FCODE()	cg_q_call(db_e_fcode)
#define	DB_S_BLOCK()	cg_q_call(db_s_block)
#define	DB_E_BLOCK()	cg_q_call(db_e_block)
#define	DB_SYMBOL(sid)	cg_q_sid(db_symbol, sid)
#define	DB_ENDF()	cg_q_call(db_endf)
#define	DB_LINENO()	cg_q_call(db_lineno)
#define	DB_SUE(t)	cg_q_type(db_sue, t)
#define	DB_SY_CLEAR(sid) cg_q_sid(db_sy_clear, sid)

#else	/* ! FAT_ACOMP */

#define DB_S_FILE(s)	db_s_file(s)
#define DB_E_FILE()	db_e_file()
#define DB_BEGF(sid)	db_begf(sid)
#define DB_S_FCODE()	db_s_fcode()
#define DB_E_FCODE()	db_e_fcode()
#define DB_ENDF()	db_endf()
#define DB_S_BLOCK()	db_s_block()
#define DB_E_BLOCK()	db_e_block()
#define DB_SYMBOL(sid)	db_symbol(sid)
#define DB_LINENO()	db_lineno()
#define DB_SUE(t)	db_sue(t)
#define DB_SY_CLEAR(sid) db_sy_clear(sid)

#endif	/* def FAT_ACOMP */
