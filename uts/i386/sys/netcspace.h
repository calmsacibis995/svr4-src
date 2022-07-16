/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head.sys:sys/netcspace.h	1.1.5.1"
/* TEMPORARY - added for kernel rpc */

struct nc_data {
	char         *string;
	unsigned long value;
};

static struct nc_data nc_semantics[] = {
	"tpi_clts",	NC_TPI_CLTS,
	"tpi_cots",	NC_TPI_COTS,
	"tpi_cots_ord",	NC_TPI_COTS_ORD,
	"npi_raw",	NC_NPI_RAW,
	NULL,		(unsigned)-1L
};

static struct nc_data nc_flag[] = {
	"-",		NC_NOFLAG,
	"v",		NC_VISIBLE,
	NULL,		(unsigned)-1L
};

static struct nc_data nc_protofmly[] = {
	"-",		NC_NOPROTOFMLY,
	"loopback",	NC_LOOPBACK,
	"inet",		NC_INET,
	"implink",	NC_IMPLINK,
	"pup",		NC_PUP,
	"chaos",	NC_CHAOS,
	"ns",		NC_NS,
	"nbs",		NC_NBS,
	"ecma",		NC_ECMA,
	"datakit",	NC_DATAKIT,
	"ccitt",	NC_CCITT,
	"sna",		NC_SNA,
	"decnet",	NC_DECNET,
	"dli",		NC_DLI,
	"lat",		NC_LAT,
	"hylink",	NC_HYLINK,
	"appletalk",	NC_APPLETALK,
	"nit",		NC_NIT,
	"ieee802",	NC_IEEE802,
	"osi",		NC_OSI,
	"x25",		NC_X25,
	"osinet",	NC_OSINET,
	"gosip",	NC_GOSIP,
	NULL,		(unsigned)-1L
};

static struct nc_data nc_proto[] = {
	"-",		NC_NOPROTO,
	"tcp",		NC_TCP,
	"udp",		NC_UDP,
	NULL,		(unsigned)-1L
};
