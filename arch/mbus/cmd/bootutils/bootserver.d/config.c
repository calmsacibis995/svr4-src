/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1988, 1989  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:cmd/bootutils/bootserver.d/config.c	1.3"

/************************************************
 *    BootServer configuration file reading utilities.
 *
 * Included herein are:
 *		ConfigInit	-- Initialize this module with configuration filename
 *		GetHostAndMethod--Check the existance of host information and
 *				  booting method.
 *		GetBPString	-- Get the BPS for a host
 *		GetBootLoader-- Get the boot loading file for a host
 *		ConfigClose	-- Un-initialize this module
 *
 ************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "bootserver.h"
extern void free();
extern char *sys_errlist[];
extern int errno;

/* these are the maximum values as specified in the functional spec */
#define MAXCONFIGLINE 4096
#define MAXVALUELEN 257

static char ConfigFilename[1024];	/* name of the configuration file */
static long ConfigModify;			/* date of last modification */
static FILE *cfile;		/* handle to the opened configuration file */

static char *GlobServer;			/* global server information */
static char *GlobClient;			/* global client information */
static char *HostServer;			/* host server information */
static char *HostClient;			/* host client information */
static int HostHost;		/* who the above server/client info is for */

static char TheBPS[MAXCONFIGLINE+MAXCONFIGLINE];

/* control for input character fetching */
#define STRNG 1				/* fetch characters from string */
#define CNFG 0				/* fetch characters from configruation file */
#define MAXSAVE 5
static int getflag = CNFG;	/* source we're getting characters from */
static char *getpntr;		/* pointer to character to fetch next */
static int savegetflag[MAXSAVE];	/* stack of higher fetching */
static char *savegetpntr[MAXSAVE];
static int getsave = 0;		/* level that character status is saved at */

static int ReadHostInfo();
static int Xgetc();
static int Xungetc();
static void CheckConfigAge();
static void XGetParamValue();
static void XScanWhiteSpace();
static void XGetTerminator();
static void XScanParam();
static void XScanValue();
static void XAddElement();
static char *GetValue();
static void XInitString();
static void XPushGetc();
static void XPopGetc();

#ifdef DEBUGCONFIG
static int DebugConfig = DEBUGCONFIG;
#	define DDEBUG(x,y) if (((x)|1)&DebugConfig)printf y
#	define DALL 0x01	/* come out all the time */
#	define DMAIN 0x02	/* main routine entries */
#	define DCLSR 0x04	/* client server parsing - ReadHostInfo */
#	define DVALUE 0x08	/* Value extraction */
#	define DDOLR 0x10	/* Dollar extraction and replacement */
#else
#	define DDEBUG(x,y)
#endif

/* flags for saying whether backslashes and dollars are resolved in value */
#define NORESOLVE 0
#define RESOLVE 1

/************************************************
 *	ConfigInit -- Initialize with the name of the config file
 *
 *  Passed: Pointer to name of the configuration file
 *  Returns: -1 if cannot open configuration file for reading
 ************************************************/
int
ConfigInit(Filename)
	char *Filename;
{
	GlobServer = (char *)NULL;
	GlobClient = (char *)NULL;
	HostServer = (char *)NULL;
	HostClient = (char *)NULL;

	ConfigModify = 0L;
	(void)strcpy(&ConfigFilename[0], Filename);
	cfile = fopen(ConfigFilename, "r");
	if (cfile == NULL) {
		/* cannot open the configuration file */
		return(-1);
	}
	(void)fclose(cfile);
	return(0);
}

/************************************************
 *		GetBPString -- get BPS for a host
 * Scans through the configuration file for the BSP for this
 *   host and returns the BPS for him.
 *
 * Passed: hostID to get boot parameter string for
 * Returns: pointer to the fetched string, or
 *          NULL if no information for this host
 *
 ************************************************/
char *
GetBPString(hostID)
	unsigned short hostID;
{
	if (ReadHostInfo(hostID) < 0) {
		/* no infomation for this host */
		return((char *)NULL);
	}
	(void)strcat(&TheBPS[0], GlobClient);
	(void)strcat(&TheBPS[0], ";");
	(void)strcpy(&TheBPS[0], HostClient);
	DDEBUG(DMAIN,("GetBPString: hostID=%d, BPS='%s'\n",hostID,TheBPS));
	return(&TheBPS[0]);
}

