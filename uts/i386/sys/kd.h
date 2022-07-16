/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head.sys:sys/kd.h	1.1.3.1"

#ifndef	_SYS_KD_H
#define	_SYS_KD_H

/*
 * Types of displays supported
 */
#define	KD_MONO		01	/* monochrome display adapter */
#define	KD_HERCULES	02	/* Hercules monochrome graphics adapter */
#define	KD_CGA		03	/* color graphics adapter */
#define	KD_EGA		04	/* enhanced graphics adapter */
#define KD_VGA		05	/* video graphics array */
#define KD_VDC400	06	/* VDC 400 adapter */
#define KD_VDC750	07	/* VDC 750 adapter */
#define KD_VDC600	010	/* VDC 600 adapter */

#ifdef EVGA

#define KD_EVGA		011	/* extended vga. Used for Vdc.v_info.cntlr */	

/*
 * Specific evga card types
 */

#define EVGA_VGA	0	
#define EVGA_VEGA	1
#define EVGA_STBGA	2
#define EVGA_SIGMAH	3
#define EVGA_PVGA1A	4
#define EVGA_DELL	5
#define EVGA_VRAM	6
#define EVGA_ORVGA	7
#define EVGA_ORVGAni	8
#define EVGA_TVGA	9
#define EVGA_TVGAni	10
#define EVGA_GVGA	11
#define EVGA_EGA	12
#define EVGA_PEGA	13
#define EVGA_GEGA	14
#define EVGA_FASTWRITE	15
#define EVGA_WON	16
#define EVGA_PVGA1024	17

#endif	/*EVGA*/

/*
 * Types of monitors supported
 */

#define KD_UNKNOWN	0
#define KD_STAND_M	1	/* standard monochrome monitor */
#define KD_STAND_C	2	/* standard color monitor */
#define KD_MULTI_M	3	/* multi-mode monochrome monitor */
#define KD_MULTI_C	4	/* multi-mode color monitor */

/* Xenix display/adapter definitions */
#define MONO	1		/* Xenix value for monochrome */
#define CGA	2		/* Xenix value for color graphics */
#define PGA	3		/* Xenix value for professional graphics */
#define EGA	4		/* Xenix value for enhanced graphics */
#define VGA	5		/* Xenix value for Video Graphics Array */
#define	S_ADAP	100		/* special adapter which has no definition */

/*
 * Virtual display constants
 */
#define	KD_WIDTH	80		/* normal screen width */
#define	KD_NARROWWIDTH	40		/* narrow screen width */
#define	KD_EXTRAWIDTH	132		/* wide screen width */
#define	KD_HEIGHT	25		/* screen height */
#define	KD_TALLHEIGHT	43		/* tall screen height */
#define	KD_MAXSCRSIZE	(KD_EXTRAWIDTH * KD_TALLHEIGHT)	/* max screen size */
#define	KD_SCRTOBUF	0		/* transfer from screen to buffer */
#define	KD_BUFTOSCR	1		/* transfer from buffer to screen */

/*
 * keyboard controller I/O port addresses
 */
#define KB_OUT	0x60		/* output buffer R/O */
#define KB_IDAT 0x60		/* input buffer data write W/O */
#define KB_STAT 0x64		/* keyboard controller status R/O */
#define KB_ICMD 0x64		/* input buffer command write W/O */

/*
 * keyboard controller commands and flags
 */
#define KB_INBF		0x02	/* input buffer full flag */
#define KB_OUTBF	0x01	/* output buffer full flag */
#define KB_GATE20	0x02	/* set this bit to allow addresses > 1Mb */
#define KB_ROP		0xD0	/* read output port command */
#define KB_WOP		0xD1	/* write output port command */
#define	KB_RCB		0x20	/* read command byte command */
#define	KB_WCB		0x60	/* write command byte command */
#define	KB_ENAB		0xae	/* enable keyboard interface */
#define	KB_DISAB	0x10	/* disable keyboard */
#define	KB_EOBFI	0x01	/* enable interrupt on output buffer full */
#define KB_ACK		0xFA	/* Acknowledgement byte from keyboard */
#define	KB_RESETCPU	0xFE	/* command to reset AT386 cpu */
#define KB_READID	0xF2	/* command to read keyboard id */
#define	KB_RESEND	0xFE	/* response from keyboard to resend data */
#define	KB_ERROR	0xFF	/* response from keyboard to resend data */
#define	KB_RESET	0xFF	/* command to reset keyboard */
#define	KB_ENABLE	0xF4	/* command to to enable keyboard
				 * this is different from KB_ENAB above in
				 * that KB_ENAB is a command to the 8042 to
				 * enable the keyboard interface, not the
				 * keyboard itself */

/*
 * Structure of keyboard translation table
 */
#define NUM_KEYS	256		/* Maximum number of keys */
#define NUM_STATES	8		/* Number of key states */
#pragma pack(2)
typedef struct {
	short  n_keys ;			/* Number of entries in table */
	struct key_t {
		unsigned char map[NUM_STATES];	/* Key code for each state */
		unsigned char spcl;	/* Bits marking states as special */
		unsigned char flgs;	/* Flags */
	} key[NUM_KEYS+1];		/* One entry for each key */
} keymap_t;
#pragma pack()
/*
 * Keyboard translation modes
 */
#define	K_RAW		0x00		/* Just send raw up/down scan codes */
#define	K_XLATE		0x01		/* Translate scan codes to ASCII */
/*
 * character table flag bits.
 */
#define	NUMLCK		0x8000	/* flag key as affected by num lock key */
#define	CAPLCK		0x4000	/* flag key as affected by caps lock key */
#define	CTLKEY		0x2000	/* flag key as afected by control keys */
/*
 * character table key types
 */
#define	NORMKEY		0x0000	/* key is a normal key, send with no prefix */
#define	SHIFTKEY	0x0100	/* key is a shift key */
#define	BREAKKEY	0x0200	/* key is a break key */
#define	SS2PFX		0x0300	/* prefix key with <ESC> N */
#define	SS3PFX		0x0400	/* prefix key with <ESC> O */
#define	CSIPFX		0x0500	/* prefix key with <ESC> [ */
#define	SPECIALKEY	0x0600	/* special key type */
#define	NOKEY		0x0f00	/* flag a key as nonexistant */
#define	TYPEMASK	0x0f00	/* select key type bits */


/*
 * Table selectors for KDSKBENT/KDGKBENT ioctl's
 */
#define	K_NORMTAB	0x00		/* Select unaugmented keyboard table */
#define	K_SHIFTTAB	0x01		/* Select shifted keyboard table */
#define	K_ALTTAB	0x02		/* Select alted keyboard table */
#define	K_ALTSHIFTTAB	0x03		/* Select alt-shifted keyboard table */
#define	K_SRQTAB	0x04		/* select sysreq table */
/*
 * Make/break distinctions
 */
#define	KBD_BREAK	0x80		/* Key make/break bit (break=1) */
/*
 * Flags for key state calculation.
 */
#define SHIFTED	0x01			/* Keys are shifted */
#define CTRLED	0x02			/* Keys are ctrl'd */
#define ALTED 	0x04			/* Keys are alt'd */
/*
 * Possible key states.  Used as index into translation entry for key.
 */
#define NORMAL		0		/* Unmodified */
#define SHIFT		SHIFTED		/* Shift key depressed */
#define CNTRL		CTRLED		/* Ctrl key depressed */
#define	SHFCTL		(CTRLED|SHIFTED)	/* Shift and ctrl keys */
#define ALT		ALTED		/* Alt key depressed */
#define	ALTSHF		(ALTED|SHIFTED)	/* Shift and alt keys depressed */
#define	ALTCTL		(ALTED|CTRLED)	/* Ctrl and alt keys depressed */
#define	ALTSHFCTL	(ALTED|CTRLED|SHIFTED)	/* Shift, ctrl, and alt keys */
/*
 * Key map table flags
 */
#define	KMF_CLOCK	0x01		/* Key affected by caps lock */
#define	KMF_NLOCK	0x02		/* Key affected by num lock */
/*
 * kb_state bit definitions
 */
