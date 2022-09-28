/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/



#ident	"@(#)i286emu:sysent.c	1.1"

#include "sysent.h"

/*
 * This table is the switch used to transfer
 * to the appropriate routine for processing a system call.
 */

int nosys(),exit(),Fork(),read(),write(),open(),close(),Wait();
int creat(),link(),unlink(),Exec(),chdir(),time(),mknod(),chmod();
int chown(),Sbreak(),Stat(),lseek(),Getpid(),mount(),umount(),setuid();
int Getuid(),Stime(),Ptrace(),alarm(),Fstat(),pause(),utime(),stty();
int gtty(),access(),nice(),statfs(),sync(),kill(),fstatfs(),Setpgrp();
int dup(),Pipe(),times(),profil(),lock(),setgid(),Getgid();
int Signal(),Msgsys(),Sysi86(),acct(),Shmsys(),Semsys(),Ioctl();
int uadmin(),Utssys(),Exece(),umask(),chroot();
int Fcntl(),ulimit(), plock();
int rmdir(),mkdir(),getdents(),Sysfs();
int getmsg(),putmsg(),poll();

struct sysent sysent[] =
{
      { nosys                                 }, /*  0 = indir */
      { exit,   INT                           }, /*  1 = exit */
      { Fork                                  }, /*  2 = fork */
      { read,   INT, PTR, UINT                }, /*  3 = read */
      { write,  INT, PTR, UINT                }, /*  4 = write */
      { open,   PTR, INT, INT                 }, /*  5 = open */
      { close,  INT                           }, /*  6 = close */
      { Wait                                  }, /*  7 = wait */
      { creat,  PTR, INT                      }, /*  8 = creat */
      { link,   PTR, PTR                      }, /*  9 = link */
      { unlink, PTR                           }, /* 10 = unlink */
      { Exec,   SPECIAL                       }, /* 11 = exec */
      { chdir,  PTR                           }, /* 12 = chdir */
      { time,   ZERO                          }, /* 13 = time */
      { mknod,  PTR, INT, INT                 }, /* 14 = mknod */
      { chmod,  PTR, INT                      }, /* 15 = chmod */
      { chown,  PTR, INT, INT                 }, /* 16 = chown; now 3 args */
      { Sbreak, SPECIAL                       }, /* 17 = break */
      { Stat,   SPECIAL                       }, /* 18 = stat */
      { lseek,  INT, LONG, INT                }, /* 19 = seek */
      { Getpid                                }, /* 20 = getpid */
      { mount,  PTR, PTR, INT                 },  /* 21 = mount */
      { umount, PTR                           },  /* 22 = umount */
      { setuid, INT                           },  /* 23 = setuid */
      { Getuid, SPECIAL                       },  /* 24 = getuid */
      { Stime,  SPECIAL                       },  /* 25 = stime */
      { Ptrace                                },  /* 26 = ptrace */
      { alarm,  UINT                          },  /* 27 = alarm */
      { Fstat,  SPECIAL                       },  /* 28 = fstat */
      { pause                                 },  /* 29 = pause */
      { utime,  PTR, PTR                      },  /* 30 = utime */
      { stty,   INT, PTR                      },  /* 31 = stty */
      { gtty,   INT, PTR                      },  /* 32 = gtty */
      { access, PTR, INT                      },  /* 33 = access */
      { nice,   INT                           },  /* 34 = nice */
      { statfs, PTR, PTR, INT, INT            },  /* 35 = statfs */
      { sync                                  },  /* 36 = sync */
      { kill,   INT, INT                      },  /* 37 = kill */
      { fstatfs, INT, PTR, INT, INT           },  /* 38 = fstatfs */
      { Setpgrp, SPECIAL                      },  /* 39 = setpgrp */
      { nosys                                 },  /* 40 = tell - obsolete */
      { dup,    INT                           },  /* 41 = dup */
      { Pipe                                  },  /* 42 = pipe */
      { times,  PTR                           },  /* 43 = times */
      { profil, PTR, INT, PTR, INT            },  /* 44 = prof */
      { plock,   INT                          },  /* 45 = proc lock */
      { setgid, INT                           },  /* 46 = setgid */
      { Getgid, SPECIAL                       },  /* 47 = getgid */
      { Signal, SPECIAL                       },  /* 48 = signal */
      { Msgsys, SPECIAL                       },  /* 49 = IPC Messages */
      { Sysi86, SPECIAL                       },  /* 50 = sysi86 system calls */
      { acct,   PTR                           },  /* 51 = turn acct off/on */
      { Shmsys, SPECIAL                       },  /* 52 = IPC Shared Memory */
      { Semsys, SPECIAL                       },  /* 53 = IPC Semaphores */
      { Ioctl,  SPECIAL                       },  /* 54 = ioctl */
      { uadmin, INT, INT, INT                 },  /* 55 = uadmin */
      { nosys,                                },  /* 56 = x */
      { Utssys, SPECIAL                       },  /* 57 = utssys */
      { nosys,                                },  /* 58 = reserved for USG */
      { Exece,  SPECIAL                       },  /* 59 = exece */
      { umask,  INT                           },  /* 60 = umask */
      { chroot, PTR                           },  /* 61 = chroot */
      { Fcntl,  SPECIAL                       },  /* 62 = fcntl */
      { ulimit, INT, LONG                     },  /* 63 = ulimit */
      { nosys,                                },
      { nosys,                                },
      { nosys,                                },
      { nosys,                                },
      { nosys,                                },
      { nosys,                                },
      { nosys,                                },
      { nosys,                                },
      { nosys,                                },
      { nosys,                                },
      { nosys,                                },
      { nosys,                                },
      { nosys,                                },
      { nosys,                                },
      { nosys,                                },
      { rmdir,  PTR                           },
      { mkdir,  PTR, INT                      },
      { getdents, INT, PTR, INT               },
      { nosys,                                },
      { nosys,                                },
      { Sysfs,  SPECIAL                       },
      { getmsg, INT, PTR, PTR, PTR            },        /* ? */
      { putmsg, INT, PTR, PTR, INT            },        /* ? */
      { poll,   PTR, LONG, INT                },
};