/************************************************
 *		GetHostAndMethod -- Say if we have info for this host
 * Scan the configuration file and return a flag saying
 * whether there is booting information for this host.
 *
 * Passed: hostID to lookup
 * Returns: NO_HOSTID if no information for hostID 
 *				else
 *          NO_METHOD if no BL_method specified in configuration file
 *          DEP_METHOD if BL_method=dependent specified in configuration file
 *          QUA_METHOD if BL_method=quasi specified in configuration file
 ************************************************/
int
GetHostAndMethod(hostID)
	unsigned short hostID;
{
	int i;
	char *method;
	char lowstr[MAX_BL_METH+1];

	if (ReadHostInfo(hostID) < 0) {
		/* host information does not exist -- return false */
		DDEBUG(DMAIN,("GetHostAndMethod: no info for hostID=%d\n",hostID));
		return(NO_HOSTID);
	} 
	/* the host exists -- return true */
	DDEBUG(DMAIN,("GetHostAndMethod: info for hostID=%d\n",hostID));
	method = GetValue(HostServer, "BL_method");
	if (method == NULL) {
		method = GetValue(GlobClient, "BL_method");
		if (method == NULL) {
			DDEBUG(DMAIN,("GetHostAndMethod: no method for hostID=%d\n",hostID));
			return(NO_METHOD);
		}
	}
	for (i=0; i<MAX_BL_METH; i++, method++) {
		lowstr[i] = tolower(*method);
	}
	lowstr[i] = '\0';
	if (strncmp(lowstr, "dep",MAX_BL_METH) == 0) {
		DDEBUG(DMAIN,("GetHostAndMethod: dependent boot for hostID=%d\n",hostID));
		return(DEP_METHOD);
	}
	if (strncmp(lowstr, "qua",MAX_BL_METH) == 0) {
		DDEBUG(DMAIN,("GetHostAndMethod: quasi boot for hostID=%d\n",hostID));
		return(QUA_METHOD);
	}
		DDEBUG(DMAIN,("GetHostAndMethod: invalid method for hostID=%d\n",hostID));
		return(NO_METHOD);
}

/************************************************
 *		GetBootLoader -- Get name of second stage file for this hostID
 * Scan the configuration file for this hostID.  Return the value of
 * "BL_second_stage" for that host.  If no such symbol, lookup the
 * same symbol in the global server information for the default.
 * If not there either, return the default filename for UNIX.
 *
 * Passed: hostID to return second stage bootloader file
 * Returns: pointer to null terminated string
 ************************************************/
char *
GetBootLoader(hostID)
	unsigned short hostID;
{
	char *bsn;

	DDEBUG(DMAIN,("GetBootLoader: entry: hostID=%d\n", hostID));
	if (ReadHostInfo(hostID) < 0) {
		/* no information for this host */
		DDEBUG(DMAIN,("GetBootLoader: no info: hostID=%d\n", hostID));
		return((char *)NULL);
	}
	bsn = GetValue(HostServer, "BL_second_stage");
	if (bsn == NULL) {
		bsn = GetValue(GlobClient, "BL_second_stage");
		if (bsn == NULL) {
			bsn = "/etc/default/bootserver/secondstage";
		}
	}
	DDEBUG(DMAIN,("GetBootLoader: hostID=%d, file='%s'\n", hostID, bsn));
	return(bsn);
}

/************************************************
 *		ConfigClose -- un initialize this module
 *
 * Provides a clean external interface.  Actually just forgets
 * the configuration filename passed by initialization.
 ************************************************/
void
ConfigClose()
{
	ConfigModify = 0L;
	ConfigFilename[0] = '\0';
}

/************************************************
 *	ReadHostInfo -- Read information for this host
 * The configuration file is checked for aging.  If
 * the data therein is still good, and we have the
 * data in memory, we continue to use it.  Otherwise,
 * the config file  is read for the information for
 * this host.
 *
 * Passed: hostID to lookup
 * Returns: integer 1 if information found for this host
 *                  0 if no information for this host
 *          global variables modified:
 *		GlobServer - GLOBAL server information (sans []'s)
 *		GlobClient - GLOBAL client information
 *		HostHost   - the hostID information is for
 *		HostServer - hostID's server information (sans []'s)
 *		HostClient - hostID's client information
 *
 ************************************************/