#define LEFT_SHIFT	0x0001	/* left shift key depressed */
#define	LEFT_ALT	0x0002	/* left alt key depressed */
#define	LEFT_CTRL	0x0004	/* left control key depressed */
#define	RIGHT_SHIFT	0x0008	/* right shift key depressed */
#define	RIGHT_ALT	0x0010	/* right alt key depressed */
#define	RIGHT_CTRL	0x0020	/* right control key depressed */
#define	CAPS_LOCK	0x0040	/* caps lock key down */
#define	NUM_LOCK	0x0080	/* num lock key down */
#define	SCROLL_LOCK	0x0100	/* scroll lock key down */
#define	ALTSET		(LEFT_ALT|RIGHT_ALT)
#define	SHIFTSET	(LEFT_SHIFT|RIGHT_SHIFT)
#define	CTRLSET		(LEFT_CTRL|RIGHT_CTRL)
#define	NONTOGGLES	(ALTSET|SHIFTSET|CTRLSET)
/*
 * Keyboard scan codes
 */
#define	SCROLLLOCK	0x46		/* Scroll lock key */
/*
 * Number of entries in 0xe0 prefix translation table
 */
#define	ESCTBLSIZ	18		/* Entries in 101/102 key table */
/*
 * Character flags.  Should not conflict with FRERROR and friends in tty.h
 */
#define NO_CHAR		0x8000		/* Do not generate a char */
#define GEN_ESCLSB	0x0800		/* Generate <ESC> [ prefix to char */
#define GEN_ESCN	0x0400		/* Generate <ESC> N prefix to char */
#define GEN_ZERO	0x0200		/* Generate 0 prefix to char */
#define	GEN_FUNC	0x0100		/* Generate function key */
#define GEN_ESCO	0x1000		/* Generate <ESC> O prefix to char */
/*
 * Special key code definitions
 */
#define	K_NOP	0			/* Keys with no function */
#define K_LSH	2			/* Left shift */
#define K_RSH	3			/* Right shift */
#define K_CLK	4			/* Caps lock */
#define K_NLK	5			/* Num lock */
#define K_SLK	6			/* Scroll lock */
#define K_ALT	7			/* Alt */
#define	K_BTAB	8			/* Back tab */
#define K_CTL	9			/* Control */
#define	K_LAL	10			/* Left alt */
#define	K_RAL	11			/* Right alt */
#define	K_LCT	12			/* Left control */
#define	K_RCT	13			/* Right control */
#define K_AGR	14			/* ALT-GR key  -- 102 keyboard only */
#define	K_FUNF	27			/* First function key */
#define	K_FUNL	122			/* Last function key */
#define	K_SRQ	123			/* System request */
#define	K_BRK	124			/* Break */
#define	K_ESN	125			/* <ESC> N <unalt'd value> sequence */
#define	K_ESO	126
#define	K_ESL	127
#define	K_RBT	128			/* Reboot system */
#define	K_DBG	129			/* Invoke debugger */
#define	K_NEXT	130
#define	K_PREV	131
#define	K_FRCNEXT 132
#define	K_FRCPREV 133
#define K_VTF	134
#define	K_VTL	148
#define K_MGRF	149
#define	K_MGRL	191
#define	K_PFXF	192
#define	K_PFXL	255



/*
 * Macro for recognizing scan codes for special keys
 */
#define	IS_SPECIAL(s,i)	(kdkeymap.key[(s)].spcl & (0x80>>(i)))	/* Special? */
#define	IS_SPECKEY(k,s,i)	((k)->key[(s)].spcl & (0x80>>(i)))	/* Special? */
#define	IS_NPKEY(s)	(((s) >= 71) && ((s) <=83))	/* Number pad? */
/*
 * Function key constants and macros
 */
#define	NSTRKEYS	(K_FUNL-K_FUNF+1)	/* Number of string keys */
#define STRTABLN	512		/* Max length of sum of all strings */
#define STRTABLN_21	256		/* Max length of sum in version 2.1 */
#define	IS_FUNKEY(c)	(((int)(c) >= K_FUNF) && ((int)(c) <= K_FUNL) || ((int)(c) >= K_PFXF) && ((int)(c) <= K_PFXL))	/* Function? */
typedef unchar strmap_t[STRTABLN];	/* String map table type */
typedef ushort stridx_t[NSTRKEYS + 1];	/* String map index type */

typedef unsigned char	srqtab_t[NUM_KEYS + 1];

/* Shorthand for constants so columns line up neatly */
#define	KF	K_FUNF			/* First function key */
#define	L_O	0			/* Key not affected by locks */
#define	L_C	KMF_CLOCK		/* Key affected by caps lock */
#define	L_N	KMF_NLOCK		/* Key affected by num lock */
#define	L_B	(KMF_CLOCK|KMF_NLOCK)	/* Key affected by caps and num lock */

/*
 * Structure used for SETFKEY and GETFKEY ioctls
 */
#define MAXFK	30			/* Maximum length of string */
#pragma pack(2)
struct fkeyarg {
	unsigned short	keynum;		/* Function key number */
	unchar	 	keydef[MAXFK];	/* Key definition */
	char		flen;		/* Length of key definition */
};
#pragma pack()
/*
 * Constants for extended keys
 */
#define	NUMEXTSTATES	4		/* Number of extended states */

/* Commands for LED and typematic start/rate on the AT */
#define	KDCQMAX		8	/* length of keyboard command queue */
#define LED_WARN	0xED	/* Tell kbd that following byte is led status */
#define LED_SCR		0x01	/* Flag bit for scroll lock */
#define LED_CAP		0x04	/* Flag bit for cap lock */
#define LED_NUM		0x02	/* Flag bit for num lock */
#define	ACK_WAIT	0x01	/* flag for waiting for kbd response */
#define	TYPE_WARN	0xF3	/* command--next byte is typematic values */
#define	TYPE_VALS	0x20	/* max speed (30/s) and 1/2 sec delay */
#define	SCAN_WARN	0xF0	/* kbd command to set scan code set */


/*
 * Screen mapping constants and types
 */
#define NUM_ASCII	256		/* Number of ASCII characters */
typedef unsigned char scrnmap_t[NUM_ASCII];	/* Screen map type */
typedef unsigned char *scrnmapp_t;	/* Pointer to screen map type */


/*
 * defines for KDDFLTKEYMAP, KDDFLTSTRMAP and KDDFLTSCRNMAP ioctls
 */
#define	KD_DFLTSET	1
#define	KD_DFLTGET	2

/*
 * structure for KDDFLTSTRMAP ioctl 
 */
struct str_dflt {
	int	str_direction;
	strmap_t	str_map;
};

/*
 * structure for KDDFLTSCRNMAP ioctl 
 */
struct scrn_dflt {
	int	scrn_direction;
	scrnmap_t	scrn_map;
};

/*
 * structure for KDDFLTKEYMAP ioctl 
 */
struct key_dflt {
	int	key_direction;
	keymap_t	key_map;
};


/*
 * defines and structures for WS_PIO_ROMFONT ioctl
 */

#define F8x8_SIZE	2048
#define F8x14_SIZE	3072
#define F8x16_SIZE	4096

#define	F8x8_BPC	8
#define	F8x14_BPC	14
#define	F8x16_BPC	16
#define	F9x16_BPC	16

#define	MAX_ROM_CHAR	256	/* Current EGA/VGA support only 256 chars */

struct char_def {
	int cd_index;
	unchar cd_map_8x8[F8x8_BPC];
	unchar cd_map_8x14[F8x14_BPC];
	unchar cd_map_8x16[F8x16_BPC];
	unchar cd_map_9x16[F9x16_BPC];
};

typedef struct rom_font_arg {
	unsigned int fnt_numchar;
	struct char_def	fnt_chars[MAX_ROM_CHAR];
} rom_font_t;

/*
 * type of adapter installed, matches bits in CMOS ram
 */
#define	MCAP_UNK	0xff	/* adapter not determined yet */
#define MCAP_MONO	0x03	/* mono adapter installed */
#define MCAP_COLOR	0x02	/* color adapter installed in 80 column mode */
#define MCAP_COLOR40	0x01	/* color adapter installed in 40 column mode */
#define MCAP_EGA	0x00	/* EGA adapter installed */

/* 6845 base addresses */
#define	MONO_REGBASE	0x03b4	/* Base register address for mono modes */
#define	COLOR_REGBASE	0x03d4	/* Base register address for color modes */

/* offsets from 6845 base address for various registers */
#define DATA_REG	0x1
#define MODE_REG	0x4
#define MODE2_REG	0x0A	/* rite-vu card only */
#define COLOR_REG	0x5	/* color adapter only */
#define STATUS_REG	0x6

