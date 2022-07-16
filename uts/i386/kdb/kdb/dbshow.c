/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kdb:kdb/dbshow.c	1.3.1.1"

/*
 * print various things
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/immu.h"
#include "sys/tss.h"
#include "sys/proc.h"
#include "sys/var.h"
#include "sys/vfs.h"
#include "sys/vnode.h"
#include "sys/user.h"
#include "sys/fs/s5inode.h"
#include "sys/fs/snode.h"
#include "sys/procfs.h"
#include "fs/proc/prdata.h"
#include "sys/sysmacros.h"
#include "sys/kdebugger.h"
#include "debugger.h"
#include "../db_as.h"

#define L(X) ((long)(X))

extern int	db_cur_size;

proc_t *db_procp();

char *findsymname();
long findsymaddr();
void dbprintf();
void _debugger();


db_pvfs(_vfsp)
	as_addr_t	_vfsp;
{
	vfs_t		vfs_buf;
	register vfs_t *vfsp = &vfs_buf;
	register u_int	idx;

	if (db_read(_vfsp, (char *)vfsp, sizeof(vfs_t)) == -1) {
		dberror("invalid address");
		return;
	}

	dbprintf( "vfs struct @ %x:\n", L(_vfsp.a_addr) );

	dbprintf( "fstype %-2x: ", L(vfsp->vfs_fstype) );
	if ((idx = vfsp->vfs_fstype) < nfstype)
		dbprintf( "%-8s ", vfssw[idx].vsw_name );
	else
		dbprintf( "         " );

	dbprintf( "op %-8x         mounted on %x\n",
		L(vfsp->vfs_op), L(vfsp->vfs_vnodecovered));

	dbprintf( "flag:    %8x   bsize:   %8d   data:    %8x   dev: %d,%d\n",
		L(vfsp->vfs_flag), L(vfsp->vfs_bsize), L(vfsp->vfs_data),
		getemajor(vfsp->vfs_dev), geteminor(vfsp->vfs_dev) );

	dbprintf( "bcount:  %8x   nsubmounts:  %4x   next:    %8x\n",
		L(vfsp->vfs_bcount), L(vfsp->vfs_nsubmounts), L(vfsp->vfs_next) );
}

static char *vtype_name[] = {
	"NON", "REG", "DIR", "BLK", "CHR", "LNK", "FIFO", "XNAM", "BAD"
};
#define N_VTYPE	(sizeof(vtype_name) / sizeof(char *))

db_pvnode(_vp)
	as_addr_t	_vp;
{
	vnode_t		vnode_buf;
	register vnode_t *vp = &vnode_buf;
	vtype_t		vtype;

	if (db_read(_vp, (char *)vp, sizeof(vnode_t)) == -1) {
		dberror("invalid address");
		return;
	}

	dbprintf( "vnode struct @ %x:\n", L(_vp.a_addr) );

	vtype = vp->v_type;
	dbprintf(
	"type: %-2x - %-8s count:   %8x   vdata:   %8x   flag:    %8x\n",
		L(vtype), (u_int)vtype >= N_VTYPE ? "???" : vtype_name[vtype],
		L(vp->v_count), L(vp->v_data), L(vp->v_flag) );

	dbprintf(
	"vfsp:    %8x   vfsmountedhere:  %8x               stream:  %8x\n",
		L(vp->v_vfsp), L(vp->v_vfsmountedhere), L(vp->v_stream) );

	dbprintf( "pages:   %8x   filocks: %8x   rdev: %d,%d\n",
		L(vp->v_pages), L(vp->v_filocks),
		getemajor(vp->v_rdev), geteminor(vp->v_rdev) );
}


db_pinode(_ip)
	as_addr_t	_ip;
{
	inode_t		inode_buf;
	register inode_t *ip = &inode_buf;
	static inode_t	*Inode;

	if (db_read(_ip, (char *)ip, sizeof(inode_t)) == -1) {
		dberror("invalid address");
		return;
	}

	if (Inode == NULL) {
		long	_Inode;

		if ((_Inode = findsymaddr("inode")) != 0L)
			Inode = *(inode_t **)_Inode;
	}
	if (Inode == NULL || _ip.a_as != AS_KVIRT)
		dbprintf( "s5 inode @ %x:\n", L(_ip.a_addr) );
	else
		dbprintf( "s5 inode %x @ %x:\n", (inode_t *)_ip.a_addr - Inode,
						 L(_ip.a_addr) );

	dbprintf( "flag:    %8x   number:  %8x   dev: %d,%d   rdev: %d,%d\n",
		L(ip->i_flag), L(ip->i_number),
		getemajor(ip->i_dev), geteminor(ip->i_dev),
		getemajor(ip->i_rdev), geteminor(ip->i_rdev) );

	dbprintf(
	"links:   %8x   uid:     %8x   gid:     %8x   size:    %8x\n",
		L(ip->i_nlink), L(ip->i_uid), L(ip->i_gid), L(ip->i_size) );

	dbprintf( "mode:      %06o   gen:     %8x   addr:    %8x\n",
		L(ip->i_mode), L(ip->i_gen),
		L(&((inode_t *)_ip.a_addr)->i_addr[0]) );

	dbprintf( "nilocks:     %4x   owner:       %4x\n",
		L(ip->i_nilocks), L(ip->i_owner) );

	dbprintf( "map:     %8x   mapcnt:  %8x   nextr:   %8x\n",
		L(ip->i_map), L(ip->i_mapcnt), L(ip->i_nextr) );

	_ip.a_addr = (u_long)ITOV((inode_t *)_ip.a_addr);
	db_pvnode(_ip);
}


db_psnode(_sp)
	as_addr_t	_sp;
{
	struct snode	snode_buf;
	register struct snode *sp = &snode_buf;

	if (db_read(_sp, (char *)sp, sizeof(struct snode)) == -1) {
		dberror("invalid address");
		return;
	}

	dbprintf( "snode struct @ %x:\n", L(_sp.a_addr) );

	dbprintf( "flag:    %8x   size:    %8x   fsid:    %8x   dev: %d,%d\n",
		L(sp->s_flag), L(sp->s_size), L(sp->s_fsid),
		getemajor(sp->s_dev), geteminor(sp->s_dev) );

	dbprintf( "realvp:  %8x   commonvp: %8x  nextr:   %8x   count:   %8x\n",
		L(sp->s_realvp), L(sp->s_commonvp),
		L(sp->s_nextr), L(sp->s_count) );

	_sp.a_addr = (u_long)STOV((struct snode *)_sp.a_addr);
	db_pvnode(_sp);
}


db_pprnode(_prp)
	as_addr_t	_prp;
{
	prnode_t	prnode_buf;
	register prnode_t *prp = &prnode_buf;

	if (db_read(_prp, (char *)prp, sizeof(prnode_t)) == -1) {
		dberror("invalid address");
		return;
	}

	dbprintf( "prnode struct @ %x:\n", L(_prp.a_addr) );

	dbprintf( "flags:       %4x   proc:    %8x   mode: %06o\n",
		L(prp->pr_flags), L(prp->pr_proc), L(prp->pr_mode) );
	dbprintf( "opens:   %8x   writers: %8x\n",
		L(prp->pr_opens), L(prp->pr_writers) );

	_prp.a_addr = (u_long)PTOV((prnode_t *)_prp.a_addr);
	db_pvnode(_prp);
}


db_dump(addr, count)
	as_addr_t	addr;		/* address to dump from */
	u_long		count;		/* how many bytes to dump */
{
	paddr_t	from;		/* first address to dump from */
	paddr_t	to;		/* last address to dump from */
	paddr_t	start, end;	/* range of dump (from, to rounded to 16-bytes) */
	u_char	val[4];
	char	row[17], *row_p;
	int	i;
	static void stuffrow();

	from = addr.a_addr;
	to = from + count - 1;
	to -= (to % db_cur_size);
	from -= (from % db_cur_size);

	start = (from & ~15);
	end = (to | 15) - (db_cur_size - 1);

	row[16] = '\0';

	for (i = 0, addr.a_addr = start;;) {
		if (addr.a_addr < from || addr.a_addr > to) {
			dbprintf( "%.*s", 2 * db_cur_size, "........" );
			stuffrow( row + i, "...." );
		} else
		if (db_read(addr, val, db_cur_size) == -1) {
			dbprintf( "%.*s", 2 * db_cur_size, "????????" );
			stuffrow( row + i, "????" );
		} else {
			switch (db_cur_size) {
			case 1:
				dbprintf( "%2x", *(u_char *)val );
				break;
			case 2:
				dbprintf( "%4x", *(u_short *)val );
				break;
			case 4:
				dbprintf( "%8x", *(u_long *)val );
				break;
			}
			stuffrow( row + i, val );
		}
		if ((i += db_cur_size) < 16)
			dbprintf( " " );
		else {
			dbprintf( "    %08x  %s\n", addr.a_addr & ~15, row );
			i = 0;
		}
		if ((addr.a_addr += db_cur_size) > end)
			break;
	}
}