int
ReadHostInfo(hostID)
	unsigned short hostID;
{
	static void FlushKnownHostConfig();
	static int GetClientServer();
	static int BPI_numeric();
	char *bvp;
	char Srv[MAXCONFIGLINE];
	char Clnt[MAXCONFIGLINE];

	Srv[0] = '\0';
	Clnt[0] = '\0';
	CheckConfigAge();
	while ((int)hostID != HostHost) {
		FlushKnownHostConfig();
		if (GetClientServer(&Srv[0], &Clnt[0]) < 0) {
			break;
		}
		DDEBUG(DCLSR,("ReadHostInfo: srv='%s', clnt='%s'\n",Srv,Clnt));
		bvp = GetValue(Srv, "BL_host_id");
		DDEBUG(DCLSR,("ReadHostInfo: sym='BL_host_id', val='%s'\n",bvp));
		if (bvp != NULL) {
			if (strcmp(bvp, "GLOBAL") == 0) {
				if (GlobServer == NULL) {
					DDEBUG(DCLSR,("ReadHostInfo: GLOBAL\n"));
					/* global parameters */
					GlobServer = strdup(Srv);
					GlobClient = strdup(Clnt);
				}
			} else {
				DDEBUG(DCLSR,("ReadHostInfo: this host? bvp='%s', hostID=%d\n",
						bvp, hostID));
				if (BPI_numeric(bvp) == hostID) {
					/* the entry for this host */
					DDEBUG(DCLSR,("ReadHostInfo: info for hostID=%d\n",hostID));
					FlushKnownHostConfig();
					HostServer = strdup(Srv);
					HostClient = strdup(Clnt);
					HostHost = hostID;
				}
			}
		}
	}
	(void)fclose(cfile);
	if ((int)hostID == HostHost) {
		return(0);
	} else {
		return(-1);
	}
}

/************************************************
 *	CheckConfigAge -- see if the config file changed since last look
 * See if our configuration information is correct and if
 * not, clear out the known info.  This also opens the
 * configuration file.
 *
 * Passed: nothing
 * Returns: nothing
 *          global variables modified:
 *      If file is older then GlobServer, GlobClient, HostServer,
 *        are cleared HostClient.
 ************************************************/
void
CheckConfigAge()
{
	static void FlushKnownConfig();
	struct stat thestat;

	cfile = fopen(ConfigFilename, "r");
	if (cfile == (FILE *)NULL) {
		return;
	}
	(void)fstat(fileno(cfile), &thestat);
	if (thestat.st_mtime > ConfigModify) {
		FlushKnownConfig();
		ConfigModify = thestat.st_mtime;
	}
	getsave = 0;
}

/************************************************
 *	GetClientServer -- scan forward in config file for next
 *		client server parameter pair.
 *
 * Passed: place to return next client and server strings
 * Returns: 0 if there is another pair to return
 *          -1 if we've reached the end of file
 ************************************************/
static int
GetClientServer(srv, clnt)
	char *srv;	/* place to return client information */
	char *clnt; /* place to return server information */
{
	static void XInitConfig();
	static void AddParamValue();
	char tparam[MAXVALUELEN];
	char tvalue[MAXVALUELEN];
	int tterm;

	*srv = '\0';
	*clnt = '\0';
	XInitConfig();	/* information from configuration file */
	/* CONSTANTCONDITION */
	while (1) {
		/* skip over leading junk until server info found */
		XGetParamValue(&tparam[0], &tvalue[0], &tterm);
		DDEBUG(DCLSR,("GetClientServer: leading: prm='%s' val='%s' term='%c'\n",
				tparam,tvalue,tterm));
		if (tterm == '[' || tterm == EOF) {
			break;
		}
	}
	/* CONSTANTCONDITION */
	while (1) {
		/* suck up server info */
		XGetParamValue(&tparam[0], &tvalue[0], &tterm);
		DDEBUG(DCLSR,("GetClientServer: server: prm='%s' val='%s' term='%c'\n",
				tparam,tvalue,tterm));
		AddParamValue(&tparam[0], &tvalue[0], &srv[0]);
		if (tterm == ']' || tterm == EOF) {
			break;
		}
	}
	/* CONSTANTCONDITION */
	while (1) {
		/* suck up client info */
		XGetParamValue(&tparam[0], &tvalue[0], &tterm);
		DDEBUG(DCLSR,("GetClientServer: client: prm='%s' val='%s' term='%c'\n",
				tparam,tvalue,tterm));
		AddParamValue(&tparam[0], &tvalue[0], &clnt[0]);
		if (tterm == '[' || tterm == EOF) {
			break;
		}
	}
	/* this saves the terminator for the next time around */
	(void)Xungetc(tterm);
	if (strlen(srv) == 0 && strlen(clnt) == 0 && tterm == EOF) {
		return(-1);
	} else {
		return(0);
	}
}

