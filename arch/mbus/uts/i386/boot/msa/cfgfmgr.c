/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1988  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:uts/i386/boot/msa/cfgfmgr.c	1.3"

/*
 * This is the configuration file manager for Multibus II 386 protected mode
 * Second stage bootstrap loader.	It reads 
 * the configuration file from Unix file system, provides the functions 
 * to extract server parameters, client parameters, and, a lookup function.
 *
 * It is assumed that the configuration file is already open and the 
 * caller will close the file after the parameters are extracted. 
 *
 * The configuration file manager has no capability to seek while scanning 
 * within the file, and reads the file sequentially only. In normal use, 
 * it scans in a single pass manner, examining parameter strings once and 
 * only once.  However, this module does indirectly support the ability 
 * to rescan all or portions of the configuration file.
 *
 * To do this, the caller must seek to the portion of the file to be 
 * rescanned prior to scanning.  To rescan the complete file, the caller 
 * would seek to offset 0 in the file.  To rescan a particular host entry, 
 * the caller would seek to the offset of that entry first.
 *
 * It is important the the complete file be scanned at least once
 * following the init operation, either when the file is first
 * being opened, or after the file has been modified while
 * scanning.  This allows global parameters at the start of the
 * file to be read and saved internally for later use.
 */
#include "../sys/boot.h"
#include "../sys/s2main.h"
#include "../sys/error.h"

extern	char	bl_host_id_key[];
extern	char	global_id_tag[];
extern	ushort	ourDS; 

char	scan_buffer	 	[MAX_BPS_IMAGE];
char	global_save_area	[MAX_BPS_IMAGE];
char	tmp_save_area		[MAX_BPS_IMAGE];
char	host_id_str		[MAX_PARAM_VALUE];

ushort	temp_num [4];
ulong	c_actual;
ulong	fail;
ulong	file_offset;
ulong	tmp_file_offset;
ushort	entry_id;
ushort	eof;
int	(*CF_read)();	/* pointer to file read procedure */

struct	scan_info	{
	ushort	index;
	ushort	limit;
 	char	*buffer_ptr;
	ushort	buffer_sel;
} scan_info;

/******************************************************************************
*
* TITLE: CF_initcfm
*
* CALLING SEQUENCE:
* 	CF_initcfm(read_proc);
*
* INTERFACE VARIABLES:
* read_proc 
*    	is a POINTER to procedure conforming to the XX_File_Read interface
*	defined in the Bootstrap Functional specification.  This procedure
*  	will be called to read data from the configuration file as
*  	required by the configuration file manager.
*
* CALLING PROCEDURES:
*	 pboot
*
* CALLS:
* 	None
*
* ABSTRACT:
*	This procedure takes as a parameter a pointer to a file I/O 
* 	routine provided by the caller. This I/O routine is used by the 
*	configuration file management routines for reading the configuration 
*	file, and must conform to the XX_File_Read interface defined in 
*	the Bootstrap functional specification.
*
*	This procedure must be called once and only once, before using the 
*	other services of the configuration file manager.
*
******************************************************************************/
void
CF_initcfm(read_proc)
register int	 (*read_proc)();
{
	/* save file read procedure parameter for later use */
	CF_read = read_proc ;

	scan_info.buffer_sel = ourDS;
	scan_info.buffer_ptr = scan_buffer ;

	eof = FALSE;
}

