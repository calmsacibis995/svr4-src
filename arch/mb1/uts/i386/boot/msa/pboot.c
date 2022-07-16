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

#ident	"@(#)mb1:uts/i386/boot/msa/pboot.c	1.3"

/*
 *  MSA Second stage bootstrap loader
 */
#include "../sys/boot.h"
#include "../sys/dib.h"
#include "../sys/s2main.h"
#include "../sys/error.h"

#define	PARITY_CONT_REG 0x43		/* applicable to 386/1xx boards only */

extern		ushort	get_host_id();
extern		void	ic_write();
extern		void	BL_file_read();
/*
extern		void	CF_initcfm();
*/

extern		struct 	dib 	dib_data;
#ifdef MB1
extern caddr_t path_off;
extern ushort path_sel;
extern ulong bootparam;
extern ushort ourDS;
#endif

/* static  char	default_value [] 	= { "OFF" };
 * static  char	bl_config_file_key [] 	= { "BL_Config_file" };
 * static  char	bl_debug_boot_key [] 	= { "BL_Debug_on_boot" };
 * static  char bl_ramdisk_key [] 	= { "BL_Ram_disk" };
 * static  char bl_partnum_key [] 	= { "BL_Partition_num" };
 * static  char	bl_target_file_key [] 	= { "BL_Target_file" };
 * static  char	bl_host_id_key [] 	= { "BL_Host_id" };
 * static  char	bl_clear_parity_key [] 	= { "BL_Parity" };
 * static  char	target_file [] 		= { "/unix" };

 * static char config_open_msg [] 	= {"Cannot open configuration file" };
 * static char param_msg [] = {"Server configuration parameters not found"};
 * static char noclient_msg [] = {"No client parameters found"};
 * static char bpserr_msg [] = {"Error while updating BPS from configuration file"};
 */

extern		char	default_value [];
extern		char	def_mode_value [];
extern		char	bl_config_file_key [];
extern		char	bl_boot_name_key [];
extern		char	bl_startup_mode_key [];
extern		char	bl_debug_boot_key [];
extern		char	bl_ramdisk_key [];
extern		char	bl_partnum_key [];
extern		char	bl_target_file_key [];
extern		char	bl_host_id_key [];
extern		char	bl_clear_parity_key [];
extern		char	target_file [];
extern		char	config_file [];
extern		char	config_open_msg [];
extern		char	param_msg [];
extern		char	noclient_msg [];
extern		char	bpserr_msg [];
extern		ulong	status;

ushort	numeric_key [4];
ushort	prot_mode_boot;
ushort	debug_on_boot;
ushort	ramdisk_num;
ushort	part_num;
ushort	hostid;
ushort	found;
ulong	server_offset;
ulong	client_offset;
char	*config_path_p;

ulong	error_loc;
char	*path_p;

char 	host_BPS_image	[MAX_BPS_IMAGE];
char	server_params 	[MAX_BPS_IMAGE];

char	temp			[MAX_PARAM_VALUE];
char	config_path 		[MAX_PARAM_VALUE];
char	target_path 		[MAX_PARAM_VALUE];
char	debug_value 		[MAX_BDEVICE_VALUE];
char	mode_value 		[MAX_BDEVICE_VALUE];
char	bdevice_name	 	[MAX_BDEVICE_VALUE];