/************************************************
 *	XGetParamValue -- get a parameter and value from config line
 * Scan forward in the input stream and get the next parameter
 * value pair and it's terminator.  A zero length value denotes
 * no value (a null string would be quoted).  The terminator is
 * removed from the input stream.  A zero length parameter would
 * denote no parameter and the terminator is the only real data.
 *
 * Passed: place to return the parameter, value, and terminator.
 * Returns: the parameter, value, and terminator.
 *
 ************************************************/
void
XGetParamValue(param, value, term)
	char *param;	/* place to return parameter */
	char *value;	/* place to return value */
	int *term;		/* place to return terminating character */
{
	int lterm;
	char vparam[MAXVALUELEN];
	char vvalue[MAXVALUELEN];

	*param = '\0';
	*value = '\0';
	vparam[0] = '\0';
	vvalue[0] = '\0';
	XScanWhiteSpace(&lterm);
	XScanParam(&vparam[0]);
	XScanWhiteSpace(&lterm);
	if (lterm == '=') {
		XGetTerminator(&lterm);
		XScanWhiteSpace(&lterm);
		XScanValue(&vvalue[0],NORESOLVE);
		XScanWhiteSpace(&lterm);
	}
	XGetTerminator(&lterm);
	(void)strcpy(param, vparam);
	(void)strcpy(value, vvalue);
	*term = lterm;
	return;
}

/************************************************
 *		ScanWhiteSpace -- scan over white space
 * The input stream is scanned passed any white space chars.
 * The terminator of the whitespace is left in the input
 * stream so that the next fetch will get it.
 *
 * Passed: pointer to location to place terminator
 * Returns: the whitespace terminator.
 ************************************************/
void
XScanWhiteSpace(term)
	int *term;
{
	int c;

	/* CONSTANTCONDITION */
	while (1) {
		c = Xgetc();
		if (c == '#') {
			/* this is one of those comment things */
			/* CONSTANTCONDITION */
			while (1) {
				c = Xgetc();
				if (c == '\n' || c == EOF) {
					break;
				}
			}
			continue;
		}
		if ((c!=' ' && c!='\t' && c!='\n' && c!='\r') || c == EOF) {
			(void)Xungetc(c);
			break;
		}
	}
	*term = c;
	return;
}

/************************************************
 *		XGetTerminator -- remove the terminator from the input stream.
 * Used after a XScanWhiteSpace to get the terminator
 * out of the input stream.
 *
 * Passed: character pointer to where to return the terminator
 * Returns: the terminator
 ************************************************/
void
XGetTerminator(mterm)
	int *mterm;
{
	*mterm = Xgetc();
}

/************************************************
 *		XScanParameter -- Scan over a parameter
 * After a call to ScanWhiteSpace, this call will scan over a
 * parameter and copy that parameter to the specified place.
 * The terminator is left in the input stream for the next fetch.
 * A zero length parameter means none scanned.
 *
 * Passed: pointer to buffer to receive the parameter
 * Returns: the parameter in the buffer
 ************************************************/
void
XScanParam(param)
	char *param;	/* parameter scanned */
{
	int c;

	/* CONSTANTCONDITION */
	while (1) {
		c = Xgetc();
		if (isalnum(c) || c == '_') {
			*(param++) = (char)c;
		} else {
			break;
		}
	}
	*param = '\0';
	(void)Xungetc(c);
}

/************************************************
 *		XScanValue -- scan over a value and copy into buffer
 * After a call to ScanWhiteSpace, this will scan over a value
 * and copy it into a supplied buffer.  'ResolveFlag' will specifiy
 * whether replacement parameters (beginning with dollar signs)
 * are resolved and whether escaped characters (backslashed) are
 * left in the value.
 *
 * Passed: pointer to buffer to copy the value string.
 * Returns: value in buffer
 ************************************************/
void
XScanValue(value,ResolveFlag)
	char *value;	/* value scanned */
	int ResolveFlag;
{
	int tterm;

	/* CONSTANTCONDITION */
	while (1) {
		XAddElement(value,ResolveFlag);
		XScanWhiteSpace(&tterm);
		DDEBUG(DCLSR,("XScanValue: value='%s', term='%c'\n",value,tterm));
		if (tterm == EOF || strchr(";[]\377", tterm) != NULL) {
			break;
		}
	}
	(void)Xungetc(tterm);
}

