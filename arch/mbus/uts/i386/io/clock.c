/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1983, 1986, 1987, 1988  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:uts/i386/io/clock.c	1.3"

#ifndef lint
static char clock_copyright[] = "Copyright 1983,1986,1988 Intel Corp. 463030";
#endif

#include "sys/types.h"
#include "sys/param.h"
#include "sys/errno.h"
#include "sys/cred.h"
#include "sys/rtc.h"
#include "sys/clockcal.h"



struct clknm {
	int   n_clkid;
	dev_t n_dev;
	struct clkrtn n_clkrtn;
} clknm = { 0, 0, { NULL, NULL, NULL, NULL}}; 

#define bcdtoi(byte) ((((byte)>>4)*10)+((byte)&0xF))
#define itobcd(byte) ((((byte)/10)<<4)+((byte)%10))

#define VALID_HOUR(x)	 	((x >= 0) && (x < 24)) ? 1 : 0 
#define VALID_MONTH(x)	   	((x >= 1) && (x <= 12)) ? 1 : 0 

int clkdevflag = 0;		/* V4 ddi/dki driver */

/* ARGSUSED */
clkopen (dev, flag, otyp, cred_p)
dev_t *dev;
int flag, otyp;
struct cred *cred_p;
{
	return 0;
}

/* ARGSUSED */
clkclose (dev, flag, otyp, cred_p)
dev_t *dev;
int flag, otyp;
struct cred *cred_p;
{
	return 0;
}

/*
 * routine to convert from rtc format to 546 clock format
 */

void
clk_rtc_to_cal (rclk, clkcal)
struct rtc_t *rclk;
struct clkcal *clkcal;
{
	clkcal->ck_milsec = clkcal->ck_decsec = 0;
	clkcal->ck_sec = rclk->rtc_sec;	
	clkcal->ck_min = rclk->rtc_min;
	clkcal->ck_hour = rclk->rtc_hr;
	clkcal->ck_wday = rclk->rtc_dow;
	clkcal->ck_mday = rclk->rtc_dom;
	clkcal->ck_month = rclk->rtc_mon;
	clkcal->ck_reserv1 = 0;
	clkcal->ck_year = rclk->rtc_yr;
	clkcal->ck_reserv2 = 0;
	return;
}

/*
 * routine to convert from 546 clock format to at386 rtc format
 */

void
clk_cal_to_rtc (clkcal, rclk)
struct clkcal *clkcal;
struct rtc_t *rclk;
{
	rclk->rtc_sec = clkcal->ck_sec; 
	rclk->rtc_asec = 0; 
	rclk->rtc_min = clkcal->ck_min;
	rclk->rtc_amin = 0;
	rclk->rtc_hr = clkcal->ck_hour;
	rclk->rtc_ahr = 0;
	rclk->rtc_dow = clkcal->ck_wday;
	rclk->rtc_dom = clkcal->ck_mday;
	rclk->rtc_mon = clkcal->ck_month;
	rclk->rtc_yr = clkcal->ck_year;
	rclk->rtc_statusa = rclk->rtc_statusb = rclk->rtc_statusc = rclk->rtc_statusd = 0;
	return;
}

/*
 * routine to convert from at386 rtc format to csmclk format
 */

void
clk_rtc_to_csm (rclk, csmclk)
struct rtc_t *rclk;
struct csmclk *csmclk;
{
	
	csmclk->csm_msec = 0;	
	csmclk->csm_sec = rclk->rtc_sec;
	csmclk->csm_min = rclk->rtc_min;
	csmclk->csm_hour = rclk->rtc_hr;
	csmclk->csm_mday = rclk->rtc_dom;		
	csmclk->csm_month = rclk->rtc_mon;
	csmclk->csm_year1 = rclk->rtc_yr;
	csmclk->csm_year2 = 0;
	return;
}


/*
 * routine to convert from csmclk format to at386 rtc format 
 */

void
clk_csm_to_rtc (csmclk, rclk)
struct csmclk *csmclk;
struct rtc_t *rclk;
{
	rclk->rtc_sec = csmclk->csm_sec;
	rclk->rtc_asec = 0;
	rclk->rtc_min = csmclk->csm_min;
	rclk->rtc_amin = 0;
	rclk->rtc_hr = csmclk->csm_hour;
	rclk->rtc_ahr = 0;
	rclk->rtc_dom = csmclk->csm_mday;
	rclk->rtc_mon = csmclk->csm_month;
	rclk->rtc_yr = csmclk->csm_year1;
	rclk->rtc_statusa = rclk->rtc_statusb = rclk->rtc_statusc = rclk->rtc_statusd = 0;
	return;
}


/*
 * exported routine to stime and other kernel routines to read the 
 * multibus clock
 */

