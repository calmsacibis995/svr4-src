/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ifndef	_NETINET_SYMREDEF_H
#define	_NETINET_SYMREDEF_H

#ident	"@(#)kern-inet:symredef.h	1.3"

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
 * Redefine all of the global symbols used in the internet code.
 * If two implementations of the internet code are compiled into
 * one kernel, this header file must be included by the source modules
 * of one of the implementations to avoid duplicate symbol definitions.
 */

#define arpwhohas arpwhohas_s
#define arptimer arptimer_s
#define arptnew arptnew_s
#define arptfree arptfree_s
#define arpresolve arpresolve_s
#define arpioctl arpioctl_s
#define ether_sprintf ether_sprintf_s
#define etherbroadcastaddr etherbroadcastaddr_s
#define icmp_input icmp_input_s
#define icmp_send icmp_send_s
#define icmp_reflect icmp_reflect_s
#define icmp_error icmp_error_s
#define icmpstat icmpstat_s
#define in_arpinput in_arpinput_s
#define in_cksum in_cksum_s
#define in_control in_control_s
#define in_localaddr in_localaddr_s
#define in_broadcast in_broadcast_s
#define in_makeaddr in_makeaddr_s
#define in_ifinit in_ifinit_s
#define in_lnaof in_lnaof_s
#define in_canforward in_canforward_s
#define in_netof in_netof_s
#define in_pcbdisconnect in_pcbdisconnect_s
#define in_pcbbind in_pcbbind_s
#define in_pcbdetach in_pcbdetach_s
#define in_losing in_losing_s
#define in_setsockaddr in_setsockaddr_s
#define in_pcbnotify in_pcbnotify_s
#define in_pcbconnect in_pcbconnect_s
#define in_rtchange in_rtchange_s
#define in_setpeeraddr in_setpeeraddr_s
#define in_pcblookup in_pcblookup_s
#define in_pcballoc in_pcballoc_s
#define inet_netmatch inet_netmatch_s
#define inet_hash inet_hash_s
#define inetctlerrmap inetctlerrmap_s
#define ip_forward ip_forward_s
#define ip_slowtimo ip_slowtimo_s
#define ip_nhops ip_nhops_s
#define ip_srcroute ip_srcroute_s
#define ip_stripoptions ip_stripoptions_s
#define ip_freef ip_freef_s
#define ip_reass ip_reass_s
#define ip_pcbopts ip_pcbopts_s
#define ip_optcopy ip_optcopy_s
#define ip_rtaddr ip_rtaddr_s
#define ip_enq ip_enq_s
#define ip_dooptions ip_dooptions_s
#define ip_drain ip_drain_s
#define ip_insertoptions ip_insertoptions_s
#define ip_output ip_output_s
#define ip_id ip_id_s
#define ip_protox ip_protox_s
#define ipq ipq_s
#define iptime iptime_s
#define ipintr ipintr_s
#define ipstat ipstat_s
#define rawcb rawcb_s
#define rip_input rip_input_s
#define rip_output rip_output_s
#define rip_ctloutput rip_ctloutput_s
#define rthost	rthost_s
#define rtnet	rtnet_s
#define rtinit rtinit_s
#define rthashsize rthashsize_s
#define rtrequest rtrequest_s
#define rtioctl rtioctl_s
#define rtalloc rtalloc_s
#define rtredirect rtredirect_s
#define rtfree rtfree_s
#define save_rte save_rte_s
#define tcb tcb_s
#define tcpstat tcpstat_s
#define tcp_iss tcp_iss_s
#define tcp_keepidle tcp_keepidle_s
#define tcp_attach tcp_attach_s
#define tcp_slowtimo tcp_slowtimo_s
#define tcp_pulloutofband tcp_pulloutofband_s
#define tcp_mss tcp_mss_s
#define tcprexmtthresh tcprexmtthresh_s
#define tcpprintfs tcpprintfs_s
#define tcp_reass tcp_reass_s
#define tcp_dooptions tcp_dooptions_s
#define tcpcksum tcpcksum_s
#define tcp_output tcp_output_s
#define tcp_initopt tcp_initopt_s
#define tcp_setpersist tcp_setsersist_s
#define tcp_outflags tcp_outflags_s
#define tcp_disconnect tcp_disconnect_s
#define tcp_sendspace tcp_sendspace_s
#define tcp_usrclosed tcp_usrclosed_s
#define tcp_ctloutput tcp_ctloutput_s
#define tcp_recvspace tcp_recvspace_s
#define tcp_ctlinput tcp_ctlinput_s
#define tcp_template tcp_template_s
#define tcp_close tcp_close_s
#define tcp_quench tcp_quench_s
#define tcp_newtcpcb tcp_newtcpcb_s
#define tcp_drop tcp_drop_s
#define tcp_ttl tcp_ttl_s
#define tcp_drain tcp_drain_s
#define tcp_respond tcp_respond_s
#define tcp_timers tcp_timers_s
#define tcp_fasttimo tcp_fasttimo_s
#define tcp_keepintvl tcp_keepintvl_s
#define tcp_backoff tcp_backoff_s
#define tcp_canceltimers tcp_canceltimers_s
#define tcp_keepidle tcp_keepidle_s
#define udb udb_s
#define udp_ctlinput udp_ctlinput_s
#define udp_in udp_in_s
#define udp_output udp_output_s
#define udp_input udp_input_s
#define udp_ttl udp_ttl_s
#define udpstat udpstat_s

#endif	_NETINET_SYMREDEF_H