/******************************************************************************
*
* TITLE: CF_get_host
*
* CALLING SEQUENCE:
* 	found = CF_get_host(flags, host_id, server_buf, server_off, 
*							client_off, status);
*
* INTERFACE VARIABLES:
*  found
*	is of type SHORT and indicates whether an entry for the requested 
*	host was found.
*  flags
*	is a 32-bit quantity containing bit flags controlling the 
*    	action of the configuration file manager.
*		Bit 0 - Seeked in file.
*		Bit 1 - New file.
*		Bit 2 - Next entry.
*  host_id
*  	is of type SHORT containing the host id of the board for which 
*  	configuration parameters are desired. If the 'next entry' flag 
*	is set, this parameter is ignored.
*  server_buf
*	is a POINTER to a MAX_BPS_IMAGE byte buffer in which the 
*	server parameters for the host will be stored.
*  server_off
*	is a POINTER to a 32-bit quantity where the byte offset in the 
*	file of the start of the server parameter string for this host 
*	will be returned.
*  client_off
*	is a POINTER to a 32-bit quantity where the byte offset in the 
*	file of the start of the client parameter string for this host 
*	will be returned.
*  status
*	is a POINTER to a 32-bit quantity where a status code is to 
*	be returned.
*
* CALLING PROCEDURES:
*	pboot
*
* CALLS:
*	None
*
* ABSTRACT:
*	This procedure searches the configuration file for a host entry.
*
*	Configuration file entries are distinguished by a keyword value 
*	in the server parameters of that entry. To determine whether a 
*	specific entry is the one desired, the server parameters of that 
*	entry are read, the value of the host id keyword is looked up, and 
*	that value is matched to the host id desired. If the host id does 
*	not match, the following client parameters are skipped, and
*	the next server parameters are checked.
*
******************************************************************************/
ushort
CF_get_host(flags, host_id, server_buf, server_off, client_off, status)
register ulong	flags;
register ushort	host_id;
register char	*server_buf;
register ulong	*server_off;
register ulong	*client_off;
register ulong	*status;
{	register ushort	host_found;
	register short	entry_found;
	register ushort	done;
	
	host_found = FALSE;
	done = FALSE;
	if (((flags & CF_SEEKED) == CF_SEEKED) ||
		((flags & CF_NEW_FILE) == CF_NEW_FILE)) {

		/* Initialize global scanning state variables */

		scan_info.limit = sizeof(scan_buffer);
		scan_info.index = scan_info.limit;
	}
	
	/* if new file, initialize global parameter save area */

	if ((flags & CF_NEW_FILE) == CF_NEW_FILE) 
		*status = BPX_init((char *)global_save_area, 
				sizeof(global_save_area));
	
	file_offset = 0;	/* Initialize file offset to zero */
	
	/* Search for start of first set of server parameters */
	
	entry_found = find_server(&file_offset, status);
	if (*status != E_OK)
		done = TRUE;
	if ((!done) && (!entry_found)) {
		/* Empty configuration file.  Initialize server and client 
		 * parameters to null.
		 */
		*status = BPX_copy((char *)global_save_area, 
				(char *)server_buf, sizeof(global_save_area));
	
		*server_off = file_offset;
		*client_off = file_offset;	
		done = TRUE;
	}
	if (!done) {
		/* Save file offset of the start of server parameters.  
		 * Include the '[' character that marks the start of parameters.
		 */
		*server_off = file_offset - 1;
			
		/* Load first server parameter string */
	
		load_server_params(server_buf, (ulong *)&file_offset, 
			status);
		if (*status != E_OK)
			done = TRUE;
	}	
	if (!done) {
		/* Get host ID value from server parameters	 */
	
		*status = BPX_get((char *)server_buf, 
				(char *)bl_host_id_key, 
				(char *)host_id_str, sizeof(host_id_str));
		if (*status == E_PARAMETER_NOT_FOUND) 
			*status = E_CONFIG_NO_HOST_ID;
		if (*status != E_OK)
			done = TRUE;
	}
	
	/* Check for Global parameters */
	
	if ((!done) && (strcmp_cl((char *)host_id_str, global_id_tag)))	{

		/* Read Global Parameters */
		
		entry_found = load_params('[', global_save_area, 
				&file_offset, status);
		if (*status != E_OK)
			done = TRUE;
	
		if ((!done) && (entry_found)) {
			/* Save file offset of the start of server parameters.
			 * Include the '[' character that marks the start 
			 * of parameters.
			 */
			*server_off = file_offset - 1;
			
			/* Load next set of server parameters */
			
			load_server_params(server_buf, 
				(ulong *)&file_offset, status);
			if (*status != E_OK)
				done = TRUE;
			else {
				/* Get Host ID of these server parameters */

				*status = BPX_get((char *)server_buf, 
						bl_host_id_key, host_id_str,
						  sizeof(host_id_str));
				if (*status == E_PARAMETER_NOT_FOUND) 
					*status = E_CONFIG_NO_HOST_ID;
				if (*status != E_OK)
					done = TRUE;
			}
		}
	}  /* Global parameters */
	
	/* Search for server parameters with matching Host ID */

	if (!done) {
		*status = BP_numeric((char *)host_id_str, temp_num);
		entry_id = temp_num[0];
		if (*status != E_OK)
			done = TRUE;
	}
	
	while (	(!done) && 
		(entry_found) &&
		(entry_id != host_id) &&
		((flags & CF_GET_NEXT_HOST) == CF_GET_SELECTED_HOST)) {
	
		/* Skip client parameters */
		
		entry_found = find_server(&file_offset, status);
		if (*status != E_OK)
			done = TRUE;
		if ((!done) && (entry_found)) {
			/* Save file offset of the start of server parameters.
			 * Include the '[' character that marks the 
			 * start of parameters.
			 */
			*server_off = file_offset - 1;
			
			/* Load next server parameters */
			
			load_server_params(server_buf, 
				(ulong *)&file_offset, status);
			if (*status != E_OK)
				done = TRUE;
	
			/* Get Host ID of these server parameters */

			if (!done) {
				*status = BPX_get((char *)server_buf, 
					bl_host_id_key, host_id_str, 
					sizeof(host_id_str));
				if (*status == E_PARAMETER_NOT_FOUND) 
					*status = E_CONFIG_NO_HOST_ID;
				if (*status != E_OK)
					done = TRUE;
				else {
					*status = BP_numeric(
						(char *)host_id_str, temp_num);
					entry_id = temp_num[0];
					if (*status != E_OK)
						done = TRUE;
				}
			}
	
		}	 /* Found server parmeters */
	
	}		/* While host ID does not match. */
	
	/* Get location of client parameters if host was found */	
	
	if ((!done) && (entry_found))  {
		*client_off = file_offset;
		host_found = TRUE;
	}
	
	return (host_found);
	
}

