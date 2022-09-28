/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sccs:hdr/filehand.h	6.2"

/* EMACS_MODES: c !fill tabstop=4 */

/* EMPTY is used as an indicator for an empty string or as a last
 * argument.  It should always be used for such purposes when calling
 * filehand.  Because we only compare to this address, and do not
 * actually use it, it is safe to do this on all machines.  No pointer
 * problems can occur because we are concerned only with a constant
 * address, not its contents. */

# define EMPTY (char*) 0

            /* Operation codes for routine ftrans. */

/* These opcodes will cause an error if file2 exists. */

# define CPY 1			/* Copy file1 to file2. */
# define RENAME 2		/* Give file1 the new name file2. */
# define APPEND 4		/* Append file1 to file2. */

/* These opcodes will destroy file2 (if it exists) and then execute the
 * command. */

# define COPYOVER 8		/* Copy file1 to file2. */
# define MOVE 16		/* Replace file2 with file1. */

           /* Operation codes for routine sweep. */

# define VERIFY 4		/* See if something is in file1. */
# define INSERT 8		/* Insert a new record. */
# define DELETE 16		/* Delete an old record. */
# define REPLACE 32		/* Change an old record. */
# define PUTNOW 128		/* Put immediately into file2. */

# define SEQUENTIAL 64	/* Records are sequential flag. */
# define SEQVERIFY 68	/* These are just the regular operations with */
# define SEQINSERT 72	/* SEQUENTIAL logically anded with them. */
# define SEQDELETE 80	/* As before, EXIT is automatic. */
# define SEQREPLACE 96

						/* fldsep codes. */

/* WHITE is a code used to specify that spaces and tabs seperate
 * fields in records.  It takes the value of NULL, because NULL
 * cannot be used to seperate fields. */

# define WHITE 0

/* Normal return codes.  These codes are returned on normal filehand
 * and ftrans operations. */

# define FOUND 8		/* A record was found. */
# define DONE 10		/* Request was done successfully. */
# define NOTFOUND 13	/* Record was not there. */
# define OK DONE		/* For readrec. */

                        /* Error codes. */

/* Those that are followed by "NK" will cause the KEEP option of opcode
 * to be ignored. */

# define DESTEXISTS 1	/* File2 exists and it shouldn't. */
# define NOSOURCE 2		/* Can't read/find file1. (NK) */
# define NODEST 3		/* Failed to open file2. (NK) */
# define BADSIZE 4		/* Bad maximum record size. (NK) */
# define NOSPACE 6		/* Couldn't allocate enough core. (NK) */
# define BIGREC 7		/* A record bigger than maxlen was encountered. */
# define NOTNEW 9		/* An attempt to overwrite with INSERT. */
# define LIVEDEST 11	/* Couldn't kill destination file. (NK) */
# define ABEND 12		/* Should never happen. */
# define LIVESRC 14		/* File1 was not deleted. */
# define NOTPUT 18		/* Message could not be PUTNOW. */
# define FILE1EOF 19	/* End of file on file1. */
# define COPYERROR 20	/* Error in copying the rest of the file. */
# define SHORTREC 21	/* An incorrect size FIXED record was read. */
# define BADTYPE 22		/* Record type was not FIXED or VARIED. */
# define RESETERR 23	/* NOTFOUND, and could not remove file2. */
# define NOCLOSE 24		/* Could not close a file near end of run. */
# define RECURSE 1024	/* Error occured in ftrans.  Lower bits give error. */

						/* Record type codes */

# define FIXED 1		/* Records are of a fixed size. */
# define VARIED 2		/* Records are of a varied size. */