/* definitions for bits in the color adapter mode register */
#define M_ALPHA40	0x00	/* 40 by 25 alphanumeric */
#define M_ALPHA80	0x01	/* 80 by 25 alphanumeric */
#define M_GRAPH		0x02	/* 320x200 or 640x200 graphics */
#define M_BW		0x04	/* black & white */
#define M_ENABLE	0x08	/* video enable */
#define M_HIGHRES	0x10	/* 640x200 B&W graphics */
#define M_BLINK		0x20	/* enable blink attribute */

/* definitions for bits in the color adapter mode2 register ( rite-vu card )*/
#define	M_GRAPH2	0x01	/* 640x400 graphics AT&T mode */
#define	M_DEGAUSS	0x02	/* degauss color monitor */
#define	M_ALTCHAR	0x04	/* rite-vu card alternate character set */
#define	M_PGSEL		0x08	/* switch to 2nd 16kbytes of memory */
#define	M_UNDRLINE	0x40	/* underline white chars, not blue */


/* definitions for bits in the color adapter status register */
#define S_UPDATEOK	0x01	/* safe to update regen buffer */
#define S_VSYNC		0x08	/* raster is in vertical retrace mode */

/* definitions for loading data into CRT controller registers */
#define R_STARTADRH	12	/* start address, high word */
#define R_STARTADRL	13	/* start address, low word */
#define R_CURADRH	14	/* cursor address, high word */
#define R_CURADRL	15	/* cursor address, low word */

/* definitions for the EGA */
#define IN_STAT_0	0x3c2	/* input status register zero */
#define MISC_OUT	0x3c2	/* miscellaneous output register */
#define IN_STAT_1	6	/* offset of input status register 1 */
#define FEAT_CTRL	6	/* offset of feature control register */
#define SW_SENSE	0x10	/* switch sense bit in input status zero */
#define CLKSEL		2	/* shift to select switch number */
#define GRAPH_1_POS	0x3cc	/* graphics 1 position */
#define GRAPH_2_POS	0x3ca	/* graphics 2 position */
#define GRAPHICS1	0	/* value for graphics 1 */
#define GRAPHICS2	1	/* value for graphics 2 */
#define SEQ_RST		0x01	/* reset sequencer */
#define SEQ_RUN		0x03	/* start sequencer */
#define PALETTE_ENABLE	0x20	/* palette address source in attribute reg */
#define CHGEN_BASE	0xa0000	/* base address for character generator */
#define CHGEN_SIZE	8192	/* character generator is 8K */
#define MONO_BASE	0xb0000	/* Location of monochrome display memory */
#define MONO_SIZE	0x8000	/* Monochrome has 32K of memory available */
#define MONO_SCRMASK	0x7ff	/* Mono text memory wraps at 4K */
#define COLOR_BASE	0xb8000	/* Location of color display memory */
#define COLOR_SIZE	0x8000	/* Color has up to 32K of memory available */
#define COLOR_SCRMASK	0x1fff	/* Color text memory wraps at 16K */
#define	EGA_BASE	0xa0000	/* Location of enhanced display memory */
#define EGA_SIZE	0x10000	/* EGA has at 64K of memory available */
#define EGA_LGSIZE	0x20000	/* Larger EGA has 128K of memory available */
#define EGA_SCRMASK	0x3fff	/* EGA text memory wraps at 32K (minimum) */
#define NSEQ		5	/* number of sequencer registers */
#define NATTR		20	/* number of attribute registers */
#define NGRAPH		9	/* number of graphics registers */
#define LOAD_COLOR	8	/* mode number for loading color characters */
#define LOAD_MONO	9	/* mode number for loading mono characters */

/* Font types supported */
#define	FONTINV		0
#define	FONT8x8		1		/* 8x8 font */
#define	FONT8x14	2		/* 8x14 font */
#define	FONT8x14m	3		/* 8x14 font for monochrome */
#define	FONT8x16	4		/* 8x16 font (VGA) */
#define	FONT9x16	5		/* 9x16 font (VGA) */
#define FONT7x9		6		/* 7x9 font (VDC-600) */
#define FONT7x16	7		/* 7x16 font (VDC-600) */

/* Offsets of font tables in ROM */
#define F8x8_OFF	0xf30		/* Offset of 8x8 font table */
#define	F8x8_BPC	8		/* Bytes per character in 8x8 font */
#define	F8x8_NCH	256		/* Number of characters in 8x8 font */

#define	F8x14_OFF	0		/* Offset of 8x14 font table */
#define F8x14m_OFF	0xe00		/* Offset of 8x14 mono fudge table */
#define	F8x14_BPC	14		/* Bytes per character in 8x14 font */
#define	F8x14_NCH	256		/* Number of characters in 8x14 font */
#define	F8x16_BPC	16		/* Bytes per character in 8x16 font */
#define	F8x16_NCH	256		/* Number of characters in 8x16 font */

#define	F8x8_INDX	0		/* Index of 8x8 font pointer */
#define F8x14_INDX	1		/* Index of 8x14 font pointer */
#define F9x14_INDX	2		/* Index of 9x14 font pointer */
#define F8x16_INDX	3		/* Index of 8x16 font pointer */
#define F9x16_INDX	4		/* Index of 9x16 font pointer */

#define	FONT_TAB_LIM	0x172f		/* Total size of all font tables */

/* definitions for bits in the attribute byte */
#define BLINK		0x80
#define BRIGHT		0x08
#define REVERSE		0x70
#define NORM		0x07
#define UNDERLINE	0x01	/* underline on mono, blue on color */

#define CLEAR		(NORM<<8|0x20)
#define BCLEAR		(NORM|BRIGHT<<8|0x20)
#define ALLATTR		(BLINK|BRIGHT|REVERSE|NORM|UNDERLINE)
#define NOTBGRND	(ALLATTR&(~REVERSE))
#define NOTFGRND	(ALLATTR&(~NORM))

/* definitions for ringing the bell */
#define NORMBELL	1331	/* initial value loaded into timer */
#define BELLLEN		(HZ/10)	/* ring for 1/10 sec. between checks */
#define BELLCNT		2	/* check bell twice before turning off */
#define TONE_ON		3	/* 8254 gate 2 and speaker and-gate enabled */
#define TIMER		0x40	/* 8254.2 timer address */
#define TIMERCR		TIMER+3	/* timer control register address */
#define TIMER2		TIMER+2	/* timer tone generation port */
#define T_CTLWORD	0xB6	/* value for timer control word */
#define TONE_CTL	0x61	/* address for enabling timer port 2 */


struct adtstruct {
	ushort	*ad_scraddr;	/* address of adaptor memory */
	ushort	ad_scrmask;	/* mask for fitting text into screen memory */
	ushort	ad_address;	/* address of corresponding M6845 or EGA */
	unchar	ad_type;	/* adapter type */
	unchar	ad_colsel;	/* color select register byte */
	unchar	ad_modesel;	/* mode register byte */
	unchar	ad_mode2sel;	/* mode2 register byte (rite-vu card ) */
	unchar	ad_undattr;	/* attribute to use when underlining */
};


#define	KD_TEXT		0	/* ansi x3.64 emulation mode */
#define KD_TEXT0	0	/* same as above */
#define KD_TEXT1	2	/* new text mode doesnt load char generator */
#define	KD_GRAPHICS	1	/* graphics mode */

/*
 * Values for kv_flags
 */
#define	KD_MAPPED	0x01		/* Display is mapped */
#define	KD_LOCKED	0x02		/* Keyboard is locked */
#define	KD_QRESERVE	0x04		/* Queue mode init in progress */

/*
 * Types for indexing into reginfo table.
 */
#define I_6845MONO	0
#define I_6845COLOR	1
#define I_EGAMONO	2
#define I_EGACOLOR	3
#define I_SEQ		4
#define I_GRAPH		5
#define I_ATTR		6

/*
 * Macro for setting a particular EGA register.
 */
#define out_reg(riptr, index, data)	outb((riptr)->ri_address, (index)); \
					outb((riptr)->ri_data, (data));

#ifdef EVGA
#define in_reg(riptr, index, data)	outb((riptr)->ri_address, (index)); \
					(data) = inb((riptr)->ri_data);
#endif	/*EVGA*/

struct reginfo {
	ushort	ri_count;	/* number of registers */
	ushort	ri_address;	/* address register */
	ushort	ri_data;	/* data register */
};