static void
stuffrow(row_p, data)
	char	*row_p;		/* pointer into array for row of ASCII chars */
	u_char	*data;		/* char(s) to stuff in row */
{
	int	n = db_cur_size;

	while (n-- > 0) {
		if (*data < ' ' || *data > 127) {
			*row_p++ = '.';
			++data;
		} else
			*row_p++ = (char)*data++;
	}
}


db_sleeping() {
	register proc_t	*p;
	register u_int	i;

	dbprintf( "proc\twchan\n" );

	for (i = 0; i < v.v_proc; i++) {
		if ((p = pid_entry(i)) != NULL)
			dbprintf( "%x\t%x\n", i, p->p_wchan );
	}
}


struct user *
db_ptou(p, prf)
	proc_t	*p;
	void	(*prf)();
{
 	if (p->p_stat == SZOMB) {
		if (prf)
			(*prf) ("(zombie)	");
		return NULL;
	} else if (!(p->p_flag & SULOAD)) {
		if (prf)
			(*prf) ("(swapped out) ");
		return NULL;
	} else
		return PTOU(p);
}


proc_t *
db_procp(p, show)
	proc_t	*p;
{
	register u_int	prslot;
	struct pid	pid, *pidp;
	as_addr_t	addr;

	if ((u_int)p < v.v_proc) {
		p = pid_entry(prslot = (u_int)p);
	} else {
		if ((int)p == -1) {
			p = u.u_procp;
			if (show)
				dbprintf( "(current) " );
		}
		addr.a_as = AS_KVIRT;
		addr.a_addr = (u_long)&p->p_pidp;
		if (db_read(addr, (char *)&pidp, sizeof(pidp)) == -1)
			prslot = 0;
		else {
			addr.a_addr = (u_long)pidp;
			if (db_read(addr, (char *)&pid, sizeof(pid)) == -1)
				prslot = 0;
			else
				prslot = pid.pid_prslot;
		}
	}
	if (show)
		dbprintf( "proc %x @ %x:  ", prslot, p );
	if (prslot >= v.v_proc)
		p = NULL;
	if (pid_entry(prslot) != p)
		p = NULL;
	if (show && p == NULL)
		dbprintf("no such process\n");

	return p;
}


