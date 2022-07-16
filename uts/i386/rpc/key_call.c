/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-rpc:key_call.c	1.3"
#if !defined(lint) && defined(SCCSIDS)
static char sccsid[] = "@(#)key_call.c 1.5 89/01/13 SMI"
#endif

/*
 *  		PROPRIETARY NOTICE (Combined)
 *  
 *  This source code is unpublished proprietary information
 *  constituting, or derived under license from AT&T's Unix(r) System V.
 *  In addition, portions of such source code were derived from Berkeley
 *  4.3 BSD under license from the Regents of the University of
 *  California.
 *  
 *  
 *  
 *  		Copyright Notice 
 *  
 *  Notice of copyright on this source code product does not indicate 
 *  publication.
 *  
 *  	(c) 1986,1987,1988,1989  Sun Microsystems, Inc.
 *  	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
 *  	          All rights reserved.
 */

/*
 * key_call.c, Interface to keyserver
 * key_encryptsession(agent, deskey) - encrypt a session key to talk to agent
 * key_decryptsession(agent, deskey) - decrypt ditto
 * key_gendes(deskey) - generate a secure des key
 * netname2user(...) - get unix credential for given name (kernel only)
 */
#include <sys/param.h>
#include <sys/types.h>
#include <sys/time.h>
#include <rpc/rpc.h>
#include <rpc/key_prot.h>
#ifdef _KERNEL
#include <sys/user.h>
#include <sys/proc.h>
#include <sys/sysmacros.h>
#include <sys/vnode.h>
#include <sys/uio.h>
#include <sys/debug.h>
#include <sys/utsname.h>
#endif

#define KEY_TIMEOUT	30	/* per-try timeout in seconds */
#define KEY_NRETRY	6	/* number of retries */

static struct timeval keytrytimeout = { KEY_TIMEOUT, 0 };

enum clnt_stat key_call();

enum clnt_stat
key_encryptsession(remotename, deskey)
	char *remotename;
	des_block *deskey;
{
	cryptkeyarg arg;
	cryptkeyres res;
	enum clnt_stat stat;

	RPCLOG(2, "key_encryptsession(%s, ", remotename);
	RPCLOG(2, "%x", *(u_long *)deskey);
	RPCLOG(2, "%x)\n", *(((u_long *)(deskey))+1));

	arg.remotename = remotename;
	arg.deskey = *deskey;
	if ((stat = key_call((u_long)KEY_ENCRYPT, 
		xdr_cryptkeyarg, (char *)&arg, xdr_cryptkeyres,
		(char *)&res)) != RPC_SUCCESS) 
	{
		return (stat);
	}
	if (res.status != KEY_SUCCESS) {
		RPCLOG(1, "key_encryptsession: encrypt status is %d\n", res.status);
		return (RPC_FAILED);	/* XXX */
	}
	*deskey = res.cryptkeyres_u.deskey;
	return (RPC_SUCCESS);
}

enum clnt_stat
key_decryptsession(remotename, deskey)
	char *remotename;
	des_block *deskey;
{
	cryptkeyarg arg;
	cryptkeyres res;
	enum clnt_stat stat;

	RPCLOG(2, "key_decryptsession(%s, ", remotename);
	RPCLOG(2, "%x", *(u_long *)deskey);
	RPCLOG(2, "%x)\n", *(((u_long *)(deskey))+1));

	arg.remotename = remotename;
	arg.deskey = *deskey;
	if ((stat = key_call((u_long)KEY_DECRYPT, 
		xdr_cryptkeyarg, (char *)&arg, xdr_cryptkeyres,
		(char *)&res)) != RPC_SUCCESS)
	{
		return (stat);
	}
	if (res.status != KEY_SUCCESS) {
		RPCLOG(1, "key_decryptsession: decrypt status is %d\n", res.status);
		return (RPC_FAILED);	/* XXX */
	}
	*deskey = res.cryptkeyres_u.deskey;
	return (RPC_SUCCESS);
}

