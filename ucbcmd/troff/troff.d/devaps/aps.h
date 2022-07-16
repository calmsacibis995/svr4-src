/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#ident	"@(#)ucbtroff:troff.d/devaps/aps.h	1.1.1.1"

/*******************************************************************

		PROPRIETARY NOTICE (Combined)

This source code is unpublished proprietary information
constituting, or derived under license from AT&T's UNIX(r) System V.
In addition, portions of such source code were derived from Berkeley
4.3 BSD under license from the Regents of the University of
California.



		Copyright Notice 

Notice of copyright on this source code product does not indicate 
publication.

	(c) 1986,1987,1988,1989  Sun Microsystems, Inc
	(c) 1983,1984,1985,1986,1987,1988,1989  AT&T.
	          All rights reserved.
********************************************************************/ 


#define APSNOOP		00
#define HSP25U		0201
#define HSP400U		0202
#define HSP200U		0203
#define HSP100U		0204
#define HSP50U		0205
#define HSP72P		0206
#define HSP36P		0207
#define HSP18P		0210
#define HSP9P		0211
#define HSP4.5P		0212
#define MARKLOC		0217
#define RTNMARK		0220
#define DRAW		0221
#define RTNMARKX	0222
#define RTNMARKY	0223
#define SPEED		0240
#define DCLMSZ		0241
#define SETOBLIQUE	0242
#define WRREAD		0243
#define LFTMGN		0244
#define RETRACE		0245
#define UPDOWN		0246
#define DISPFONT	0257
#define FONT		0260
#define HSIZE		0261
#define VSIZE		0262
#define HVSIZE		0263
#define REPEAT		0264
#define SAVSTR		0265
#define EXSTR		0266
#define SETCTR		0267
#define SETPTR		0270
#define STRTJOB		0272
#define STRTPG		0273
#define SELFSCAN	0274
#define VSSPOTS		0276
#define HSSPOTS		0277
#define VSPABS		0300
#define VSPREL		0301
#define HSPABS		0302
#define HSPREL		0303
#define LTRSP		0304
#define LTRSPREL	0305
#define PGLEAD		0313
#define HPSNABS		0315
#define VPSNABS		0316
#define SKIP		0317
#define STORVERT	0320
#define STORVTSPOT	0321
#define SETTAB		0323
#define STRVRA		0325
#define STRHRA		0326
#define STRVRB		0327
#define STRHRB		0330
#define STORHSPA	0332
#define STORHSPB	0333
#define STORHSPC	0334
#define STORHSPD	0335
#define MKH		0336
#define RTNHMK		0337
#define EXVSP		0340
#define RTN		0341
#define CRTN		0342
#define XTAB		0343
#define XTABCR		0344
#define XRULA		0345
#define XRULATAB	0346
#define XRULB		0347
#define XNORMAL		0350
#define XOBLIQUE	0351
#define XHSPA		0352
#define XHSPB		0353
#define XHSPC		0354
#define XHSPD		0355
#define RESET		0356
#define DISPFNTS	0357
#define ENDSTR		0360
#define DECCTR		0361
#define MKV		0362
#define RTNVMK		0363
#define ESCGRAPHICS	0366
#define CUT		0374
#define HALT		0375
#define ENDJOB		0376

#define MAXESCAPE 8400
#define MAXLEAD 32367
