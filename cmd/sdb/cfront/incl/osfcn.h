/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:cfront/incl/osfcn.h	1.1"
/*ident	"@(#)cfront:incl/osfcn.h	1.7"*/

#ifndef OSFCNH
#define OSFCNH

extern void    _exit (int);
extern int      access (const char*, int);
extern int      acct (const char*);
extern unsigned alarm (unsigned);
extern int      brk (char*);
extern int      chdir (const char*);
extern int      chmod (const char*, int);
extern int      chown (const char*, int, int);
extern int      chroot (const char*);
extern int      close (int);
extern int      creat (const char*, int);
extern int      dup (int);
extern int      execl (const char*, const char* ...);
extern int      execle (const char*, const char* ...);
extern int      execlp (const char*, const char* ...);
extern int      execv (const char*, const char**);
extern int      execve (const char*, const char**, char**);
extern int      execvp (const char*, const char**);
extern void     exit (int);
extern int      fork ();
extern char*    getcwd (char*, int);
extern unsigned short getegid ();
extern unsigned short geteuid ();
extern unsigned short getgid ();
extern unsigned short getuid ();
extern int      getpgrp ();
extern int      getpid ();
extern int      getppid ();
extern int      ioctl(int, int ...);
extern int      kill (int, int);
extern int      link (const char*, const char*);
extern long     lseek (int, long, int);
extern int      mknod (const char*, int, int);
extern int      mount (const char*, const char*, int, int);
extern int      nice (int);
extern int      open (const char*, int, ...);
extern void     pause ();
extern int      pipe  (int*);
extern void     profil (char*, int, int, int);
extern int      ptrace (int, int, int, int);
extern int      read (int, char*, unsigned);
extern char*    sbrk (int);
extern int      setgid (int);
extern int      setpgrp ();
extern int      setuid (int);
extern unsigned sleep (unsigned);
extern int      stime (long*);
extern void     sync ();
extern void     sys3b (int ...);
extern int      system (const char*);
extern long     ulimit (int, long);
extern int      umask (int);
extern int      umount (const char*);
extern int      unlink (const char*);
extern int      wait (int*);
extern int      write (int, const char*, unsigned);

#endif
