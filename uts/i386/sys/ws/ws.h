/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head.sys:sys/ws/ws.h	1.1.2.1"

#ifndef	_SYS_WS_WS_H
#define	_SYS_WS_WS_H

/*
 * Definitions for the IWE.
 */

#ifdef _KERNEL
struct map_info {
	int	m_cnt;	/* count of the number of memory locations mapped */
	struct proc	*m_procp;	/* process with display mapped */
	pid_t	m_pid;
	int m_chan; /* channel that owns the map currently */
	struct kd_memloc	m_addr[10]; /* display mapping info */
};

#define	T_ANSIMVBASE	0x00001
#define	T_BACKBRITE	0x00002

typedef struct {
	int	t_flags;	/* terminal state flags -- see above */
	unchar	t_font,		/* selected font */
		t_curattr,	/* current attribute */
		t_normattr,	/* normal character attribute */
		t_undstate;	/* underline state */
	ushort	t_rows,		/* number of characters vertically */
		t_cols,		/* number of characters horizontally */
		t_scrsz,	/* number of characters (ch_cols * ch_rows) */
		t_origin,	/* upper left corner of screen in buffer */
		t_cursor,	/* cursor position (0-based) */
		t_curtyp,	/* cursor type 1 == block, 0 == underline */
		t_row,		/* current row */
		t_col,		/* current column */
		t_sending,	/* sending screen */
		t_sentrows,	/* rows sent */
		t_sentcols;	/* cols sent */
	unchar	t_pstate,	/* parameter parsing state */
		t_ppres;	/* does output ESC sequence have a param */
	ushort	t_pcurr,	/* value of current param */
		t_pnum,		/* current param # of ESC sequence */
		t_ppar[5];	/* parameters of ESC sequence */
	struct attrmask
		*t_attrmskp;	/* pointer to attribute mask array */
	unchar	t_nattrmsk,	/* size of attribute mask array */
		t_ntabs,	/* number of tab stops set */
		*t_tabsp;	/* list of tab stops */
} termstate_t;

typedef struct {
	unchar	v_cmos,		/* cmos video controller value */
		v_type,		/* video controller type */
		v_cvmode,	/* current video mode */
		v_dvmode,	/* default video mode */
		v_font,		/* current font loaded */
		v_colsel,	/* color select register byte */
		v_modesel,	/* mode register byte */
		v_undattr,	/* underline attribute */
		v_uline,	/* underline status (on or off) */
		v_nfonts,	/* number of fonts */
		v_border;	/* border attribute */
	ushort	v_scrmsk,	/* mask for placing text in screen memory */
		v_regaddr;	/* address of corresponding M6845 or EGA */
	unchar	**v_parampp;	/* pointer to video parameters table */
	struct font_info
		*v_fontp;	/* pointer to font information */
	caddr_t	v_rscr;		/* "real" address of screen memory */
	ushort	*v_scrp;	/* pointer to video memory */
	int	v_modecnt;	/* number of modes supported */
	struct modeinfo
		*v_modesp;	/* pointer to video mode information table */
	ushort	v_ioaddrs[MKDIOADDR];	/* valid I/O addresses */
} vidstate_t;

typedef struct {
	unchar	kb_sysrq,	/* true if last character was K_SRQ */
		kb_srqscan,	/* scan code of K_SRQ */
		kb_lasthot,	/* last hot-key character */
		kb_prevscan;	/* previous scancode */
	ushort	kb_state,	/* keyboard shift/ctrl/alt state */
		kb_sstate,	/* saved keyboard shift/ctrl/alt state */
		kb_togls;	/* caps/num/scroll lock toggles state */
	int	kb_extkey,	/* extended key enable state */
		kb_altseq;	/* used to build extended codes */
} kbstate_t;

typedef unchar	extkeys_t[NUM_KEYS+1][NUMEXTSTATES];
typedef unchar	esctbl_t[ESCTBLSIZ][2];

struct pfxstate {
	unchar val;
	unchar type;
};

typedef struct pfxstate pfxstate_t[K_PFXL - K_PFXF + 1];

