#ident	"@(#)e503.h	1.1	92/09/30	JPB"

/*      @(#)e503.h	3.2 Lachman System V STREAMS TCP  source        */
/*
 *      System V STREAMS TCP - Release 3.0
 *
 *      Copyright 1987, 1988, 1989 Lachman Associates, Incorporated (LAI)
 *
 *      All Rights Reserved.
 *
 *      System V STREAMS TCP was jointly developed by Lachman
 *      Associates and Convergent Technologies.
 */
/* Header for 3com Ethernet board */

/* Ethernet frame */

#define	E3COM_ADDR	6
#define E3COM_TYPE	2      /*  -   The type field           */
#define E3COM_DATA   1500      /*  -   Max data bytes in the E3COM */
#define E3COM_CRC      4      /*  -   Four byte CRC            */

typedef struct {
	unsigned char octet[E3COM_ADDR];
} e_addr;

struct ether_header {
	unsigned char ether_dhost[E3COM_ADDR];	/* 48-bit destination address */
	unsigned char ether_shost[E3COM_ADDR];	/* 48-bit source address */
#if defined(M_I386) || defined(i386)
	unsigned short e_type;	/* 16-bit type field */
#else
	unsigned char e_type[E3COM_TYPE];	/* 16-bit type field */
#endif
};

typedef struct ether_header e_frame;

#define E503IOBASE	(device->base)

/* LAN Controller, Control Register bits 3=0, 2=0 */

#define CR	(device->base)		/* Command register (r/w) */
#define		STP	0x01			/* Stop */
#define		STA	0x02			/* Start */
#define		TXP	0x04			/* Transmit packet */
#define		RD0	0x08			/* Remote DMA command */
#define		RD1	0x10
#define		RD2	0x20
#define		PS0	0x40			/* Page select */
#define		PS1	0x80
/* page 0 registers */
#define PSTART	(device->base+0x01)	/* Page start register (w) */
#define PSTOP	(device->base+0x02)	/* Page stop register (w) */
#define BNRY	(device->base+0x03)	/* Boundary pointer (r/w) */
#define TSR	(device->base+0x04)	/* Transmit status (r) */
#define		PTX	0x01			/* Packet transmitted */
#define		NDT	0x02			/* Non-deferred transmission */
#define		COL	0x04			/* Transmit collided */
#define		ABT	0x08			/* Transmit aborted */
#define		CRS	0x10			/* Carrier sense lost */
#define 	FU	0x20			/* FIFO underrun */
#define		CDH	0x40			/* Collision-detect heartbeat */
#define		OWC	0x80			/* Out-of-windown collision */
#define TPSR	(device->base+0x04)	/* Transmit page start (w) */
#define NCR	(device->base+0x05)	/* # collisions (r) */
#define TBCR0	(device->base+0x05)	/* Transmit byte count (w) */
#define TBCR1	(device->base+0x06)
#define ISR	(device->base+0x07)	/* Interrupt status (r/w) */
#define		IPRX	0x01		 	/* Packet received */
#define		IPTX	0x02			/* Packet transmitted */
#define		IRXE	0x04			/* Receive error */
#define		ITXE	0x08			/* Transmit error */
#define		IOVW	0x10			/* Overwrite warning */
#define		ICNT	0x20			/* Counter overflow */
#define		IRDC	0x40			/* Remote DMA complete */
#define		IRST	0x80			/* Reset status */
#define RSAR0	(device->base+0x08)	/* Remote start address (w) */
#define RSAR1	(device->base+0x09)
#define RBCR0	(device->base+0x0a)	/* Remote byte count (w) */
#define RBCR1	(device->base+0x0b)
#define RSR	(device->base+0x0c)	/* Receive status (r) */
#define		PRX	0x01			/* Pakcet received intact */
#define		CRCE	0x02			/* CRC error */
#define		FAE	0x04			/* Frame alignment error */
#define		FO	0x08			/* FIFO overrun */
#define		MPA	0x10			/* Missed packet */
#define		PHY	0x20			/* Physical/multicast address */
#define		DIS	0x40			/* Receiver disabled */
#define		DFR	0x80			/* Deferring */
#define RCR	(device->base+0x0c)	/* Receive configuration (w) */
#define		SEP	0x01			/* Save errored packets */
#define		AR	0x02			/* Accept runt packets */
#define		AB	0x04			/* Accept broadcast */
#define		AM	0x08			/* Accept multicast */
#define		PRO	0x10			/* Promiscuous physical */
#define		MON	0x20			/* Monitor mode */
#define CNTR0	(device->base+0x0d)	/* Frame alignment errors (r) */
#define TCR	(device->base+0x0d)	/* Transmit configuration (w) */
#define		CRC	0x01			/* Inhibit CRC */
#define		LB0	0x02			/* Encoded loopback control */
#define		LB1	0x04
#define		ATD	0x08			/* Auto-transmit disable */
#define		OFST	0x10			/* Collision offset enable */
#define CNTR1	(device->base+0x0e)	/* CRC errors (r) */
#define DCR	(device->base+0x0e)	/* Data configuration (w) */
#define		WTS	0x01			/* Word transfer select */
#define		BOS	0x02			/* Byte order select */
#define		LAS	0x04			/* Long address select */
#define		BMS	0x08			/* Burst mode select */
#define		ARM	0x10			/* Autoinitialize remote */
#define		FT0	0x20			/* FIFO threshold select */
#define		FT1	0x40
#define CNTR2	(device->base+0x0f)	/* Missed packet errors (r) */
#define IMR	(device->base+0x0f)	/* Interrupt mask (w) */
#define		PRXE	0x01			/* Packet received */
#define		PTXE	0x02			/* Packet transmitted */
#define		RXEE	0x04			/* Receive error */
#define		TXEE	0x08			/* Transmit error */
#define		OVWE	0x10			/* Overwrite warning */
#define		CNTE	0x20			/* Counter overflow */
#define		RDCE	0x40			/* DMA complete */
/* page 1 registers */
#define PAR0	(device->base+0x01)	/* Physical address (r/w) */
#define CURR	(device->base+0x07)	/* Current page (r/w) */
#define MAR0	(device->base+0x08)	/* Multicast address (r/w) */