struct m6845init {
	unchar  mi_hortot;	/* Reg  0: Horizontal Total     (chars) */
	unchar  mi_hordsp;	/* Reg  1: Horizontal Displayed (chars) */
	unchar  mi_hsnpos;	/* Reg  2: Hsync Position       (chars) */
	unchar  mi_hsnwid;	/* Reg  3: Hsync Width          (chars) */
	unchar  mi_vertot;	/* Reg  4: Vertical Total   (char rows) */
	unchar  mi_veradj;	/* Reg  5: Vtotal Adjust   (scan lines) */
	unchar  mi_vsndsp;	/* Reg  6: Vertical Display (char rows) */
	unchar  mi_vsnpos;	/* Reg  7: Vsync Position   (char rows) */
	unchar  mi_intlac;	/* Reg  8: Interlace Mode               */
	unchar  mi_maxscn;	/* Reg  9: Max Scan Line   (scan lines) */
	unchar  mi_curbeg;	/* Reg 10: Cursor Start    (scan lines) */
	unchar  mi_curend;	/* Reg 11: Cursor End      (scan lines) */
	unchar  mi_stadh;	/* Reg 12: Start Address (H)            */
	unchar  mi_stadl;	/* Reg 13: Start Address (L)            */
	unchar  mi_cursh;	/* Reg 14: Cursor (H)                   */
	unchar  mi_cursl;	/* Reg 15: Cursor (L)                   */
};

struct egainit {
	unchar  ei_hortot;	/* Reg  0: Horizontal Total */
	unchar  ei_hde;		/* Reg  1: Horizontal Display End */
	unchar  ei_shb;		/* Reg  2: Start horizontal blank */
	unchar  ei_ehb;		/* Reg  3: End horizontal blank */
	unchar  ei_shr;		/* Reg  4: Start horizontal retrace */
	unchar  ei_ehr;		/* Reg  5: End horizontal retrace */
	unchar  ei_vertot;	/* Reg  6: Vertical total */
	unchar  ei_ovflow;	/* Reg  7: Overflow */
	unchar  ei_prs;		/* Reg  8: Preset row scan */
	unchar  ei_maxscn;	/* Reg  9: Max Scan Line */
	unchar  ei_curbeg;	/* Reg 10: Cursor Start */
	unchar  ei_curend;	/* Reg 11: Cursor End */
	unchar  ei_stadh;	/* Reg 12: Start Address (H) */
	unchar  ei_stadl;	/* Reg 13: Start Address (L) */
	unchar  ei_cursh;	/* Reg 14: Cursor location (H) */
	unchar  ei_cursl;	/* Reg 15: Cursor location (L) */
	unchar	ei_vrs;		/* Reg 16: Vertical retrace start */
	unchar	ei_vre;		/* Reg 17: Vertical retrace end */
	unchar	ei_vde;		/* Reg 18: Vertical display end */
	unchar	ei_offset;	/* Reg 19: Offset */
	unchar	ei_undloc;	/* Reg 20: Underline location */
	unchar	ei_svb;		/* Reg 21: Start vertical blank */
	unchar	ei_evb;		/* Reg 22: End vertical blank */
	unchar	ei_mode;	/* Reg 23: Mode control */
	unchar	ei_lcomp;	/* Reg 24: Line compare */
};

struct reginit {
	unchar		seqtab[NSEQ];
	unchar		miscreg;
	struct egainit	egatab;
	unchar		attrtab[NATTR];
	unchar		graphtab[NGRAPH];
};


struct adtmode {
	unchar	am_capability;
	unchar	am_colmode;
	unchar	am_colsel;
};

struct kd_dispinfo {
	char    *vaddr;		/* display memory address */
	paddr_t	physaddr;	/* display memory address */
	unsigned long size;	/* display memory size */
};

/* Display mode definitions */
#define DM_B40x25	0		/* 40x25 black & white text */
#define DM_C40x25	1		/* 40x25 color text */
#define DM_B80x25	2		/* 80x25 black & white text */
#define DM_C80x25	3		/* 80x25 color text */
#define DM_BG320	4		/* 320x200 black & white graphics */
#define DM_CG320	5		/* 320x200 color graphics */
#define DM_BG640	6		/* 640x200 black & white graphics */
#define DM_EGAMONO80x25	7		/* EGA mode 7 */
#define LOAD_COLOR	8		/* mode for loading color characters */
#define LOAD_MONO	9		/* mode for loading mono characters */
#define DM_ENH_B80x43	10		/* 80x43 black & white text */
#define DM_ENH_C80x43	11		/* 80x43 color text */
#define DM_CG320_D	13		/* EGA mode D */
#define DM_CG640_E	14		/* EGA mode E */
#define DM_EGAMONOAPA	15		/* EGA mode F */
#define DM_CG640x350	16		/* EGA mode 10 */
#define DM_ENHMONOAPA2	17		/* EGA mode F with extended memory */
#define DM_ENH_CG640	18		/* EGA mode 10* */
#define DM_ENH_B40x25	19		/* enhanced 40x25 black & white text */
#define DM_ENH_C40x25	20		/* enhanced 40x25 color text */
#define DM_ENH_B80x25	21		/* enhanced 80x25 black & white text */
#define DM_ENH_C80x25	22		/* enhanced 80x25 color text */
#define	DM_VGA_C40x25	23		/* VGA 40x25 color text */
#define	DM_VGA_C80x25	24		/* VGA 80x25 color text */
#define	DM_VGAMONO80x25	25		/* VGA mode 7 */
#define DM_VGA640x480C	26		/* VGA 640x480 2 color graphics */
#define	DM_VGA640x480E	27		/* VGA 640x480 16 color graphics */
#define	DM_VGA320x200	28		/* VGA 320x200 256 color graphics */
#define	DM_VGA_B40x25	29		/* VGA 40x25 black & white text */
#define	DM_VGA_B80x25	30		/* VGA 80x25 black & white text */
#define	DM_VGAMONOAPA	31		/* VGA mode F+ */
#define	DM_VGA_CG640	32		/* VGA mode 10+ */
#define	DM_ENH_CGA	33		/* AT&T 640x400 CGA hw emulation mode */
#define DM_ATT_640	34		/* AT&T 640x400 16 color graphics */
#define DM_VGA_B132x25	35		/* VGA 132x25 black & white text */
#define DM_VGA_C132x25	36		/* VGA 132x25 color text */
#define DM_VGA_B132x43	37		/* VGA 132x43 black & white text */
#define DM_VGA_C132x43	38		/* VGA 132x43 color text */
#define DM_VDC800x600E	39		/* VDC-600 800x600 16 color graphics */
#define DM_VDC640x400V	40		/* VDC-600 640x400 256 color graphics */
#ifdef	EVC
#define DM_EVC640x480V	41		/* EVC 640x480 256 color graphics */
#define DM_EVC1024x768E 42		/* EVC 1024x768 16 color graphics */
#define DM_EVC1024x768D 43		/* EVC 1024x768 256 color graphics */
#endif	/*EVC*/

#ifdef EVGA

/*  THE FOLLOWING VALUES SHOULD REFLECT THE NUMBER OF NON-EVGA MODE
 *  DEFINITIONS AND ENTRIES IN KD_INITTAB, RESPECTIVELY. IF THESE
 *  ARE CHANGED THEN ENDNONEVGAMODE AND STEVGA SHOULD BE ADJUSTED
 *  ACCORDINGLY.
 */
#ifndef EVC

#define ENDNONEVGAMODE 40
#define STEVGA 7

#else /* EVC defined */

#define ENDNONEVGAMODE 43
#define STEVGA 9

#endif /* EVC */ 
#endif /* EVGA */


/* Xenix display mode definitions.  Most are identical to the internal
   definitions above. */