/************************************************
 *		XAddElement -- Add a value element to a building value
 * The next element of a value is copied from the input stream
 * to the passed buffer.  The result is a NUL terminated value.
 * 'ResolveFlag' says whether to expand replacement parameters
 * and whether to leave character escapes in the value.
 * The terminator is left in the imput stream.
 *
 * Passed: point to buffer to add value element to
 *         flag to say whether to resolve replacments and escapes
 * Returns: longer value in buffer
 ************************************************/
void
XAddElement(val,ResolveFlag)
	char *val;
	int ResolveFlag;
{
	int c;
	int mt;
	char *cp;
	char ParamName[MAXVALUELEN];

	if (strlen(val) != 0) {
		(void)strcat(val, " ");
		val += strlen(val);
	}
	c = Xgetc();
	switch (c) {
	case '$':
		/* parameter type of thing */
		if (ResolveFlag) {
			XScanParam(&ParamName[0]);
			cp = GetValue(GlobClient, ParamName);
			DDEBUG(DDOLR,("AddElement: param='%s', val='%s'\n",ParamName,cp));
			if (cp == NULL) {
				/* the symbol not found -- just leave it as it was */
				*(val++) = '$';
				(void)strcat(val, ParamName);
			} else {
				/* symbol found -- append new value */
				(void)strcat(val, cp);
			}
		} else {
			*(val++) = (char)c;
			XScanParam(val);
		}
		val += strlen(val);
		break;
	case '\\':
		/* escaped character */
		if (!ResolveFlag) {
			*(val++) = (char)c;
		}
		*(val++) = Xgetc();
		break;
	case '"':
	case '\'':
		/* quoted string */
		mt = c;
		*(val++) = (char)mt;
		/* CONSTANTCONDITION */
		while (1) {
			c = Xgetc();
			if (c == '\\') {
				/* escape next character whatever it is */
				if (!ResolveFlag) {
					*(val++) = (char)c;
				}
				c = Xgetc();
			} else {
				if (c == mt) {
					/* found quoting character */
					c = Xgetc();
					/* check for doubling which means just one */
					if (c != mt || c == EOF) {
						(void)Xungetc(c);
						break;
					}
				} else {
					if (c == EOF) {
						break;
					}
				}
			}
			*(val++) = (char)c;
		}
		*(val++) = (char)mt;
		break;
	default:
		/* just a stream of characters */
		/* CONSTANTCONDITION */
		while (1) {
			if (c == '\\') {
				if (!ResolveFlag) {
					*(val++) = (char)c;
				}
				c = Xgetc();
			} else {
				if (c == EOF || strchr(" #$;\377[]=\t\n\r", c) != NULL) {
					(void)Xungetc(c);
					break;
				}
			}
			*(val++) = (char)c;
			c = Xgetc();
		}
		break;
	}
	*val = '\0';
	return;
}

/************************************************
 *	AddParamValue -- Add parameter and value to built line
 *
 *		If not adding first value, append a separating
 *  semicolon, then append the param and then, if there
 *  is a value, append an equal sign and the value.
 ************************************************/
static void
AddParamValue(param, value, str)
	char *param;	/* place to return parameter */
	char *value;	/* place to return value */
	char *str;		/* place to pack the parameter */
{
	if (param == NULL || strlen(param) == 0) {
		return;	/* just in case nothing to add */
	}
	if (strlen(str) != 0) {
		(void)strcat(str, ";");
	}
	(void)strcat(str,param);
	if (value != NULL && strlen(value) != 0) {
		(void)strcat(str, "=");
		(void)strcat(str, value);
	}
}

/************************************************
 *  GetValue -- Get value for symbol in boot param string
 *
 * Given a boot parameter string and a symbol, return
 * a point to a static buffer that contains the value.
 *
 ************************************************/
static char GVBuffer[MAXVALUELEN];