enum clnt_stat
key_gendes(key)
	des_block *key;
{
	return(key_call((u_long)KEY_GEN, xdr_void, (char *)NULL, xdr_des_block,
				(char *)key));
}
 

#ifdef _KERNEL
enum clnt_stat
netname2user(name, uid, gid, len, groups)
	char *name;
	uid_t *uid;
	gid_t *gid;
	int *len;
	int *groups;
{
	struct getcredres res;
	enum clnt_stat stat;

	res.getcredres_u.cred.gids.gids_val = (u_int *) groups;
	if ((stat = key_call((u_long)KEY_GETCRED, xdr_netnamestr, (char *)&name, 
		xdr_getcredres, (char *)&res)) != RPC_SUCCESS)
	{
		RPCLOG(1, "netname2user: timed out?\n", 0);
		return (stat);
	}
	if (res.status != KEY_SUCCESS) {
		return (RPC_FAILED);	/* XXX */
	}
	*uid = res.getcredres_u.cred.uid;
	*gid = res.getcredres_u.cred.gid;
	*len = res.getcredres_u.cred.gids.gids_len;
	return (RPC_SUCCESS);
}
#endif

#ifdef _KERNEL

#define	NC_LOOPBACK		"loopback"	/* XXX */
char	loopback_name[] = NC_LOOPBACK;

STATIC enum clnt_stat
key_call(procn, xdr_args, args, xdr_rslt, rslt)
	u_long				procn;
	bool_t				(*xdr_args)();	
	char				*args;
	bool_t				(*xdr_rslt)();
	char				*rslt;
{
	static struct knetconfig	config; /* avoid lookupname next time*/
	struct netbuf			netaddr;
	CLIENT				*client;
	enum clnt_stat			stat;
	struct vnode			*vp;
	int				error;
	static char			keyname[SYS_NMLN+16];

	strcpy(keyname, utsname.nodename);
	netaddr.len = strlen(keyname);
	strcpy(&keyname[netaddr.len], ".keyserv");

        netaddr.buf = keyname;
	/* 8 = strlen(".keyserv");
	 */
	netaddr.len = netaddr.maxlen = netaddr.len + 8;

        /* filch a knetconfig structure.
         */
	if (config.knc_rdev == 0){
		if ((error = lookupname("/dev/ticlts", UIO_SYSSPACE,
					 FOLLOW, NULLVPP, &vp)) != 0) {
			RPCLOG(1, "key_call: lookupname: %d\n", error);
			return (RPC_UNKNOWNPROTO);
		}
		config.knc_rdev = vp->v_rdev;
		config.knc_protofmly = loopback_name;
		VN_RELE(vp);
	}
	RPCLOG(8, "key_call: procn %d, ", procn);
	RPCLOG(8, "rdev %x, ", config.knc_rdev);
	RPCLOG(8, "len %d, ", netaddr.len);
	RPCLOG(8, "maxlen %d, ", netaddr.maxlen);
	RPCLOG(8, "name %x\n", netaddr.buf);

        /* now call the proper stuff.
         */
        error = clnt_tli_kcreate(&config, &netaddr, (u_long)KEY_PROG,
		(u_long)KEY_VERS, 0, KEY_NRETRY, u.u_procp->p_cred, &client);

	if (error != 0) {
		RPCLOG(1, "key_call: clnt_tli_kcreate: error %d", error);
		switch (error) {
		EINTR:		return (RPC_INTR);
		ETIMEDOUT:	return (RPC_TIMEDOUT);
		default:	return (RPC_FAILED);	/* XXX */
		}
	}
	stat = clnt_call(client, procn, xdr_args, args, xdr_rslt, rslt, 
			 keytrytimeout);
	auth_destroy(client->cl_auth);
	clnt_destroy(client);
	if (stat != RPC_SUCCESS) {
		RPCLOG(1, "key_call: keyserver clnt_call failed: stat %x",stat);
		RPCLOG(1, clnt_sperrno(stat), 0);
		return (stat);
	}
	RPCLOG(8, "key call: (%d) ok\n", procn);
	return (RPC_SUCCESS);
}

#endif