#define M_B40x25	DM_B40x25	/* black & white 40 columns */
#define M_C40x25	DM_C40x25	/* color 40 columns */
#define M_B80x25	DM_B80x25	/* black & white 80 columns */
#define M_C80x25	DM_C80x25	/* color 80 columns */
#define M_BG320		DM_BG320	/* black & white graphics 320x200 */
#define M_CG320		DM_CG320	/* color graphics 320x200 */
#define M_BG640		DM_BG640	/* black & white graphics 640x200 */
#define M_EGAMONO80x25  DM_EGAMONO80x25	/* ega-mono 80x25 */
#define M_CG320_D	DM_CG320_D	/* ega mode D */
#define M_CG640_E	DM_CG640_E	/* ega mode E */
#define M_EGAMONOAPA	DM_EGAMONOAPA	/* ega mode F */
#define M_CG640x350	DM_CG640x350	/* ega mode 10 */
#define M_ENHMONOAPA2	DM_ENHMONOAPA2	/* ega mode F with extended memory */
#define M_ENH_CG640	DM_ENH_CG640	/* ega mode 10* */
#define M_ENH_B40x25    DM_ENH_B40x25	/* enhanced black & white 40 columns */
#define M_ENH_C40x25    DM_ENH_C40x25	/* enhanced color 40 columns */
#define M_ENH_B80x25    DM_ENH_B80x25	/* enhanced black & white 80 columns */
#define M_ENH_C80x25    DM_ENH_C80x25	/* enhanced color 80 columns */
#define M_VGA_40x25	DM_VGA_C40x25	/* vga 8x16 font on color */
#define M_VGA_80x25	DM_VGA_C80x25	/* vga 8x16 font on color */
#define M_VGA_M80x25	DM_VGAMONO80x25	/* vga 8x16 font on mono */
#define M_VGA11		DM_VGA640x480C	/* vga 640x480 2 colors */
#define M_VGA12		DM_VGA640x480E	/* vga 640x480 16 colors */
#define M_VGA13		DM_VGA320x200	/* vga 640x200 256 colors */
					/* "640" because double scan */
#define M_HGC_P0	0xe0	/* hercules graphics - page 0 @ B0000 */
#define M_HGC_P1	0xe1	/* hercules graphics - page 1 @ B8000 */
#define M_ENH_B80x43	0x70		/* ega black & white 80x43 */
#define M_ENH_C80x43	0x71		/* ega color 80x43 */
#define M_MCA_MODE	0xff		/* monochrome adapter mode */
#define	OFFSET_80x43	(M_ENH_B80x43-DM_ENH_B80x43)	/* Offset for converting 80x43 numbers */
#define M_ENH_CGA	DM_ENH_CGA	/* AT&T 640x400 CGA hardware mode - (Super-VU adapter) */

/* Defines for keyboard and display ioctl's */
#define KIOC		('K'<<8)
#define	KDDISPTYPE	(KIOC|1)	/* return display type to user */
#define	KDMAPDISP	(KIOC|2)	/* map display into user space */
#define	KDUNMAPDISP	(KIOC|3)	/* unmap display from user space */
#define	KDGKBENT	(KIOC|4)	/* get keyboard table entry */
#define	KDSKBENT	(KIOC|5)	/* set keyboard table entry */
#define	KDGKBMODE	(KIOC|6)	/* get keyboard translation mode */
#define	KDSKBMODE	(KIOC|7)	/* set keyboard translation mode */
#define	KDMKTONE	(KIOC|8)	/* sound tone */
#define	KDGETMODE	(KIOC|9)	/* get text/graphics mode */
#define	KDSETMODE	(KIOC|10)	/* set text/graphics mode */
#define	KDADDIO		(KIOC|11)	/* add I/O address to list */
#define	KDDELIO		(KIOC|12)	/* delete I/O address from list */
#define KDSBORDER	(KIOC|13)	/* set ega color border */
#define KDQUEMODE	(KIOC|15)	/* enable/disable queue mode */
#define KIOCDOSMODE     (KIOC | 16)     /* obsolete -- set DOSMODE	    */
#define KIOCNONDOSMODE  (KIOC | 17)     /* obsolete -- clear DOSMODE	    */
#define KDDISPINFO	(KIOC | 18)	/* get display start and size	    */
#define KDGKBSTATE	(KIOC | 19)	/* get state of keyboard shift keys */
#define KDSETRAD        (KIOC|20)       /* set keyboard typematic rate/delay */
#define KDSCROLL        (KIOC|21)       /* set hardware scrolling on/off */

/* VP/ix reserved ioctls */
#define KDENABIO	(KIOC|60)	/* enable direct I/O to ports */
#define KDDISABIO	(KIOC|61)	/* disable direct I/O to ports */
#define KIOCINFO	(KIOC|62)	/* tell user what device we are */
#define KIOCSOUND	(KIOC|63)	/* start sound generation */
#define KDGKBTYPE	(KIOC|64)	/* get keyboard type */
#define KDGETLED	(KIOC|65)	/* get keyboard LED status */
#define KDSETLED	(KIOC|66)	/* set keyboard LED status */


/* New ioctls for UNIX System V Release 4.0. 0-50 reserved for base system */
/* 50-100 reserved for AT&T-DSG */

#define	WSIOC		(('w' << 24) | ('s' << 16))
#define	KDVDCTYPE	(WSIOC|1)	/* VDC controller/display information */
#define KDDFLTKEYMAP	(WSIOC|2)	/* set/get default keyboard map for
					 * this workstation */
#define KDDFLTSCRNMAP	(WSIOC|3)	/* set/get default screen map for
					 * this workstation */
#define KDDFLTSTRMAP	(WSIOC|4)	/* set/get default function key map for
					 * this workstation */
#define WS_PIO_ROMFONT	(WSIOC|5)	/* add user-supplied font overlays */
#define WS_CLRXXCOMPAT	(WSIOC|6)	/* add user-supplied font overlays */
#define WS_GETXXCOMPAT	(WSIOC|7)	/* add user-supplied font overlays */
#define WS_SETXXCOMPAT	(WSIOC|8)	/* add user-supplied font overlays */
#define KDEVGA        	(WSIOC|9)       /* set evga card type */

/* Defines for Xenix keyboard and display ioctl's */
#define MIOC		('k' << 8)	/* Upper byte of mapping ioctl's */
#define GETFKEY   	(MIOC | 0)	/* Get function key */
#define SETFKEY   	(MIOC | 1)	/* Set function key */
#define GIO_SCRNMAP	(MIOC | 2)	/* Get screen output map table */
#define PIO_SCRNMAP	(MIOC | 3)	/* Set screen output map table */
#define	GIO_STRMAP_21	(MIOC | 4)	/* Get 2.1 function key string table */
#define	PIO_STRMAP_21	(MIOC | 5)	/* Put 2.1 function key string table */
#define GIO_KEYMAP	(MIOC | 6)	/* Get keyboard map table */
#define PIO_KEYMAP	(MIOC | 7)	/* Set keyboard map table */
#define SETLOCKLOCK	(MIOC|10)	/* global cap/num lock on/off */
#define GIO_STRMAP	(MIOC | 11)	/* Get function key string table */
#define PIO_STRMAP	(MIOC | 12)	/* Set function key string table */
#define	KBIO_SETMODE	(MIOC|13)	/* Put AT keyboard into XT | AT mode */
#define	KBIO_GETMODE	(MIOC|14)	/* Get the AT/XT keyboard mode */

/* keyboard mode -- set by KBIO_MODE */
#define	KBM_XT	0	/* XT keyboard mode */
#define	KBM_AT	1	/* AT keyboard mode */

#define GIO_ATTR	('a' << 8)	/* Get present screen attribute */
#define GIO_COLOR	('c' << 8)	/* Get whether adaptor is color */

