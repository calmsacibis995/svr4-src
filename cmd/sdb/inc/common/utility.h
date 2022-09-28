/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:inc/common/utility.h	1.20"
#ifndef utility_h
#define utility_h


#include	<sys/types.h>
#include	<signal.h>
#include	<sys/user.h>
#include	<sys/procfs.h>
#if defined(__cplusplus)
class Process;
struct Location;
class SrcFile;
class Assoccmds;
#else
typedef struct Process Process;
typedef struct Location Location;
typedef struct SrcFile SrcFile;
typedef struct Assoccmds Assoccmds;
#endif
#ifndef Itype_h
typedef unsigned long Iaddr;
#endif

#if defined(__cplusplus) || defined(__STDC__)

extern int		interrupted;
extern Process *	current_process;
extern Process *	create_process( char *, sigset_t );
extern Process *	grab_process( int, sigset_t );
extern Process *	grab_core( char * adotout, char * core, int, sigset_t, int );
extern int 		grabbed( Process *);
extern int		destroy_process( Process * );
#if defined(__cplusplus)
extern int		release_process( Process *, int run = 1 );
#else
extern int		release_process( Process *, int run );
#endif

extern int		inform_processes();

extern int		run( Process *, int, Location *, int );
extern int		stmt_step( Process *, int, Location *, int, int );
extern int		instr_step( Process *, int, Location *, int, int );
extern int		stmt_step_over( Process *, int, Location *, int, int );
extern int		instr_step_over( Process *, int, Location *, int, int );
extern int		resume( Process * );

extern int		evaluate_expr( Process *, char * expr , char * fmt );
extern int		evaluate_addr( Process *, char * expr , char * fmt );

extern Assoccmds *	new_assoc();
extern int		add_assoc( Assoccmds *, char * );

extern int		print_map( Process * );

#if defined(__cplusplus)
extern int		print_stack( Process *, int how_many, int quick = 0 );
#else
extern int		print_stack( Process *, int how_many, int quick );
#endif

extern int		printregs( Process *, int );

#else /* old C */

extern int		interrupted;
extern Process *	current_process;
extern Process *	create_process(/* char * */);
extern Process *	grab_process(/* int */);
extern Process *	grab_core(/* char * adotout, char * core */);
extern int 		grabbed(/* Process * */);
extern int		destroy_process(/* Process * */);
extern int		release_process(/* Process *, int run = 1 */);

extern int		inform_processes();

extern int		run(/* Process *, int, unsigned long */);
extern int		stmt_step(/* Process *, int */);
extern int		instr_step(/* Process *, int */);
extern int		stmt_step_over(/* Process *, int */);
extern int		instr_step_over(/* Process *, int */);
extern int		resume(/* Process * */);

extern int		evaluate_expr(/* Process *, char * expr */);
extern int		evaluate_addr(/* Process *, char * expr */);

extern Assoccmds *	new_assoc();
extern int		add_assoc(/* Assoccmds *, char * */);

extern int		print_map(/*Process * */);

extern int		print_stack(/* Process *, int howmany, int quick = 0 */);

extern int		printregs(/* Process *, int  */);
extern int		get_addr(/* Process *, Location *, Iaddr * */);
#endif


enum LDiscrim	{	lk_addr,
			lk_addrrange,
			lk_stmt,
			lk_stmtrange,
			lk_fcn,
			lk_pc,
		};
#if !defined(__cplusplus)
typedef enum LDiscrim LDiscrim;
#endif

struct Location {
	LDiscrim				kind;
	union {
		unsigned long			addr;	/* lk_addr */
		struct {				/* lk_addrrange */
			unsigned long		loaddr;
			unsigned long		hiaddr;
		} addrrange;
		long				line;	/* lk_stmt */
		struct {				/* lk_stmtrange */
			long			loline;
			long			hiline;
		} stmtrange;
	} locn;
	char *					file_name;
	char *					fcn_name;
};

#if defined(__cplusplus) || defined(__STDC__)
extern int		set_path( char * );
extern int		set_break( Process *, Location *, Assoccmds *, int );
extern int		remove_break( Process *, Location * );
extern int		remove_all_breaks( Process *, int );
extern int		display_breaks( Process * );
extern int		set_watch( Process *, char * expr );
extern int		remove_watch( Process *, char * expr );
extern int		remove_all_watch( Process *, int );
extern int		set_signal_set( Process *, sigset_t );

extern int		disassemble(Process *, Location *, int, unsigned long*);

extern int		print_source( Process *, Location );

extern SrcFile *	find_srcfile( char * );
extern char *		current_src( Process *, long * );
extern int		set_current_src( Process *, char *, long );
extern char *		find_function( Process *, char *, long * );

extern char *		filename( SrcFile * );
extern long		num_lines( SrcFile * );
extern char *		src_text( SrcFile *, long );
extern long		first_exec_line( Process * );
extern int		change_pc( Process *, Location * );
extern int		suffix_path( char * );
extern int		pc_current_src( Process * );

#if defined(__cplusplus)
extern int		get_addr( Process *, Location *, Iaddr & );
#else
extern int		get_addr( Process *, Location *, Iaddr * );
#endif

#else	/* old C */

extern int		set_path(/* char * */);
extern int		set_break(/* Process *, Location */);
extern int		display_breaks(/* Process * */);
extern int		remove_break(/* Process *, Location */);
extern int		remove_all_breaks(/* Process *, int */);
extern int		set_watch(/* Process *, char * expr */);
extern int		remove_watch(/* Process *, char * expr */);
extern int		remove_all_watch(/* Process *, int */);
extern int		set_sigset(/* Process *, sigset_t */);

extern int		disassemble();

extern int		print_source(/* Process *, Location */);

extern SrcFile *	find_srcfile(/* char * */);
extern char *		current_src(/* Process *, long * */);
extern int		set_current_src(/* Process *, char *, long */);
extern char *		find_function(/* Process *, char *, long * */);

extern char *		filename(/* SrcFile * */);
extern long		num_lines(/* SrcFile * */);
extern char *		src_text(/* SrcFile *, long */);
extern long		first_exec_line(/* Process * */);
extern int		change_pc(/* Process *, Location * */);
extern int		suffix_path(/* char * */);
extern int		pc_current_src(/* Process * */);

#endif

#endif

/* end of utility.h */