/* ARGSUSED */
int
clkget (clk, flag)
struct rtc_t *clk;
int flag;		/* not used */
{
	struct clkcal clkcal;
	struct csmclk csmclk;
	int ret, oc_ret;
	dev_t dev;

	if (clknm.n_clkid == 0)  /* Zero is an invalid clk server id */
		return (-1);

	dev = clknm.n_dev;
	oc_ret = (*clknm.n_clkrtn.c_open)(dev, 0);
	if (oc_ret != 0)
		return (-1);
	ret = 0;
	switch (clknm.n_clkid) {
	case CLK_I546:
		ret = (*clknm.n_clkrtn.c_read)(dev, &clkcal, IN_KERNEL);
		if (ret == 0) {
			/*
 		 	 * Check if the clock value returned is valid.
 		 	 * Note: This is against first principles to interpret 
		 	 * any data in the kernel. This should be left to the 
		 	 * user to do this. But...
 		 	 */
			if (!(VALID_HOUR((int)bcdtoi(clkcal.ck_hour))) ||
				!(VALID_MONTH ((int)bcdtoi(clkcal.ck_month)))) {
				ret = 1;
				break;
			}
			(void) clk_cal_to_rtc (&clkcal, clk);
		}
		break;
	case CLK_ICSM: 
		ret = (*clknm.n_clkrtn.c_read)(dev, &csmclk, IN_KERNEL);
		if (ret == 0) {
			if (!(VALID_HOUR((int)bcdtoi(csmclk.csm_hour))) ||
				!(VALID_MONTH ((int)bcdtoi(csmclk.csm_month)))) {
				ret = 1;
				break;
			}
			(void) clk_csm_to_rtc (&csmclk, clk);
		}
		break;
	default:
		ret = 1;
		break;
	}
	oc_ret = (*clknm.n_clkrtn.c_close)(dev, 0);
	if ((oc_ret != 0) || (ret != 0))
		return (-1);
	return (0);
}

/*
 * exported routine to stime and other kernel routines to write the 
 * multibus clock
 */

clkput (rclk)
struct rtc_t *rclk;
{
	struct clkcal clkcal;
	struct csmclk csmclk;
	int ret, oc_ret;
	dev_t dev;

	if (clknm.n_clkid == 0)  /* Zero is an invalid clk server id */
		return (-1);

	dev = clknm.n_dev;
	oc_ret = (*clknm.n_clkrtn.c_open)(dev, 0);
	if (oc_ret != 0)
		return (-1);
	ret = 0;
	switch (clknm.n_clkid) {
	case CLK_I546:
		(void) clk_rtc_to_cal (rclk, &clkcal);
		ret = (*clknm.n_clkrtn.c_write)(dev, &clkcal, IN_KERNEL);
		break;
	case CLK_ICSM: 
		(void) clk_rtc_to_csm (rclk, &csmclk);
		ret = (*clknm.n_clkrtn.c_write)(dev, &csmclk, IN_KERNEL);
		break;
	default:
		ret = 1;
		break;
	}
	oc_ret = (*clknm.n_clkrtn.c_close)(dev, 0);
	if ((oc_ret != 0) || (ret != 0))
		return (-1);
	return (0);
}

/*
 * this routine performs a type of name service function for the clock device
 * servers.
 */

void
clk_log (clkid,  dev_no, clkrtn) 
int clkid;
dev_t dev_no;
struct clkrtn *clkrtn;
{
	clknm.n_clkid = clkid;
	clknm.n_clkrtn.c_open = clkrtn->c_open;
	clknm.n_clkrtn.c_close = clkrtn->c_close;
	clknm.n_clkrtn.c_read = clkrtn->c_read;
	clknm.n_clkrtn.c_write = clkrtn->c_write;
	clknm.n_dev = dev_no;
	return;
}

/*
 * This is used to read and write the clock from /dev/rtc.
 */

/* ARGSUSED */
clkioctl(dev, cmd, addr, mode, cred_p, rval_p)
dev_t dev;
int cmd;
caddr_t addr;
int mode;
struct cred *cred_p;
int *rval_p;
{
	struct rtc_t rtc;

	switch (cmd) {
	case RTCRTIME:
		if (clkget(&rtc, 0)) {
			return(EIO);
			break;
		}
		if (copyout((caddr_t)&rtc, addr, RTC_NREG)) {
			break;
		}
		break;
	case RTCSTIME:
		if (!drv_priv(cred_p)) {
			return (EACCES);
			break;
		}
		if (copyin(addr, (caddr_t)&rtc, RTC_NREGP))
			break;

		if (clkput(&rtc) != 0) {
			return (EACCES);
			break;
		}
		break;
	default:
		return (EINVAL);
		break;
	}
	return (0);
}