#define MODESWITCH	('x' << 8)	/* Upper byte of mode switch ioctl's */
#define KDMODEMASK	0xff		/* Lower byte of mode switch ioctl's */
#define SW_B40x25	(MODESWITCH | DM_B40x25)	/* Select 40x25 b&w */
#define SW_C40x25	(MODESWITCH | DM_C40x25)	/* Select 40x25 clr */
#define SW_B80x25	(MODESWITCH | DM_B80x25)	/* Select 80x25 b&w */
#define SW_C80x25	(MODESWITCH | DM_C80x25)	/* Select 80x25 clr */
#define SW_BG320	(MODESWITCH | DM_BG320)	/* Select 320x200 b&w */
#define SW_CG320	(MODESWITCH | DM_CG320)	/* Select 320x200 color */
#define SW_BG640	(MODESWITCH | DM_BG640)	/* Select 640x200 b&w */
#define SW_EGAMONO80x25	(MODESWITCH | DM_EGAMONO80x25)	/* Select EGA mode 7 */
#define SW_CG320_D	(MODESWITCH | DM_CG320_D)	/* Select EGA mode D */
#define SW_CG640_E	(MODESWITCH | DM_CG640_E)	/* Select EGA mode E */
#define SW_EGAMONOAPA	(MODESWITCH | DM_EGAMONOAPA)	/* Select EGA mode F */
#define SW_CG640x350	(MODESWITCH | DM_CG640x350)	/* EGA mode 10 */
#define SW_ENH_MONOAPA2	(MODESWITCH | DM_ENHMONOAPA2)	/* EGA mode F* */
#define SW_ENH_CG640	(MODESWITCH | DM_ENH_CG640)	/* EGA mode 16 */
#define SW_ENHB40x25	(MODESWITCH | DM_ENH_B40x25)	/* 40x25 b&w */
#define SW_ENHC40x25	(MODESWITCH | DM_ENH_C40x25)	/* 40x25 color */
#define SW_ENHB80x25	(MODESWITCH | DM_ENH_B80x25)	/* 80x25 b&w */
#define SW_ENHC80x25	(MODESWITCH | DM_ENH_C80x25)	/* 80x25 color */
#define SW_ENHB80x43	(MODESWITCH | M_ENH_B80x43)	/* 80x43 b&w */
#define SW_ENHC80x43	(MODESWITCH | M_ENH_C80x43)	/* 80x43 color */
#define SW_MCAMODE	(MODESWITCH | M_MCA_MODE)	/* Reinitialize mono */
#define SW_ATT640	(MODESWITCH | DM_ATT_640)	/* 640x400 16 color */
#define	SW_VGAC40x25	(MODESWITCH | DM_VGA_C40x25)	/* VGA 40x25 color */
#define	SW_VGAC80x25	(MODESWITCH | DM_VGA_C80x25)	/* VGA 80x25 color */
#define	SW_VGAMONO80x25	(MODESWITCH | DM_VGAMONO80x25)	/* VGA mode 7 */
#define	SW_VGA640x480C	(MODESWITCH | DM_VGA640x480C)	/* VGA mode 11 */
#define	SW_VGA640x480E	(MODESWITCH | DM_VGA640x480E)	/* VGA mode 12 */
#define	SW_VGA320x200	(MODESWITCH | DM_VGA320x200)	/* VGA mode 13 */
#define SW_VDC800x600E	(MODESWITCH | DM_VDC800x600E)	/* 800x600 16 color */
#define SW_VDC640x400V	(MODESWITCH | DM_VDC640x400V)	/* 640x400 256 color */
#define	SW_VGAB40x25	(MODESWITCH | DM_VGA_B40x25)	/* VGA 40x25 b&w */
#define	SW_VGAB80x25	(MODESWITCH | DM_VGA_B80x25)	/* VGA 80x25 b&w */
#define	SW_VGAMONOAPA	(MODESWITCH | DM_VGAMONOAPA)	/* VGA mode F+ */
#define	SW_VGA_CG640	(MODESWITCH | DM_VGA_CG640)	/* VGA mode 10+ */
#define	SW_VGA_B132x25	(MODESWITCH | DM_VGA_B132x25)	/* VGA 132x25 b&w */
#define	SW_VGA_C132x25	(MODESWITCH | DM_VGA_C132x25)	/* VGA 132x25 color */
#define	SW_VGA_B132x43	(MODESWITCH | DM_VGA_B132x43)	/* VGA 132x43 b&w */
#define	SW_VGA_C132x43	(MODESWITCH | DM_VGA_C132x43)	/* VGA 132x43 color */
#ifdef	EVC
#define SW_EVC640x480V	(MODESWITCH | DM_EVC640x480V)	/* 640x480 256 color */
#define SW_EVC1024x768E (MODESWITCH | DM_EVC1024x768E)	/* 1024x768 16 color */
#define SW_EVC1024x768D (MODESWITCH | DM_EVC1024x768D)  /* 1024x768 256 color*/
#endif	/*EVC*/

/* XENIX names for VGA modes */
#define	SW_VGA40x25	SW_VGAC40x25
#define	SW_VGA80x25	SW_VGAC80x25
#define	SW_VGAM80x25	SW_VGAMONO80x25
#define	SW_VGA11	SW_VGA640x480C
#define	SW_VGA12	SW_VGA640x480E
#define	SW_VGA13	SW_VGA320x200
#define SW_VGA_C40x25   SW_VGAC40x25
#define SW_BG640x480    SW_VGA640x480C
#define SW_CG640x480    SW_VGA640x480E
#define SW_VGA_CG320    SW_VGA320x200
#define SW_VGA_B40x25   SW_VGAB40x25


#ifdef EVGA
/* display modes for evga */

#define	VT_EGA		0	/* EGA 		640x350 16 colors */
#define	VT_PEGA		1	/* PEGA2 	640x480 16 colors*/
#define	VT_VGA		2	/* VGA 		640x480 16 colors*/
#define	VT_VEGA720	3	/* VEGA VGA 	720x540	16 colors */
#define	VT_VEGA800	4	/* VEGA VGA 	800x600	16 colors */
#define	VT_TSL8005_16	5	/* Tseng Labs	800x560	16 colors */
#define	VT_TSL8006_16	6	/* Tseng Labs	800x600	16 colors */
#define	VT_TSL960	7	/* Tseng Labs	960x720	16 colors */
#define	VT_TSL1024	8	/* Tseng Labs  1024x768 16 colors */
#define VT_TSL1024ni	9	/* Tseng Labs  1024x768 16 colors NI */
#define	VT_SIGMAH	10	/* Sigma VGA/H  800x600 16 colors */
#define VT_PVGA1A	11	/* Paradise PVGA1A 800x600 16 colors */
#define VT_V7VRAM6	12	/* Video 7 VRAM	640x480 16 colors */
#define VT_V7VRAM7	13	/* Video 7 VRAM	720x540 16 colors */
#define VT_V7VRAM8	14	/* Video 7 VRAM	800x600 16 colors */
#define VT_V7VRAM1_2	15	/* Video 7 VRAM 1024x768  2 colors */
#define VT_V7VRAM1_4	16	/* Video 7 VRAM 1024x768  4 colors */
#define VT_V7VRAM1_16	17	/* Video 7 VRAM 1024x768 16 colors */
#define VT_GENEGA_6	18	/* Genoa EGA    640x480 16 colors */
#define VT_GENEGA_8	19	/* Genoa EGA    800x600 16 colors */
#define VT_ORVGA8	20	/* Orchid VGA   800x600 16 colors */
#define VT_GVGA8_6	21	/* Genoa VGA    800x600 16 colors */
#define VT_DELL7	22	/* Dell VGA (Video 7)    720x540 16 colors */
#define VT_DELL8	23	/* Dell VGA (Video 7)    800x600 16 colors */
#define VT_VGAWON	24	/* ATI VGA Wonder    800x600 16 colors */
#define VT_PVGA1024	25	/* WD Paradise VGA 1024 1024x768 16 colors */
#define VT_PVGA1024_8	26	/* WD Paradise VGA 1024 800x600 16 colors */
/* 
 * NOTICE:  Insert new types here.  FastWrite goes at end so we can save
 * space in the register initialization table.  These are dummy entries
 */
#define VT_V7FW6	27	/* Video 7 FastWrite 640x480 16 colors */
#define VT_V7FW7	28	/* Video 7 FastWrite 720x540 16 colors */
#define VT_V7FW8	29	/* Video 7 FastWrite 800x600 16 colors */
#define VT_V7FW1_2	30	/* Video 7 FastWrite 1024x768  2 colors */
#define VT_V7FW1_4	31	/* Video 7 FastWrite 1024x768  4 colors */

#endif	/*EVGA*/

#define EVGAIOC (('E'<<24)|('V'<<16))
#define EVGAMODEMASK	0xffff	

#ifdef EVGA
/* generic mode constants for evga */

#define GEN_640x350		0
#define GEN_640x480		1
#define GEN_720x540		2
#define GEN_800x560		3
#define GEN_800x600		4
#define GEN_960x720		5
#define GEN_1024x768		6
#define GEN_1024x768x2		7
#define GEN_1024x768x4		8

/*
 * generic mode switches for evga. Currently these values
 * are just supported for the card types for which evga support
 * has been implemented. Eventually these mode switching constants 
 * will be supported for other card types as well.
 */

#define SW_GEN_640x350		(EVGAIOC | GEN_640x350)
#define SW_GEN_640x480		(EVGAIOC | GEN_640x480)
#define SW_GEN_720x540		(EVGAIOC | GEN_720x540)
#define SW_GEN_800x560		(EVGAIOC | GEN_800x560)
#define SW_GEN_800x600		(EVGAIOC | GEN_800x600)
#define SW_GEN_960x720		(EVGAIOC | GEN_960x720)
#define SW_GEN_1024x768		(EVGAIOC | GEN_1024x768)
#define SW_GEN_1024x768x2	(EVGAIOC | GEN_1024x768x2)
#define SW_GEN_1024x768x4	(EVGAIOC | GEN_1024x768x4)