/******************************************************************************
*
* TITLE: CF_get_client
*
* CALLING SEQUENCE:
* 	CF_get_client(flags, client_buf, status);
*
* INTERFACE VARIABLES:
* flags
*  	is a 32-bit quantity containing bit flags controlling the action of 
*	the configuration file manager.
*		Bit 0 - Seeked in file.
* client_buf
*	is a POINTER to a MAX_BPS_STRING_SIZE byte buffer where a 
*  	client parameter string for a host will be returned.  This string 
*	includes all global parameters from the configuration file.
* status
*	is a POINTER to a 32-bit quantity where a status code is to 
*  	be returned.
*
* CALLING PROCEDURES:
* 	CF_get_host
*
* CALLS:
*  	None
*
* ABSTRACT:
*	This procedure reads the client parameters for a particular
*	host.  It must be called immediately after a CF_get_host
*	call, or after seeking to a client_offset returned by
*	CF_get_host.
*
*	This procedure copies the global parameters into the a temporary
*	save area then reads client parameters out of the configuration
* 	file into the same save area.
*
* 	After all parameters are loaded, the contents of the temporary
* 	save area is converted to a null terminated, BPS syntax string
* 	and is returned to the caller.
*
******************************************************************************/
CF_get_client (flags, client_buf, status)
register ulong	flags;
register char	*client_buf;
register ulong	*status;
{	register ushort	found;
	
	if ((flags & CF_SEEKED) == CF_SEEKED) {
	   
		/* Invalidate internal scanning buffer */
		
		scan_info.limit = sizeof(scan_buffer);
		scan_info.index = scan_info.limit;
	}
	
	/* copy global parameters to temporary save area */
	
	*status = BPX_copy((char *)global_save_area, 
			(char *)tmp_save_area, sizeof(tmp_save_area));
	
	/* Read Client Parameters */
	
	found = load_params('[', tmp_save_area, &tmp_file_offset, 
			status);
	if (*status == E_OK) 
		/* Convert client parameters to string format */
		*status = BPX_dump((char *)tmp_save_area, client_buf,
			 	MAX_BPS_IMAGE);

}


/******************************************************************************
*
* TITLE:	CF_get_server
*
* CALLING SEQUENCE:
*	 status = CF_get_server(server_buf, name, value, value_size);
*
* INTERFACE VARIABLES:
*	 status
*	 	is a 32-bit quantity where a status code is to be returned.
* 	server_buf
*	 	is a POINTER to a MAX_BPS_IMAGE byte buffer in which 
*		the server parameters for a host have been stored.
* 	name
* 		is a POINTER to a null terminated string containing a 
* 		keyword for the parameter to be looked up.
* 	value
*	 	is a POINTER to an area of memory where the value associated 
*	 	with the keyword (if any is found) will be returned.
* 		This area must be value_size bytes long.
* 	value_size
* 		a 32-bit quantity containing the size of the value buffer.
*
* CALLING PROCEDURES:
*		
* CALLS:
*	 BPX_get
*
* ABSTRACT:
* 	This procedure looks up a parameter value from a server
* 	parameter save area.  It provides functions analogous to the
* 	BP_Get function of the BPS Manager.
*
* 	This procedure serves as an external entry point to the
* 	lookup procedure used internally by this module.
*
******************************************************************************/
long
CF_get_server(server_buf, name, value, value_size)
register char	*server_buf;
register char	*name;
register char	*value;
register ulong	value_size;
{
	return BPX_get(server_buf, name, value, value_size);
}