/******************************************************************************
*
* TITLE: pboot
*
* CALLING SEQUENCE:
* 	pboot
*
* INTERFACE VARIABLES:
* 	None.
*
* CALLING PROCEDURES:
* 	s2main
*
* CALLS:
* 	get_host_bps
* 	get_host_id
* 	BL_init, BL_file_open, BL_file_close
* 	error
* 	CF_initcfm, CF_get_host, CF_get_client
* 	BP_add, BP_get, BP_numeric
* 	compare_strings
* 	pload
*
* ABSTRACT:
* 	This procedure is responsible to get the BPS image from the 
* 	configuration file (either from the bootserver or from disk/tape), 
* 	update the memory resident BPS, get the values for the parameters 
*	BL_Host_ID, BL_Config_file, BL_Partition_num, BL_Target_file, 
*	BL_Ram_disk, BL_Debug_on_Boot, and pass control to the 
*	pre-load (pload) routine.  
*
*	This procedure handles initialization of the configuration 
*	file manager if required.  For disk devices, it calls the 
*	BL_init routine which handles alternate tracking.
*
******************************************************************************/
pboot()
{
	status = E_OK;
 
	/* Get BPS image from Configuration File */
	if (dib_data.hdr.device_type == BS_DIB)
		get_host_bps((char *)host_BPS_image, &status);
	else {
#ifdef MB1
		part_num = 1;
#else
		/* Get host ID and configuration file pathname.  
		 * To avoid dependency on i/c operation, we search
		 * for the parameter BL_Host_ID first.  If the 
		 * parameter is not found (earlier versions of bootstrap
		 * first stage), we access i/c and get the host id.
		 */
		status = BP_get((char *)bl_host_id_key, (char *)temp, 
							sizeof(temp));
		if (status == E_OK) {
			status = BP_numeric((char *)temp, numeric_key);
			hostid = numeric_key [0];
		}
		else
			hostid = get_host_id();		/* compatability !! */

		status = BP_get((char *)bl_config_file_key, 
				(char *)config_path, sizeof(config_path));
	
		config_path_p = (status != E_OK) ? (char *)config_file
							: (char *)config_path;

		/* 
		 * We need to examine the parameter BL_Partition_Num twice.
		 * Now and then after the configuration file is read.
		 * This parameter is applicable only for disk booting.
		 */
		status = BP_get((char *)bl_partnum_key, (char *)temp, 
							sizeof(temp));
		if (status != E_OK)
			part_num = 1;
		else	{
			status = BP_numeric((char *)temp, numeric_key);
			part_num = numeric_key [0];
		}
#endif
		if ((dib_data.hdr.device_type == DISK_DIB) ||
		    (dib_data.hdr.device_type == PCI_DISK_DIB))    
			/* special for disk device */
			BL_init();
		
		/* Open configuration file */
		BL_file_open((char *)config_path_p, &dib_data, &status);
		if (status != E_OK) {
			/* 
			 * no configuration file is not a fatal error, 
			 * so just set status to E_OK and continue
			 */
			status = E_OK;
			error(STAGE2, status, (char *)config_open_msg);
		}

#ifndef MB1
		CF_initcfm(BL_file_read);

		/* Find the server parameters for this host from the 
		 * configuration file
		 */
		found = CF_get_host(CF_NEW_FILE | 
					CF_GET_SELECTED_HOST, 
					hostid, server_params, 
					&server_offset, 
					&client_offset, &status);

		if (status != E_OK)
			error(STAGE2, status, (char *)param_msg);

		/* Get BPS image from this host entry in 
		 * configuration file 
		 */
		
		CF_get_client(CF_SEQUENTIAL, 
			host_BPS_image, &status);
		if (status != E_OK)	{
			BL_file_close(&status);
			error(STAGE2, status, 
				(char *)noclient_msg);
		}
#endif /* !MB1 */
		/* Close configuration file */
		BL_file_close(&status);
	}
#ifdef MB1
	iomove(path_off, path_sel, target_path, ourDS, sizeof(target_path));
	path_p = target_path;
	if (*path_p == '\0')
		path_p = target_file;
	debug_on_boot = bootparam;
#else
	if (status != E_OK) 
		error(STAGE2, status, (char *)param_msg);
	/* Enter the bootstrap parameters in the memory resident BPS */
	
	status = BP_add((char *)host_BPS_image, BP_CONFIG, &error_loc);
	if (status != E_OK) 
		error(STAGE2, status, (char *)bpserr_msg);
	/* 
	 * get parameters BL_Target_file, BL_RAM_disk, 
	 * BL_Boot_Device
	 */
	
	status = BP_get((char *)bl_target_file_key, 
			(char *)target_path,
			sizeof(target_path));
	path_p = (status != E_OK) ? (char *)target_file : 
					(char *)target_path;
	status = BP_get((char *)bl_ramdisk_key, (char *)temp, 
					sizeof(temp));
	if (status != E_OK)
		ramdisk_num = 0;
	else {
		status = BP_numeric((char *)temp, numeric_key);
		ramdisk_num = numeric_key[0];
	}

	status = BP_get((char *)bl_partnum_key, (char *)temp, 
					sizeof(temp));
	if (status != E_OK)
		part_num = 1;
	else	{
		status = BP_numeric((char *)temp, numeric_key);
		part_num = numeric_key[0];
	}

	status = BP_get((char *)bl_boot_name_key,  
			(char *)bdevice_name,  
			sizeof(bdevice_name)); 

	status = BP_get((char *)bl_debug_boot_key, 
			(char *)debug_value, 
			sizeof(debug_value));
	debug_on_boot = compare_strings((char *)debug_value, 
				(char *)default_value);

	status = BP_get((char *)bl_startup_mode_key, 
			(char *)mode_value, 
			sizeof(mode_value));
	prot_mode_boot = compare_strings((char *)mode_value, 
			(char *)def_mode_value);

	status = BP_get((char *)bl_clear_parity_key, 
			(char *)temp, sizeof(temp));
	if (status == E_OK) {
		status = BP_numeric((char *)temp, numeric_key);
		if (numeric_key[0]) 
			ic_write(PARITY_CONT_REG, 0x3);
	}
#endif
	/* Pass the baton (relay race !!!) to the COFF Loader  
	 */
	pload(path_p);
}

/******************************************************************************
*
* TITLE: compare_strings
*
* CALLING SEQUENCE:
* 	flag = compare_strings(*p1, *p2);
*
* INTERFACE VARIABLES:
* 	flag 
*		is of type SHORT and is TRUE(1) if the strings compare, 
*		FALSE (0) otherwise.
* 	p1 
*		is a POINTER to a null-terminated string to be compared 
*		against p2
* 	p2 	is a POINTER to a null-terminated string
*
* CALLING PROCEDURES:
* 	pboot
*
* CALLS:
* 	lower
*
* ABSTRACT:
* 	This procedure compares two input strings and returns true (1) if all
* 	characters (upper or lower) compare, else, it returns a false (0).
*
******************************************************************************/
compare_strings(p1, p2)
register char	*p1;
register char	*p2;
{	register short	match;
	register short	done;

	done = FALSE;
	match = FALSE;
	while (!done) {
    	if (*p1 =='\0') {
			done = TRUE;
    		if (*p2 == '\0') 
				match = TRUE;
		}
		else {
			if (lower(*p1) == lower(*p2)) {
				p1++;
    				p2++;
   			}
			else
				done = TRUE;
		}
	}
	return (match);
}

/******************************************************************************
*
* TITLE: lower
*
* CALLING SEQUENCE:
* 	lower(c);
*
* INTERFACE VARIABLES:
* 	c	- CHAR to be converted to lower case ASCII
*
* CALLING PROCEDURES:
* 	compare_strings
*
* CALLS:
* 	None
*
* ABSTRACT:
* 	This procedure converts the given ASCII  character to lower case.
*
******************************************************************************/
lower(c)
register char	c;
{
	if ((c >= 'A') && (c <= 'Z'))
    	return (c + ('a'-'A')) ;
	else
    	return (c) ;
}
