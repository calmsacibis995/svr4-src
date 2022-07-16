/	Copyright (c) 1990 UNIX System Laboratories, Inc.
/	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T
/	  All Rights Reserved

/	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
/	UNIX System Laboratories, Inc.
/	The copyright notice above does not evidence any
/	actual or intended publication of such source code.

/ 			INTEL CORPORATION PROPRIETARY INFORMATION
/
/	This software is supplied under the terms of a license  agreement or 
/	nondisclosure agreement with Intel Corporation and may not be copied 
/	nor disclosed except in accordance with the terms of that agreement.
/
/	Copyright 1988 Intel Corporation
/
	.file   "msg.s"

	.ident	"@(#)mbus:uts/i386/boot/msa/msg.s	1.3.1.1"

/ ********************************************************************
/
/	These are the constants used in the Unix V/386 R3.1 second stage 
/	bootstrap loader for Intel's MBII system 
/
/	The simple reason why the messages are declared in .text is that
/	the first stage bootstrap loader loads only the .text portion of
/	the second stage. 
/
/ ********************************************************************
	.text

	.globl def_mode_value 
def_mode_value:
	.string "p"

	.globl default_value 
default_value:
	.string "ON"

	.globl bl_config_file_key 
bl_config_file_key:
	.string	 "BL_Config_file" 

	.globl bl_debug_boot_key 
bl_debug_boot_key:
	.string "BL_Debug_on_boot"

	.globl bl_boot_name_key 
bl_boot_name_key:
	.string "BL_Boot_Device"

	.globl bl_startup_mode_key 
bl_startup_mode_key:
	.string "BL_Mode"

	.globl bl_ramdisk_key 
bl_ramdisk_key:
	.string	 "BL_Ram_disk" 

	.globl	bl_partnum_key
bl_partnum_key:
	.string	"BL_Partition_num"

	.globl bl_target_file_key 
bl_target_file_key:
	.string "BL_Target_file"

	.globl	bl_host_id_key
bl_host_id_key:
	.string	 "BL_Host_ID" 

	.globl	bl_clear_parity_key
bl_clear_parity_key:
	.string	 "BL_Clear_Parity"

	.globl	global_id_tag
global_id_tag:
	.string "GLOBAL"

	.globl	rootdev_key
rootdev_key:
	.string	"rootdev"
	
	.globl	pipedev_key
pipedev_key:
	.string "pipedev"

	.globl	swapdev_key
swapdev_key:
	.string	"swapdev"

	.globl	dumpdev_key
dumpdev_key:
	.string	"dumpdev"

	.globl target_file 
target_file:
	.string	 "/unix" 

	.globl	config_file
config_file:
	.string "/etc/default/bootserver/config"

	.globl config_open_msg
config_open_msg:
	.string	"Cannot open configuration file, using default values" 

	.globl	param_msg
param_msg:
	.string	"Server configuration parameters not found"

	.globl	noclient_msg
noclient_msg:
	.string	"No client parameters found"

	.globl	bpserr_msg
bpserr_msg:
	.string	"Error while updating BPS from configuration file"

	.globl	loading_msg
loading_msg:
	.string  "Loading target file ..  "

	.globl	target_open_msg
target_open_msg:
	.string	 "Cannot open target file " 

	.globl	bad_magic_msg
bad_magic_msg:
	.string	"Bad magic number in target file"

	.globl	no_tss_msg
no_tss_msg:
	.string	"Expected TSS not found in target file" 

	.globl	file_read_msg
file_read_msg:
	.string	"File read error" 

	.globl	mem_overlap_msg
mem_overlap_msg:
	.string	"Memory overlap error" 

	.globl	scan_past_msg
scan_past_msg:
	.string	"Internal error - scan past file"

	.globl	alt_len_msg
alt_len_msg:
	.string	"altlen greater than alt. buffer size"

	.globl	bad_alt_msg
bad_alt_msg:
	.string	"Bad Alternate Table"

	.globl	bad_vtoc_msg
bad_vtoc_msg:
	.string	"Bad Volume Table of Contents"

	.globl	fnexist_msg
fnexist_msg:
	.string	"File does not exist"