/* Ethernet Address PROM, Control Register bits 3=0, 2=1 */

#define	EADDR	(device->base) 		/* Address (read & write)*/

/* Gate Array  */

#define PSTR	(device->base+0x400)	/* Page Start Register (r/w) */
#define PSPR	(device->base+0x401)	/* Page Stop Register (r/w) */
#define DQTR	(device->base+0x402)	/* Drq Timer Register (r/w) */
#define BCFR	(device->base+0x403)	/* Base Configuration Register (ro) */
#define PCFR	(device->base+0x404)	/* EPROM Configuration Register (ro) */
#define GACFR	(device->base+0x405)	/* Ga Configuration Register (r/w) */
#define		MBS0	0x01			/* Memory bank select 0 */
#define		MBS1	0x02			/* Memory bank select 1 */
#define		MBS2	0x04			/* Memory bank select 2 */
#define		REL	0x08			/* RAM select */
#define		TEST	0x10			/* Test */
#define		OWS	0x20			/* Zero wait state */
#define		TCM	0x40			/* Terminal count mask */
#define		NIM	0x80			/* NIC int mask */
#define CTRL	(device->base+0x406)	/* Control Register (r/w) */
#define		SRST	0x01			/* Software reset */
#define		XSEL	0x02			/* Transceiver select */
#define		EALO	0x04			/* Ethernet address low */
#define		EAHI	0x08			/* Ethernet address high */
#define		SHARE	0x10			/* Interrupt share */
#define		DBSEL	0x20			/* Double buffer select */
#define		DDIR	0x40			/* DMA direction */
#define		START	0x80			/* Start DMA controller */
#define STREG	(device->base+0x407)	/* Status Register (ro) */
#define		REV	0x07			/* Ga revision */
#define		DIP	0x08			/* DMA in progress */
#define		DTC	0x10			/* DMA terminal count */
#define		OFLW	0x20			/* Overflow */
#define		UFLW	0x40			/* Underflow */
#define		DPRDY	0x80			/* Data port ready */
#define IDCFR	(device->base+0x408)	/* Interrupt/DMA Configuration Register (r/w) */
#define		DRQ1	0x01			/* DMA request 1 */
#define		DRQ2	0x02			/* DMA request 2 */
#define		DRQ3	0x04			/* DMA request 3 */