db_pstack(p, showproc, prf)
	struct proc	*p;
	void		(*prf)();
{
	struct user	*ubp;
	struct tss386	*tss;

	if ((p = db_procp(p, showproc)) == NULL)
		return;

	if (showproc && prf)
		(*prf) ("\n");

	if (p == u.u_procp) {
		db_stacktrace(prf, _debugger);
		return;
	}

	if ((ubp = db_ptou(p, prf)) == NULL) {
		if (prf)
			(*prf) ("\n");
		return;
	}

	db_st_offset = (caddr_t)ubp - (caddr_t)&u;

	tss = (struct tss386 *)((caddr_t)ubp + ((caddr_t)ubp->u_tss - (caddr_t)&u));

	db_st_startfp = tss->t_ebp;
	db_st_startsp = tss->t_esp;
	db_st_startpc = tss->t_eip;
	db_stacktrace(prf, NULL);
}


static void
ps_command(p)
	proc_t	*p;
{
	struct user	*ubp;

	if ((ubp = db_ptou(p, dbprintf)) != NULL)
		dbprintf("%-14.14s", ubp->u_comm);
}

db_ps() {
	register proc_t	*pp;
	register u_int	i;

	dbprintf(
"                  FLAGS S   UID   PID  PPID  C PRI    WCHAN COMMAND\n" );
	for (i = 0; i < v.v_proc; i++) {
		if ((pp = pid_entry(i)) != NULL) {
			dbprintf( "%3x @ %8x", i, pp );
			dbprintf( " %8x", pp->p_flag );
			switch ( pp->p_stat ) {
				case SSLEEP:    dbprintf( " S" ); break;
				case SRUN:      dbprintf( " R" ); break;
				case SZOMB:     dbprintf( " Z" ); break;
				case SSTOP:     dbprintf( " T" ); break;
				case SIDL:      dbprintf( " I" ); break;
				case SONPROC:   dbprintf( " O" );
						break;
				case SXBRK:     dbprintf( " X" ); break;
				default:        dbprintf( " ?" ); break;
			}
			if (pp == curproc)
				dbprintf("*");
			else
				dbprintf(" ");
			dbprintf( "%5d", pp->p_uid );
			if (pp->p_pidp)
				dbprintf( " %5d", pp->p_pid );
			else
				dbprintf( "   ???" );
			dbprintf( " %5d", pp->p_ppid );
			dbprintf( " %2x", pp->p_cpu );
			dbprintf( " %3d", pp->p_pri );
			if ( pp->p_stat != SRUN && pp->p_stat != SONPROC )
				dbprintf( " %8x ", (long)pp->p_wchan );
			else
				dbprintf( "          " );
			ps_command(pp);
			dbprintf( "\n" );
		}
	}
}