/* temporary kludge for X server */
#define TEMPEVC1024x768E	(MODESWITCH | 42)

#define MISC_OUT_READ 	0x3cc
#define IO_ADDR_SEL	1

/*
 * General adaptor information for evga such as size of display, 
 * pixels per inch, etc.
 */
struct at_disp_info {
	int 	type;		/* controller type, EVGA_VRAM */
	int	vt_type;	/* mode, VT_V7VRAM7 */
	int	is_vga;		/* true if adapter supports VGA write modes */
	int	xpix;		/* pixels per scanline */
	int	ypix;		/* number of scanlines */
	int	planes;		/* number of planes of memory */
	int	colors;		/* number of colors available */
	int	buf_size;	/* size of screen memory */
	int	map_size;	/* size of one plane of memory */
	int	slbytes;	/* number of bytes in a scanline */
	int	(*ext_init)();	/* called to initialize 'extended' modes */
	int	(*ext_rest)();	/* called to recover from 'extended' modes */
	struct b_param *regs;	/* registers for mode */
};

/* end of evga stuff */
#endif	/*EVGA*/


/* Hercules support */
#define SW_HGC_P0  (MODESWITCH | M_HGC_P0)
#define SW_HGC_P1  (MODESWITCH | M_HGC_P1)

#define O_MODESWITCH	('S' << 8)	/* Upper byte of mode switch ioctl's */
#define O_SW_B40x25	(O_MODESWITCH | DM_B40x25)	/* Select 40x25 b&w */
#define O_SW_C40x25	(O_MODESWITCH | DM_C40x25)	/* Select 40x25 clr */
#define O_SW_B80x25	(O_MODESWITCH | DM_B80x25)	/* Select 80x25 b&w */
#define O_SW_C80x25	(O_MODESWITCH | DM_C80x25)	/* Select 80x25 clr */
#define O_SW_BG320	(O_MODESWITCH | DM_BG320)	/* Select 320x200 b&w */
#define O_SW_CG320	(O_MODESWITCH | DM_CG320)	/* Select 320x200 color */
#define O_SW_BG640	(O_MODESWITCH | DM_BG640)	/* Select 640x200 b&w */
#define O_SW_EGAMONO80x25	(O_MODESWITCH | DM_EGAMONO80x25)	/* Select EGA mode 7 */
#define O_SW_CG320_D	(O_MODESWITCH | DM_CG320_D)	/* Select EGA mode D */
#define O_SW_CG640_E	(O_MODESWITCH | DM_CG640_E)	/* Select EGA mode E */
#define O_SW_EGAMONOAPA	(O_MODESWITCH | DM_EGAMONOAPA)	/* Select EGA mode F */
#define O_SW_CG640x350	(O_MODESWITCH | DM_CG640x350)	/* EGA mode 10 */
#define O_SW_ENH_MONOAPA2	(O_MODESWITCH | DM_ENHMONOAPA2)	/* EGA mode F* */
#define O_SW_ENH_CG640	(O_MODESWITCH | DM_ENH_CG640)	/* EGA mode 16 */
#define O_SW_ENHB40x25	(O_MODESWITCH | DM_ENH_B40x25)	/* 40x25 b&w */
#define O_SW_ENHC40x25	(O_MODESWITCH | DM_ENH_C40x25)	/* 40x25 color */
#define O_SW_ENHB80x25	(O_MODESWITCH | DM_ENH_B80x25)	/* 80x25 b&w */
#define O_SW_ENHC80x25	(O_MODESWITCH | DM_ENH_C80x25)	/* 80x25 color */
#define O_SW_ENHB80x43	(O_MODESWITCH | M_ENH_B80x43)	/* 80x43 b&w */
#define O_SW_ENHC80x43	(O_MODESWITCH | M_ENH_C80x43)	/* 80x43 color */
#define O_SW_MCAMODE	(O_MODESWITCH | M_MCA_MODE)	/* Reinitialize mono */
#define O_SW_ATT640	(O_MODESWITCH | DM_ATT_640)	/* 640x400 16 color */
#define	O_SW_VGA40x25	(O_MODESWITCH | M_VGA_40x25)
#define	O_SW_VGA80x25	(O_MODESWITCH | M_VGA_80x25)
#define	O_SW_VGAM80x25	(O_MODESWITCH | M_VGA_M80x25)
#define	O_SW_VGA11	(O_MODESWITCH | M_VGA11)
#define	O_SW_VGA12	(O_MODESWITCH | M_VGA12)
#define	O_SW_VGA13	(O_MODESWITCH | M_VGA13)
#define O_SW_VGA_C40x25 (O_MODESWITCH | M_VGA_40x25)
#define O_SW_BG640x480  (O_MODESWITCH | M_VGA11)
#define O_SW_CG640x480  (O_MODESWITCH | M_VGA12)
#define O_SW_VGA_CG320  (O_MODESWITCH | M_VGA13)
#define O_SW_VGA_B40x25 (O_MODESWITCH | DM_VGA_B40x25)
#define O_SW_VGA_B80x25 (O_MODESWITCH | DM_VGA_B80x25)

#define	CGAIOC		('C' << 8)	/* Upper byte of CGA ioctl's */
#define CGAMODE		(CGAIOC | 1)	/* Obsolete */
#define CGAIO		(CGAIOC | 2)	/* Do I/O on CGA port */
#define CGA_GET 	(CGAIOC | 3)	/* Get CGA mode setting */
#define INTERNAL_VID	(CGAIOC|72)	/* internal plasma monitor	*/
#define EXTERNAL_VID	(CGAIOC|73)	/* external plasma monitor	*/

#define PGAIOC		('P' << 8)	/* Upper byte of PGA ioctl's */
#define PGAMODE		(PGAIOC | 1)	/* Obsolete */
#define PGAIO		(PGAIOC | 2)	/* Do I/O on PGA port */
#define PGA_GET 	(PGAIOC | 3)	/* Get PGA mode setting */

#define EGAIOC		('E' << 8)	/* Upper byte of EGA ioctl's */
#define EGAMODE		(EGAIOC | 1)	/* Obsolete */
#define EGAIO		(EGAIOC | 2)	/* Do I/O on EGA port */
#define EGA_GET 	(EGAIOC | 3)	/* Get EGA mode setting */
#define EGA_IOPRIVL (EGAIOC | 4)	/* get in/out privilege for ega ports */

#define MCAIOC		('M' << 8)	/* Upper byte of MCA ioctl's */
#define MCAMODE		(MCAIOC | 1)	/* Obsolete */
#define MCAIO		(MCAIOC | 2)	/* Do I/O on MCA port */
#define MCA_GET 	(MCAIOC | 3)	/* Get MCA mode setting */

/* PC/AT Vga adapter control */
#define VGAIOC	('E' << 8)
#define VGAMODE (VGAIOC | 65)		/* change vga mode */
#define VGAIO	(VGAIOC | 66)		/* do inb/outb on vga port */
#define VGA_GET (VGAIOC | 67)		/* get vga mode setting */
#define VGA_IOPRIVL (VGAIOC | 68)	/* get in/out privilege for vga ports */

/* The following ioctl conflicts with the ioctls TCGETX in termiox.h
 * and STSET in stermio.h. The ioctl is defined only for source
 * compatibility and is only valid if SCO compatibility mode is turned
 * on -- see WS_GETXXCOMPAT
 */
/* PC/AT Special Adapter Support */
#define	S_IOC	('X' << 8)
#define	SPECIAL_IOPRIVL (S_IOC | 1)	/* get IO privl on special board.
					   using  vid_iop[] */

#define CONSIOC		('c' << 8)	/* Upper byte of console ioctl's */
#define CONS_CURRENT 	(CONSIOC | 1)	/* Get display adapter type */
#define CONS_GET	(CONSIOC | 2)	/* Get display mode setting */
#define CONSIO		(CONSIOC | 3)	/* do inb/outb on console port */

#define PIO_FONT8x8	(CONSIOC|64)
#define GIO_FONT8x8	(CONSIOC|65)
#define PIO_FONT8x14	(CONSIOC|66)
#define GIO_FONT8x14	(CONSIOC|67)
#define PIO_FONT8x16	(CONSIOC|68)
#define GIO_FONT8x16	(CONSIOC|69)