#define		IRQ2	0x09			/* Interrupt request 2 */
#define		IRQ3	0x20			/* Interrupt request 3 */
#define		IRQ4	0x40			/* Interrupt request 4 */
#define		IRQ5	0x80			/* Interrupt request 5 */

#define DAMSB	(device->base+0x409)	/* DMA Address Register MSB (r/w) */
#define DALSB	(device->base+0x40a)	/* DMA Address Register LSB (r/w) */
#define VPTR2	(device->base+0x40b)	/* Vector Pointer Register 2 (r/w) */
#define VPTR1	(device->base+0x40c)	/* Vector Pointer Register 1 (r/w) */
#define VPTR0	(device->base+0x40d)	/* Vector Pointer Register 0 (r/w) */
#define RFMSB	(device->base+0x40e)	/* Register File Access MSB (r/w) */
#define RFLSB	(device->base+0x40f)	/* Register File Access LSB (r/w) */

#define	EDEVSIZ	(0x10 * sizeof(char))
/*
 * Total buffer size, is therefore 1566. We allow 2000.
 */

#define TX_BUFBASE	0x20
#define RX_BUFBASE	0x26
#define RX_BUFLIM	0x40
#define NXT_RXBUF(p)	((p)==RX_BUFLIM-1 ? RX_BUFBASE : (p)+1)
#define PRV_RXBUF(p)	((p)==RX_BUFBASE ? RX_BUFLIM-1 : (p)-1)
#define CURRXBUF(t)	(outb(CR, PS0|RD2|STA), t=inb(CURR), outb(CR, RD2|STA), t)

#define E3COM_MINPACK    60     /* minimum output packet length */
#define E3COM_MAXPACK  1514	/* maximum output packet length */

/* transfer limits */

#define E503ETHERMIN		(E3COM_MINPACK)
#define E503ETHERMTU		(E3COM_DATA)

/* Statistics structure */

struct e503localstats {
	uint allocfails;
	uint badintr;
	uint badrstat;
	uint cdheartbt;
	uint colls;
	uint crcerrs;
	uint cslost;
	uint ethwput;
	uint etint;
	uint excolls;
	uint fifoorun;
	uint fifourun;
	uint fralignerrs;
	uint missintr;
	uint misspkts;
	uint overwr;
	uint owcolls;
	uint restarts;
	uint rdmaint;
	uint rxct;
	uint sftpkts;
	uint txct;
	uint txerrs;
};

/* The minor device structure */

struct en_minor {
	queue_t *em_top;
	unsigned short em_sap;
	unsigned int em_min;
	unsigned int em_unit;
};

struct e503device {
#ifdef DYNAMIC
#define RX_MBLKS	64
	mblk_t *mblk[RX_MBLKS];
#endif
	unsigned int base;
	unsigned int flags;
	unsigned int tid;		/* TX timeout */
	struct e503localstats e503stats;
	struct ifstats  ifstats;	/* BSD style stats */
};

/* device flags */
#define E503BUSY	0x01
#define E503WAITO	0x02

/* miscellany */
#define OK	1
#define NOT_OK	0

#define TX_TIMEOUT	(5*HZ)
#define WATCHDOG2	(5*HZ)

#define E503IMASK (PRXE|PTXE|RXEE|TXEE|OVWE|CNTE)

#ifdef _KERNEL
extern e_addr e503eaddr[];
extern struct e503device e503device[];
extern struct en_minor e503_em[];
extern unsigned int e503iobase[];
extern unsigned int e503intl[];
extern unsigned int e503xcvr[];     /* Transceiver select */
extern unsigned int e503afterboot[];/* first time through after boot */
extern unsigned int e503inited[];   /* first time through for open indication */
extern unsigned int e503nopens[];   /* number of opens */
extern unsigned int e503major[];    /* unit major numbers */
extern unsigned int n3c503unit;
extern unsigned int n3c503min;
#endif