char *
GetValue(bps, symb)
	char *bps;
	char *symb;
{
	static void ResolveDollars();
	char tparam[MAXVALUELEN];
	char tvalue[MAXVALUELEN];
	int tterm;
	char *cp, *cpt;

	DDEBUG(DVALUE,("GetValue: bps='%s', symb='%s'\n",bps,symb));
	XPushGetc();
	XInitString(bps);
	GVBuffer[0] = '\0';
	/* CONSTANTCONDITION */
	while (1) {
		XGetParamValue(&tparam[0], &tvalue[0], &tterm);
		DDEBUG(DVALUE,("GetValue: fetch: par='%s', val='%s', term='%c'\n",
				tparam, tvalue, tterm));
		if (strlen(symb) == strlen(tparam)) {
			DDEBUG(DVALUE,("GetValue: same length: sym='%s', param='%s'\n",
				symb, tparam));
			for (cp = symb,cpt = &tparam[0]; *cp; cp++, cpt++) {
				if (tolower(*cp) != tolower(*cpt)) {
					break;
				}
			}
			if (*cp == '\0') {
				DDEBUG(DVALUE,("GetValue: match\n"));
				if (bps != GlobClient) {
					ResolveDollars(&tvalue[0]);
				}
				XPopGetc();
				(void)strcpy(GVBuffer, tvalue);
				return(&GVBuffer[0]);
			}
		}
		if (tterm == EOF) {
			break;
		}
	}
	DDEBUG(DVALUE,("GetValue: failure\n"));
	XPopGetc();
	return(NULL);
}

/************************************************
 *
 ************************************************/
static void
ResolveDollars(val)
	char *val;	/* the parameter value to do dollar fixups */
{
	char RDBuffer[MAXVALUELEN];

	RDBuffer[0] = '\0';
	XPushGetc();
	XInitString(val);
	DDEBUG(DDOLR,("ResolveDollar: before val='%s'\n",val));
	XScanValue(&RDBuffer[0], RESOLVE);
	(void)strcpy(val, RDBuffer);
	DDEBUG(DDOLR,("ResolveDollar: after val='%s'\n",val));
	XPopGetc();
	return;
}

/************************************************
 *
 ************************************************/
static void
FlushKnownConfig()
{
	static void FlushKnownHostConfig();

	if (GlobServer != NULL) {
		free(GlobServer);
		GlobServer = NULL;
	}
	if (GlobClient != NULL) {
		free(GlobClient);
		GlobClient = NULL;
	}
	FlushKnownHostConfig();
}

/************************************************
 *
 ************************************************/
static void
FlushKnownHostConfig()
{
	if (HostServer != NULL) {
		free(HostServer);
		HostServer = NULL;
	}
	if (HostClient != NULL) {
		free(HostClient);
		HostClient = NULL;
	}
	HostHost = -1;
}

/************************************************
 *
 ************************************************/
static void
XInitString(strng)
	char * strng;
{
	getflag = STRNG;
	getpntr = strng;
}

/************************************************
 *
 ************************************************/
static void
XInitConfig()
{
	getflag = CNFG;
}

/************************************************
 *
 ************************************************/
static void
XPushGetc()
{
	savegetflag[getsave] = getflag;
	savegetpntr[getsave++] = getpntr;
}

/************************************************
 *
 ************************************************/
static void
XPopGetc()
{
	getflag = savegetflag[--getsave];
	getpntr = savegetpntr[getsave];
}

/************************************************
 *
 ************************************************/
static int
Xgetc()
{
	if (getflag == STRNG) {
		if (getpntr == (char *)NULL || *getpntr == '\0') {
			return(EOF);
		} else {
			return(*(getpntr++));
		}
	} else {
		if (cfile == (FILE *)NULL)
			return(EOF);
		else
			return(getc(cfile));
	}
}

/************************************************
 *
 ************************************************/
static int
Xungetc(c)
	int c;
{
	if (getflag == STRNG) {
		if (c == EOF) {
			return(EOF);
		} else {
			return(*(--getpntr));
		}
	} else {
		return(ungetc(c, cfile));
	}
}

/************************************************
 *
 ************************************************/
static char *Digits = "0123456789ABCDEF";

static int
BPI_numeric(valstr)
	register char *valstr;
{
	char *cp;
	int base = 10;
	int packed = 0;

	switch (toupper(valstr[strlen(valstr)-1])) {
		case 'Y':
			base = 2; break;
		case 'Q':
			base = 8; break;
		case 'T':
			base = 10; break;
		case 'H':
			base = 16; break;
	}
	/* CONSTANTCONDITION */
	while (1) {
		cp = strchr(Digits, toupper(*(valstr++)));
		if (cp == NULL || *cp == 0) {
			return(packed);
		}
		packed = packed*base + (cp - &Digits[0]);
	}
}