/******************************************************************************
*
* TITLE:	load_server_params
*
* CALLING SEQUENCE:
*	 load_server_params(save_area, file_off, status);
*
* INTERFACE VARIABLES:
* 	save_area
* 		is a POINTER to a MAX_BPS_IMAGE byte buffer where the server 
* 		parameters will be stored.  Parameters for this host may be 
* 		looked up from this save area with the CF_get_server function.
* 		This save area will includes any global parameters that have 
* 		been found in the file.
* 	file_off
* 		is a POINTER to a 32-bit quantity containing the file offset 
*		of the next text to be processed by the configuration file 
*		manager.  This value is updated by this procedure to the 
*		offset just following these server parameters.  This 
*		offset is the start of the client parameters for this host, 
*		and is the location at 	which subsequent scanning will start.
* 	status
* 		is a POINTER to a 32-bit quantity where a status code is to 
* 		be returned.
*
* CALLING PROCEDURES:
* 	
* CALLS:
* 	BPX_get
*
* ABSTRACT:
* 	This procedure scans the configuration file and loads a set
* 	of server parameters.
*
* 	This procedure loads the next server parameter string in the
* 	file into a BPS save area.  It assumes that reading of the
* 	file will begin just after the opening delimiter of the
* 	server parameters ([), and reading will continue until
* 	reading the closing delimiter (]);
*
******************************************************************************/
load_server_params (save_area, file_off, status)
register char 	*save_area;
register ulong	*file_off;
register ulong	*status;
{	register ushort	end_found;

	/* Load global parameters into save area */
	
	*status = BPX_copy((char *)global_save_area, (char *)save_area, 
			MAX_BPS_IMAGE);
	
	/* Load server parameters from file into save area */
	
	end_found = load_params(']', save_area, file_off, status);
	if ((*status == E_OK) && (! end_found))
		*status = E_CONFIG_SYNTAX;

}

/******************************************************************************
*
* TITLE:	find_server
*
* CALLING SEQUENCE:
* 	found = find_server(file_off, status);
*
* INTERFACE VARIABLES:
* 	found
* 		is of type SHORT and indicates whether the start of server 
* 		parameter string was found.  If the server parameters were 
* 		found, TRUE (0xFF) is returned, else, FALSE (0) is returned.
* 	file_off
* 		is a POINTER to a 32-bit quantity containing the file offset 
*  		of the next text to be processed by the configuration file 
*  		manager.  This value is updated by this procedure to the 
* 		offset of the beginning of the server parameters.  
* 		This offset points past the character that marks the start 
*		of the server parameters.
* 	status
* 		is a POINTER to a 32-bit quantity where a status code is 
* 		to be returned.
*
* CALLING PROCEDURES:
*  	CF_get_host
* 	
* CALLS:
* 	BPX_init
* 	load_params
*
* ABSTRACT:
* 	This procedure scans through the file searching for the
* 	beginning of a set of server parameters.
*
******************************************************************************/
find_server(file_off, status)
register ulong	*file_off;
register ulong	*status;
{	register char	c;
	
	/* Initialize the scratch save area */
	
	*status = BPX_init((char *)tmp_save_area, sizeof(tmp_save_area));
	
	/* Read anything up to the first opening square bracket ([) into 
	 * the save area and throw it away.
	 */
	c = '[';
	return (load_params(c, tmp_save_area, file_off, status));

}