typedef struct charmap {
	keymap_t
		*cr_keymap_p;	/* scancode to character set mapping */
	extkeys_t
		*cr_extkeyp;	/* extended code mapping */
	esctbl_t
		*cr_esctblp;	/* 0xe0 prefixed scan code mapping */
	strmap_t
		*cr_strbufp;	/* function key mapping */
	srqtab_t
		*cr_srqtabp;	/* sysrq key mapping */
	stridx_t
		*cr_strmap_p;	/* string buffer */
	pfxstate_t
		*cr_pfxstrp;
	struct charmap
		*cr_defltp;	/* pointer to default information for ws */
} charmap_t;


typedef struct scrn {
	scrnmap_t	*scr_map_p;
	struct scrn	*scr_defltp;
} scrn_t;


struct channel_info {
	queue_t	*ch_qp;		/* channel read queue pointer */
	int	ch_opencnt,	/* number of opens */
		ch_closing,	/* indicates the channel is closing */
		ch_id,		/* channel id number */
		ch_slpaddr,	/* address to sleep on */
		ch_flags;	/* channel flags */
	struct proc
		*ch_procp;
	int	ch_pid,
		ch_timeid;	/* timeout id for channel switching */
	short	ch_relsig,	/* release signal */
		ch_acqsig,	/* acquire signal */
		ch_frsig;	/* free signal */
	unchar	ch_dmode;	/* current display mode */
	struct wstation 
		*ch_wsp;
	kbstate_t
		ch_kbstate;	/* keyboard state information */
	charmap_t
		*ch_charmap_p;	/* character mapping tables */
	scrn_t
		ch_scrn;
	vidstate_t
		ch_vstate;	/* channel video state information */
	termstate_t
		ch_tstate;	/* terminal state structure for this channel */
	struct strtty
		ch_strtty;
	xqInfo	ch_xque;
	struct channel_info
		*ch_nextp,	/* next channel in linked list */
		*ch_prevp;	/* previous channel in linked list */
#ifdef MERGE386
	struct mcon *ch_merge;	/* pointer to merge console structure */
#endif
};

typedef struct channel_info	channel_t;

typedef struct wstation {
	int	w_init,		/* workstation has been initialized */
		w_intr,		/* indicates interrupt processing */
		w_active,	/* active channel */
		w_nchan,	/* number of channels */
		w_ticks,	/* used for BELL functionality */
		w_tone,
		w_flags,
		w_noacquire,
		w_wsid,
		w_forcechan,
		w_forcetimeid,
		w_lkstate,
		w_clkstate;
	unchar	w_kbtype,
		w_dmode;
	queue_t	*w_qp;		/* pointer to queue for this minor device */
	mblk_t	*w_mp;		/* pointer to current message block */
	int	w_timeid;	/* id for pending timeouts */
	caddr_t	w_private;	/* used for any workstation specific info */
	channel_t
		**w_chanpp,
		*w_switchto;
	ushort	**w_scrbufpp;
	scrn_t	w_scrn;
	vidstate_t
		w_vstate;	/* workstation video state information */
	termstate_t
		w_tstate;
	struct map_info
		w_map;
	charmap_t
		w_charmap;	/* default charmap for workstation */
	int	(*w_stchar)(),
		(*w_clrscr)(),
		(*w_setbase)(),
		(*w_activate)(),
		(*w_setcursor)(),
		(*w_bell)(),
		(*w_shiftset)(),
		(*w_mvword)(),
		(*w_undattr)(), 
		(*w_rel_refuse)(),
		(*w_acq_refuse)(),
		(*w_scrllck)(),
		(*w_cursortype)(),
		(*w_unmapdisp)();
} wstation_t;

#define	WSCMODE(x)	((struct modeinfo *)x->v_modesp + x->v_cvmode)
#define WSMODE(x, n)	((struct modeinfo *)x->v_modesp + n)
#define WSNTIM	-1

#define HOTKEY	0x10000

#define	WS_NOMODESW	0x01
#define WS_NOCHANSW	0x02
#define WS_LOCKED	0x04
#define WS_KEYCLICK	0x08

#define CHN_UMAP	0x001
#define CHN_XMAP	0x002
#define CHN_QRSV	0x004
#define CHN_ACTV	0x008
#define CHN_PROC	0x010
#define CHN_WAIT	0x020
#define CHN_HIDN	0x040
#define CHN_WACT	0x080
#define CHN_KILLED	0x100

#define CHN_MAPPED	(CHN_UMAP | CHN_XMAP)

#define CHNFLAG(x, y)	(x->ch_flags & y)
#endif /* _KERNEL */

#define	WS_MAXCHAN	15

#endif	/* _SYS_WS_WS_H */