#define CONSADP		(CONSIOC|72)	/* get specific adapter screen */
#define CONS_GETINFO	(CONSIOC|73)	/* get vid_info struct S001 */
#define CONS_6845INFO	(CONSIOC|74)	/* get m6845_info struct S001 */

struct colors {
	char fore;			/* foreground colors	*/
	char back;			/* background colors	*/
};

#define	BLACK		0x0
#define	BLUE		0x1
#define	GREEN		0x2
#define	CYAN		0x3
#define	RED		0x4
#define	MAGENTA		0x5
#define	BROWN		0x6
#define	WHITE		0x7
#define	GRAY		0x8
#define	LT_BLUE		0x9
#define	LT_GREEN	0xA
#define	LT_CYAN		0xB
#define	LT_RED		0xC
#define	LT_MAGENTA	0xD
#define	YELLOW		0xE
#define	HI_WHITE	0xF

/*
 * flag definitions for mk_keylock
 */
#define	CLKED	0x01		/* caps   locked */
#define	NLKED	0x02		/* num    locked */
#define	SLKED	0x04		/* scroll locked */

struct vid_info
{
	short	size;			/* must be first field		*/
	short	m_num;			/* multiscreen number, 0 based	*/
	ushort	mv_row, mv_col;		/* cursor position		*/
	ushort	mv_rsz, mv_csz;		/* text screen size		*/
	struct colors	mv_norm,	/* normal attributes		*/
			mv_rev,		/* reverse video attributes	*/
			mv_grfc;	/* graphic character attributes	*/
	uchar_t mv_ovscan;		/* border color			*/
	uchar_t mk_keylock;		/* caps/num/scroll lock		*/
};

struct m6845_info
{
	short	size;		/* must be first field */
	ushort	screen_top;	/* offset of screen in video	*/
	ushort	cursor_type;	/* cursor shape			*/
};

typedef struct {
	int cmd, flg;		/* weird data structure to make loadable */
	faddr_t faddr;		/* fonts easier to implement	*/
} pgiofontarg_t;		/* Not for use by user programs!! */

#define MAPADAPTER 	('m' << 8)	/* Upper byte of mapping ioctl's */
#define MAPCONS 	(MAPADAPTER)	/* Map display adapter memory */
#define MAPMONO 	(MAPADAPTER | MONO)	/* Map MCA adapter memory */
#define MAPCGA  	(MAPADAPTER | CGA) 	/* Map CGA adapter memory */
#define MAPPGA  	(MAPADAPTER | PGA)	/* Map PGA adapter memory */
#define MAPEGA  	(MAPADAPTER | EGA)	/* Map EGA adapter memory */
#define MAPVGA		(MAPADAPTER | VGA)
#define MAPSPECIAL	(MAPADAPTER | S_ADAP)

#define SWAPCONS 	('s' << 8)	/* Upper byte of switching ioctl's */
#define SWAPMONO 	(SWAPCONS | MONO)	/* Swap MCA adapter */
#define SWAPCGA  	(SWAPCONS | CGA)	/* Swap CGA adapter */
#define SWAPPGA  	(SWAPCONS | PGA)	/* Swap PGA adapter */
#define SWAPEGA  	(SWAPCONS | EGA)	/* Swap EGA adapter */
#define SWAPVGA  	(SWAPCONS | VGA)	/* Swap VGA adapter */

#ifndef _KB_386	/* These are also defined in termios.h	*/
#define _KB_386
#define TIOCKBON  	(TIOC | 8)	/* Turn on extended keys */
#define TIOCKBOF  	(TIOC | 9)	/* Turn off extended keys */
#define KBENABLED 	(TIOC | 10)	/* Are extended keys enabled? */
#endif	/* _KB_386	*/


/* VP/ix keyboard types */
#define KB_84		1
#define KB_101		2
#define KB_OTHER	3

struct kbentry {
	unchar	kb_table;	/* which table to use */
	unchar	kb_index;	/* which entry in table */
	ushort	kb_value;	/* value to get/set in table */
};

#define	MKDCONFADDR	20	/* max no. of configurable addrs 
				 * that can be supported */
#define	MKDIOADDR	64	/* max no. of I/O addresses supported */

struct kd_disparam {
	long	type;		/* display type */
	char	*addr;		/* display memory address */
	ushort	ioaddr[MKDIOADDR];	/* valid I/O addresses */
};

struct kd_vdctype {
	long	cntlr,	/* controller type */
		dsply,	/* display type */
		rsrvd;	/* reserved for future enhancement */
};

struct kd_memloc {
	char	*vaddr;		/* virtual address to map to */
	char	*physaddr;	/* physical address to map from */
	long	length;		/* size in bytes to map */
	long	ioflg;		/* enable I/O addresses if non-zero */
};

struct	kd_range {
	ulong	start;		/* start address of video memory */
	ulong	end;		/* last address of video memory */
};

struct kd_quemode {
	int	qsize;	/* desired # elements in queue (set by user) */
	int	signo;	/* signal number to send when queue goes non-empty
							(set by user) */
	char	*qaddr;	/* user virtual address of queue (set by driver) */
};

/* Defines for port I/O ioctls for graphics adapter ports */
#define IN_ON_PORT	1
#define OUT_ON_PORT	0

/* Structures for port I/O ioctls for graphics adapter ports */
#pragma pack(2)
struct port_io_struct {
	char	dir;		/* Direction flag (in or out) */
	ushort	port;		/* Port address */
	char	data;		/* Port data */
};
struct port_io_arg {
	struct port_io_struct args[4];	/* Port I/O's for single call */
};
#pragma pack()

#define MKDBASEIO	35	/* base system I/O address array size */

struct b_param { /* format of video parameters in bios */
	unchar		fill[5];
	unchar		seqtab[NSEQ - 1],
			miscreg;
	struct egainit	egatab;
	unchar		attrtab[NATTR],
			graphtab[NGRAPH];
};

#define	KD_BIOS	0	/* video parameters reside in the BIOS tables */
#define	KD_TBLE	1	/* video parameters reside in hard-coded tables */
#define	KD_CAS2	2	/* video parameters reside in supplemental table */

/*
 * Register values for CGA modes
 */

struct cgareginfo {
	unchar	cga_mode;		/* Mode select register value */
	unchar	cga_color;		/* Color select register value */
	ushort	cga_index;		/* Index into cga_videop array */
} ;

/*
 * EGA mode information.
 */

/* NOTE: This structure defined for compatibility only. Use the modeinfo struct
 * in vid.h
 */
struct mode_info {
	ushort	width,	/* alphanumeric mode widths (0 indicates graphics) */
		height; /* alphanumeric mode heights (0 indicates graphics) */
	unchar	color;	/* non-zero value indicates a color mode */
	paddr_t	base;	/* base screen memory physical address */
	ulong	size;	/* screen memory size */
	unchar	font,	/* font type (0 indicates graphics) */
		params,	/* parameter location: in bios or static table */
		offset,	/* offset given m_loc above */
		ramdac;	/* RAMDAC table offset to use */
} ;

struct kb_shiftmkbrk {
	ushort	mb_mask;
	unchar	mb_make,
		mb_break;
	unchar	mb_prfx;
} ;

struct font_info {
	unchar	*f_fontp;
	ushort	f_bpc;
	ulong	f_count;
};

struct vertim {
	unchar	v_ind,
		v_val;
} ;

#define KD_HERCINDX	0
#define KD_MONOINDX	1
#define KD_COLRINDX	2
#define KD_EGAINDX	3

#define VDCGRNUM	6

struct vdc_graphadds {
	unchar	v_graphtab[VDCGRNUM];
};

struct kdvtinfo {		/* device specific info for kd vt driver */
	scrnmapp_t		kv_scrnmap;	/* Output character map */
	unsigned		kv_sending;	/* Sending screen? */
	ushort			kv_rows;	/* Rows sent from screen */
	ushort			kv_cols;	/* Columns sent from screen */
	unchar			kv_kbmode;	/* keyboard mode */
	unchar			kv_dmode;	/* display mode */
	unchar			kv_flags;	/* flags */
	unchar			kv_egamode;	/* Saved EGA mode */
};

struct ext_graph {
	long	pid;
	struct proc *procp;
};


#endif /* _SYS_KD_H */