/******************************************************************************
*
* TITLE:	load_params
*
* CALLING SEQUENCE:
* 	found = load_params(terminator, save_area, file_off, status);
*
* INTERFACE VARIABLES:
* 	found
* 		is of type SHORT indicating whether the terminating 
*		character was found. A value of TRUE (0xFF) indicates 
* 		the terminator was found; a value of FALSE (0) indicates 
* 		it was not. If the terminator was not found, the entire 
*		configuration file has been read.
* 	terminator
* 		is of type CHAR containing a closing delimiter to search for.
* 		All parameters will be loaded until the end of file is reached, 
* 		the terminator is found, or an error has occurred.
* 	save_area
* 		is a POINTER to an initialized save area where the parameters 
* 		will be stored.
* 	file_off
* 		is a POINTER to a 32-bit quantity containing the file offset 
* 		of the next text to be processed by the configuration file manager.
* 		This value is updated by this procedure to the offset just past 
* 		the terminator character.
* 	status
* 		is a POINTER to a 32-bit quantity where a status code is to 
* 		be returned.
*
* CALLING PROCEDURES:
* 	CF_get_host
* 	CF_get_client
* 	find_server
* 	load_server_params
* 	
* CALLS:
* 	BPX_add
* 	CF_read
*
* ABSTRACT:
* 	This procedure loads the next parameter string delimited by a
* 	terminating character from the configuration file into the save
* 	area.  This function returns a boolean value if the terminating
* 	delimiter is found.
*
******************************************************************************/
load_params(terminator, save_area, file_off, status)
register char 	terminator;
register char	*save_area;
register ulong	*file_off;
register ulong	*status;
{	register ulong	scanned;
	register ulong	old_loc;
	register ushort	found;
	register ushort	done;

	found = FALSE;
	done = FALSE;
	scanned = 0;
	
	while ((! found) && (!done)) {
	
		/* Ensure that the global scanning buffer is not empty */

		if ((scan_info.index >= scan_info.limit) && (!eof)) {
			
			/* Fill the scanning buffer */
			
			(*CF_read)((char *)scan_buffer, ourDS,
				(long )sizeof(scan_buffer), 
					&c_actual, status);
			if ((*status == E_FILE_MARK) || (*status = E_EOF)) {
				eof = TRUE;
				*status = E_OK;
			}

			scan_info.limit = c_actual;
	
			/* Point to the start of the buffer */
			
			scan_info.index = 0;
			
			/* Check for error or end of file */
			
			if ((*status != E_OK) || (scan_info.limit == 0))
				done = TRUE;
		}
		if (!done) {

			/* Search for the terminator character in the 
			 * scanning buffer 
			 */
			
			old_loc = scan_info.index;
			*status = BPX_add(save_area, &scan_info, BP_CONFIG,
					&fail);
			scanned += (scan_info.index - old_loc);
			if (*status == E_OK) {
				if ((scan_buffer[scan_info.index - 1] == 
					terminator))
					found = TRUE;
				else
					done = TRUE;
			}
			else {
				if ((*status == E_CONTINUE) && eof) {
					*status = E_OK;
					found = TRUE;
				}
				if (*status != E_CONTINUE)
					done = TRUE;
			}
		}
	}	  /* While not found */
	
	/* add number scanned to current file_off */
	
	*file_off = *file_off + scanned;
	
	return (found);

}

/******************************************************************************
*
* TITLE:	strcmp_cl
*
* CALLING SEQUENCE:
* 	flag = strcmp_cl(*p1, *p2);
*
* INTERFACE VARIABLES:
* 	flag 
* 		is of type SHORT and is TRUE(1) if the strings compare,
*		FALSE (0) otherwise.
* 	p1 
*		is a POINTER to a null-terminated string to be compared 
*		against p2
* 	p2 	
*		is a POINTER to a null-terminated string
*
* CALLING PROCEDURES:
* 	CF_get_host
*
* CALLS:
* 	None
*
* ABSTRACT:
* 	This procedure compares two input strings and returns true (1) if all
* 	characters (upper or lower) compare, else, it returns a false (0).
*
******************************************************************************/
strcmp_cl(p1, p2)
register char 	*p1;
register char	*p2;
{	register ushort	match;
	register ushort	done;
	register char	c1, c2;

	done = FALSE;
	match = FALSE;
	while (!done) {
	 	if (*p1 =='\0') {
			done = TRUE;
	 		if (*p2 == '\0') 
				match = TRUE;
		}
		else {
			/* first coerce to lower case */
			
			c1 = ((*p1 >= 'a') && (*p1 <= 'z')) 
				? *p1 - ('a' - 'A') : *p1;
			c2 = ((*p2 >= 'a') && (*p2 <= 'z'))
				? *p2 - ('a' - 'A') : *p2;

			/* check for equality */
			
			if (c1 == c2) {
				p1++;
	 			p2++;
	 		}
			else
				done = TRUE;
		}
	}
	return (match);
}
