/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	Copyright (c) 1987, 1988  Intel Corporation	*/
/*	All Rights Reserved	*/

/*	INTEL CORPORATION PROPRIETARY INFORMATION	*/

/*	This software is supplied to AT & T under the terms of a license   */ 
/*	agreement with Intel Corporation and may not be copied nor         */
/*	disclosed except in accordance with the terms of that agreement.   */	

#ident	"@(#)mbus:cmd/mbusutils/mb2.d/cci.d/dl.d/down.c	1.3"

static char down_copyright[] = "Copyright 1987 Intel Corp. 461770";

struct interp_rec {
	unsigned char rec_type;
	char *rec_name;
	} interp_tab[] = {
		0x6e,"RHEADR",
		0x70,"REGINT",
		0x72,"IGNORE",
		0x74,"IGNORE",
		0x7a,"IGNORE",
		0x7c,"IGNORE",
		0x7e,"IGNORE",
		0x80,"THEADR",
		0x82,"LHEADR",
		0x84,"PEDATA",
		0x86,"PIDATA",
		0x88,"IGNORE",
		0x8a,"MODEND",
		0x8c,"IGNORE",
		0x8e,"IGNORE",
		0x90,"IGNORE",
		0x92,"IGNORE",
		0x94,"IGNORE",
		0x96,"IGNORE",
		0x98,"IGNORE",
		0x9a,"IGNORE",
		0x9c,"IGNORE",
		0xa4,"IGNORE",
		0xa6,"IGNORE",
		0xa8,"IGNORE",
		0xaa,"IGNORE"};

#include "trans.h"
#include <stdio.h>
#include "ext.h"
#include "xprt.h"
#include "coff.h"

struct omf_type_rec {
	int omf_type;
	char *omf_type_name;
	} omf_type_tab[] = {
		OMF86_TYPE,  "omf86",
		OMF86_TYPE,  "OMF86",
		OMF286_TYPE, "omf286",
		OMF286_TYPE, "OMF286",
		OMF386_TYPE, "omf386",
		OMF386_TYPE, "OMF386",
		COFF386_TYPE, "coff386",
		COFF386_TYPE, "COFF386"};

FILE *fopen(), *fp, *ofile;
unsigned short rec_len;
unsigned long start_addr;
int load_only_flag;
unsigned char slot_num;
extern int getopt();
extern char *optarg;
extern int optind;
unsigned char *progname;
unsigned int buf_size;
unsigned int delay;
int reset_flag;
short firmware_reg;
unsigned short portid;
int omf_type;
	
unsigned char *data_buf;
unsigned char *exp_buf;
	
/****************************************************************************/
/*                                                                          */
/*                            MAIN           		                        */
/*                            ----			                                */
/*                                                                          */
/****************************************************************************/

main(argc, argv)
int argc;
char *argv[];
{
	char *rec_t2n();
	void do_omf86();
	void do_omf286();
	void do_omf386();
	void do_coff386();
	int c;

	buf_size = BUF_SIZE;
	delay = set_delay();	
	ofile = open_bit_bucket();
	portid = DEFAULT_PORTID;
	progname = (unsigned char *) argv[0];
	omf_type = OMF86_TYPE;

	if (argc <= MAX_ARGS) {
		fprintf(stderr,"\n%s: Insufficient Arguments", progname);
		usage();
		exit(1);
	}
	
	catch_interrupt();

	while ((c = getopt(argc, argv, "b:lo:p:rt:v")) != EOF)
		switch(c) {
			case 'v':
				ofile = stdout;
				break;

			case 'l':
				load_only_flag = 1;
				break;


			case 'r':
				reset_flag = 1;
				break;

			case 'p':
				portid = atoi(optarg);
				if (portid <= MIN_PORT) {
					fprintf(stderr,"\n%s: Invalid Port ID\n",progname);
					exit(1);
				} 
				break;

			case 'b':
				buf_size = atoi(optarg);
				if (buf_size == 0) {
					fprintf(stderr,"\n%s: Buffer Size is too Small\n",progname);
					exit(1);
				}
				if (buf_size > BUF_SIZE) {
					fprintf(stderr,"\n%s: Buffer Size is too Large\n",progname);
					exit(1);
				}
				break;

			case 'o':
				omf_type = get_omf_type(optarg);
				if (omf_type == -1) {
					fprintf(stderr,"\n*** Error - Bad OMF Type\n");
					exit(1);
				}
				break;

			case 't':
				delay = atoi(optarg);
				break;

			case'?':
				usage();
				exit(1);
		}
				
	argc -= optind;
	argv += optind;

	if (argc != MAX_ARGS) {
		usage();
		exit(1);
	}

	if ((fp = fopen(argv[0], "rb")) ==NULL) {
		fprintf(stderr,"\n%s: Can't open %s\n",progname,argv[0]);
		exit(1);
	}

	slot_num = atoi(argv[1]);

	if (slot_num > MAX_SLOT) {
		fprintf(stderr,"\n%s: Invalid Slot Id\n",progname);
		exit(1);
	}
	
	data_buf = (unsigned char *) malloc(buf_size);
	if (data_buf == NULL) {
		fprintf(stderr,"\n%s: Memory Not Available\n",progname);
		exit (1);
	}	
	
	exp_buf = (unsigned char *) malloc(buf_size);
	if (exp_buf == NULL) {
		fprintf(stderr,"\n%s: Memory Not Available\n",progname);
		exit (1);
	}	

	fprintf(ofile,"Target Board is at slot %02d\n",slot_num);
	
	transport_init();
	
	firmware_reg = find_rec(slot_num, IC_FW_REC);
	if (firmware_reg  <= -1) {
		fprintf(stderr,"\n%s: Interconnect Record Not Found\n",progname);
		exit(1);
	}

	if (reset_flag)
		do_board_reset();

	if (omf_type == OMF86_TYPE)
		do_omf86();
	else if (omf_type == OMF286_TYPE)
		do_omf286();
	else if (omf_type == OMF386_TYPE)
		do_omf386();
	else if (omf_type == COFF386_TYPE)
		do_coff386();
}

/****************************************************************************/
/*                                                                          */
/*                                DO_OMF86   		                    */
/*                                --------		                    */
/*                                                                          */
/****************************************************************************/

void do_omf86()
{
	int j, count;
	unsigned char rec_type;
	unsigned char checksum;
	long offset;
	unsigned long exec_addr;
	void check_addr();

	j = 0;
	count = 0;
	offset = 0;
	start_addr = INVALID_ADDR;
	fprintf(ofile,"\nLoading:\n");
 
	for(;;) {

		unsigned int i;
		int status, x;
		unsigned short name_len;
		unsigned char name_buf[MAX_CHARS];
		unsigned char mod_addr_flag;
		
		status = my_read(data_buf,1,fp);
		if (status) {
			if (j == 0) {
				fprintf(stderr,"\n%s: File Is Empty\n",progname);
				exit(1);
			}	
			fprintf(stderr,"\n");
			fprintf(ofile,"\nEOF Encountered\n");
			check_addr(count, exec_addr);
			exit(0);
		}

		status = my_read(&data_buf[1],2,fp);
		if (status) {
			fprintf(stderr,"\n%s: Premature EOF Encountered\n",progname);
			exit(1);
		}

		rec_type = data_buf[0];
		if ((j == 0) && (rec_type != LHEAD) && (rec_type != LIBHEAD) && 
                   (rec_type != THEAD) && (rec_type != RHEAD)) {
			fprintf(stderr,"\n%s: Incorrect Header, Invalid OMF File\n",progname);
			exit(1);
		}
		rec_len = *((unsigned short *)&data_buf[1]);

		if (rec_len > (buf_size - 3)) {
			fprintf(stderr,"\n%s: Record Too Large\n",progname);
			exit(1);
		}	

		status = my_read(&data_buf[3],rec_len,fp);
		if (status) {
			fprintf(stderr,"\n%s: Premature EOF Encountered\n",progname);
			exit(1);
		}

		checksum = 0;

		for(i=0; i<rec_len+3; i++)
			checksum += data_buf[i];

		if (checksum != 0) {
			fprintf(stderr,"\n%s: Checksum Error\n",progname);
			fprintf(ofile,"Checksum = %u\n", checksum);
			for(i=0; i<rec_len+3; i++) {
				fprintf(ofile,"%02xH ",data_buf[i]);
				if (i % 16 == 0)
					fprintf(ofile,"\n");
			}
			exit(1);
		}

		if (j % 100 == 0)
		fprintf(stderr,".");
		fflush(stdout);
		j++;

		switch (rec_type) {

			case LHEAD: /* HEADER */ {

				name_len = rec_len - 1;
				i = 0;
				x = NAME_START;
				while (name_len > 0) {
					name_buf[i] = data_buf[x];
					i++;
					x++;
					name_len--;
				}
				break;
			}

			case PE_DATA: /* PEDATA */ {

				do_pedata(data_buf,rec_len);			
				break;
			}

			case PI_DATA: /* PIDATA */ {

				do_pidata(data_buf,exp_buf);			
				break;
			}
			
			case EXT_DEF: /* EXTDEF */ {

				fprintf(stderr,"\n%s: File Contains Unresolved Externals\n",progname);
				break;
			}

			case FIX_UP: /* FIXUP */ {

				fprintf(stderr,"\n%s: File Contains Fixup Records\n",progname);
				break;
			}

			case MOD_END: /* MODEND */ {

				do_modend(data_buf, &start_addr, &mod_addr_flag, name_buf);
				if (mod_addr_flag == MAIN_START) {
					count++;
					exec_addr = start_addr;
					mod_addr_flag = 0;
				}
				break;
			}

			default:
				break;
		}

		offset += rec_len + 3;
	}
}

/****************************************************************************/
/*                                                                          */
/*                                DO_OMF286                                 */
/*                                ---------	                            */
/*                                                                          */
/****************************************************************************/

void do_omf286()
{
	unsigned char file_type;
	struct toc {
		long abstxt_offset;
		long debtxt_offset;
		long last_offset;
	} toc;	
	long offset;
	long last_offset;
	
	/* Verify file type */

	if (my_read(&file_type, sizeof(file_type), fp)) {
		fprintf(stderr,"\n*** Error - Premature EOF Encountered\n");
		exit(1);
	}

	if (file_type != BOOT_LOADABLE_286) {
		fprintf(stderr,"\n*** Error - Not Boot Loadable File\n");
		exit(1);
	}

	/* Skip 75 byte header */
	
	if (fseek(fp, (long)BMOD_HDR_SIZE, 1)) {
		fprintf(stderr,"\n*** Error - Premature EOF Encountered\n");
		exit(1);
	}

	/* Read Table of Contents */

	if (my_read(&toc, sizeof(toc), fp)) {
		fprintf(stderr,"\n*** Error - Premature EOF Encountered\n");
		exit(1);
	}

	/* Seek to Start of ABSTXT */

	if (fseek(fp, toc.abstxt_offset, 0)) {
		fprintf(stderr,"\n*** Error - Premature EOF Encountered\n");
		exit(1);
	}

	last_offset = (toc.debtxt_offset == 0 ? toc.last_offset:toc.debtxt_offset);
	offset = toc.abstxt_offset;

	fprintf(ofile,"\nLoading:\n");
 
	while (offset < last_offset) {

#pragma pack(1)
		struct abstxt {
			char phys_addr_b[3];
			unsigned short len;
		} abstxt;
#pragma pack()
		unsigned long phys_addr_dw;
		unsigned long bytes_left;
		unsigned short frag_offset;
				
		/* Read 5 byte header */

		if (my_read(&abstxt, sizeof(abstxt), fp)) {
			fprintf(stderr,"\n*** Error - Premature EOF Encountered\n");
			exit(1);
		}

		fprintf(ofile, "ABSTXT -     %8lu bytes to %08lxH\n",
			abstxt.len, *(unsigned long *)abstxt.phys_addr_b);

		bytes_left = abstxt.len;
		frag_offset = 0;
		
		phys_addr_dw = *(unsigned long *)abstxt.phys_addr_b;
		phys_addr_dw = phys_addr_dw & 0x0ffffff;
		
		while (bytes_left > 0) {
			unsigned short bytes_to_read;

			bytes_to_read = (bytes_left > buf_size ? buf_size : bytes_left);

			/* Read in Data */
		
			if (my_read(data_buf, bytes_to_read, fp)) {
				fprintf(stderr,"\n*** Error - Premature EOF Encountered\n");
				exit(1);
			}

			phys_addr_dw += frag_offset;

			fprintf(ofile, "Downloading %5u bytes to %08lxH\n", 
				bytes_to_read, phys_addr_dw);

			do_download(&phys_addr_dw, data_buf, bytes_to_read);
	
			frag_offset += bytes_to_read;
			bytes_left -= bytes_to_read;
		
		}
		offset += abstxt.len + sizeof(abstxt);
	}
}

/****************************************************************************/
/*                                                                          */
/*                                DO_OMF386    	                            */
/*                                ---------		                    */
/*                                                                          */
/****************************************************************************/

void do_omf386()
{
	unsigned char file_type;
	struct toc {
		long abstxt_offset;
		long debtxt_offset;
		long last_offset;
	} toc;	
	long offset;
	long last_offset;
	
	/* Verify file type */

	if (my_read(&file_type, sizeof(file_type), fp)) {
		fprintf(stderr,"\n*** Error - Premature EOF Encountered\n");
		exit(1);
	}

	if (file_type != BOOT_LOADABLE_386) {
		fprintf(stderr,"\n*** Error - Not Boot Loadable File\n");
		exit(1);
	}

	/* Skip 75 byte header */
	
	if (fseek(fp, (long)BMOD_HDR_SIZE, 1)) {
		fprintf(stderr,"\n*** Error - Premature EOF Encountered\n");
		exit(1);
	}

	/* Read Table of Contents */

	if (my_read(&toc, sizeof(toc), fp)) {
		fprintf(stderr,"\n*** Error - Premature EOF Encountered\n");
		exit(1);
	}

	/* Seek to Start of ABSTXT */

	if (fseek(fp, toc.abstxt_offset, 0)) {
		fprintf(stderr,"\n*** Error - Premature EOF Encountered\n");
		exit(1);
	}

	last_offset = (toc.debtxt_offset == 0 ? toc.last_offset:toc.debtxt_offset);
	offset = toc.abstxt_offset;

	fprintf(ofile,"\nLoading:\n");
 
	while (offset < last_offset) {
		struct abstxt {
			char phys_addr_b[4];
			unsigned long len;
		} abstxt;
		unsigned long phys_addr_dw;
		unsigned long bytes_left;
		unsigned short frag_offset;
				
		/* Read 8 byte header */

		if (my_read(&abstxt, sizeof(abstxt), fp)) {
			fprintf(stderr,"\n*** Error - Premature EOF Encountered\n");
			exit(1);
		}

		fprintf(ofile, "ABSTXT -     %8lu bytes to %08lxH\n",
							abstxt.len, *(unsigned long *)abstxt.phys_addr_b);

		bytes_left = abstxt.len;
		frag_offset = 0;
		
		phys_addr_dw = *(unsigned long *)abstxt.phys_addr_b;
		
		while (bytes_left > 0) {
			unsigned short bytes_to_read;

			bytes_to_read = (bytes_left > buf_size ? buf_size : bytes_left);

			/* Read in Data */
		
			if (my_read(data_buf, bytes_to_read, fp)) {
				fprintf(stderr,"\n*** Error - Premature EOF Encountered\n");
				exit(1);
			}

			phys_addr_dw += frag_offset;
			
			fprintf(ofile, "    Downloading %5u bytes to %08lxH\n", 
												bytes_to_read, phys_addr_dw);

			do_download(&phys_addr_dw, data_buf, bytes_to_read);
	
			frag_offset += bytes_to_read;
			bytes_left -= bytes_to_read;
		}

		offset += abstxt.len + sizeof(abstxt);
	}
}

/****************************************************************************/
/*                                                                          */
/*                                DO_COFF386   	                            */
/*                                ----------		                    */
/*                                                                          */
/****************************************************************************/

void do_coff386()
{
	FILHDR	filhdr_buf;
	AOUTHDR	aouthdr_buf;
	SCNHDR	scnhdr_buf;
	unsigned short i;
	unsigned short buf_len;
	long	file_offset;
	long	read_size;
	long	phys_addr;
	
	file_offset = 0L;
	
	/* Verify Magic Number */

	if (my_read(&filhdr_buf, FILHSZ, fp)) {
		fprintf(stderr,"\n*** Error - Premature EOF Encountered\n");
		exit(1);
	}

	file_offset += (long)(unsigned)FILHSZ;
	
	if (filhdr_buf.f_magic != FILE_MAGIC_NUM) {
		fprintf(stderr,"\n*** Error - Bad Magic Number in File Header\n");
		exit(1);
	}

	if (my_read(&aouthdr_buf, AOUTHSZ, fp)) {
		fprintf(stderr,"\n*** Error - Premature EOF Encountered\n");
		exit(1);
	}

	file_offset += (long)(unsigned)AOUTHSZ;
	
	if (aouthdr_buf.magic != AOUT_MAGIC_NUM) {
		fprintf(stderr,"\n*** Error - Bad Magic Number in a.out header\n");
		exit(1);
	}

	for (i = 0; i < filhdr_buf.f_nscns; i++) {
		if (my_read(&scnhdr_buf, SCNHSZ, fp)) {
			fprintf(stderr,"\n*** Error - Premature EOF Encountered\n");
			exit(1);
		}

		if (scnhdr_buf.s_flags == STYP_TEXT)
			fprintf(ofile, "\nTEXT -     %8lu bytes to %08lxH\n",
					scnhdr_buf.s_size, scnhdr_buf.s_paddr);
		else if (scnhdr_buf.s_flags == STYP_DATA)
			fprintf(ofile, "\nDATA -     %8lu bytes to %08lxH\n",
					scnhdr_buf.s_size, scnhdr_buf.s_paddr);
				
		file_offset += (long)(unsigned)SCNHSZ;

		if ((scnhdr_buf.s_size > 0) && 
			(scnhdr_buf.s_flags == STYP_TEXT ||
			scnhdr_buf.s_flags == STYP_DATA)) {

			if (fseek(fp, scnhdr_buf.s_scnptr, 0)) {
				fprintf(stderr,"\n*** Error - Premature EOF Encountered\n");
				exit(1);
			}

			read_size = 0L;
			phys_addr = scnhdr_buf.s_paddr;
			while (read_size < scnhdr_buf.s_size) {
				buf_len = 
				(scnhdr_buf.s_size - read_size > (long)(unsigned)buf_size ? 
					buf_size:(scnhdr_buf.s_size - read_size));

				if (my_read(data_buf, buf_len, fp)) {
					fprintf(stderr,"\n*** Error - Premature EOF Encountered\n");
					exit(1);
				}
				
				fprintf(ofile, "    Downloading %5u bytes to %08lxH\n", 
												buf_len, phys_addr);

				do_download(&phys_addr, data_buf, buf_len);  	

				read_size += (long)(unsigned)buf_len;
				phys_addr += (long)(unsigned)buf_len;
			}
		}

		if (fseek(fp, file_offset, 0)) {
			fprintf(stderr,"\n*** Error - Premature EOF Encountered\n");
			exit(1);
		}
		
	}

	if (!load_only_flag) {	
		fprintf(ofile,"Start Address = %08lxH\n",aouthdr_buf.xentry);
	 	do_execute(aouthdr_buf.xentry);
	}
}

/****************************************************************************/
/*                                                                          */
/*                            TYPE TO NAME TRANSLATION                      */
/*                            ------------------------                      */
/*                                                                          */
/****************************************************************************/

char *rec_t2n(rec_type)
unsigned char rec_type;
	
{
	struct interp_rec *p;
	int i;
	p=interp_tab;
	for (i=0; i< (sizeof interp_tab)/(sizeof (struct interp_rec)); i++) {
		if (p->rec_type == rec_type)
			return(p->rec_name);
		p++;
	}

	return("??????");
}

/****************************************************************************/
/*                                                                          */
/*                            GET OMF TYPE                                  */
/*                            ------------                                  */
/*                                                                          */
/****************************************************************************/

int get_omf_type(opt_p)
	char *opt_p;
	
{
	struct omf_type_rec *p;
	int i;

	p = omf_type_tab;
	for (i=0; i< (sizeof omf_type_tab)/(sizeof (struct omf_type_rec)); i++) {
		if (strcmp(p->omf_type_name, opt_p) == 0)
			return(p->omf_type);
		p++;
	}

	return(-1);
}

/****************************************************************************/
/*                                                                          */
/*                            CHECK ADDRESS           		            */
/*                            -------------		                    */
/*                                                                          */
/****************************************************************************/

void check_addr(count, exec_addr)
unsigned char count;
unsigned long exec_addr;
{

	if (!load_only_flag) {	
		if (count >= 1) {
			if (count > 1) 
				fprintf(stderr,"\n%s: More Than One Valid Start Address - Previous Are Overwritten\n",progname); 
			fprintf(ofile,"Start Address = %08lxH\n",exec_addr);
			do_execute(exec_addr);
		}
		else
			if (start_addr != INVALID_ADDR) {
				fprintf(ofile,"Start Address = %08lxH\n",start_addr);
				do_execute(start_addr);
			}	
			else {
				fprintf(stderr,"\n%s: No Valid Start Address Found\n",progname); 
				exit(1);
			}	
	}
}

/****************************************************************************/
/*                                                                          */
/*                            USAGE           		                    */
/*                            -----			                    */
/*                                                                          */
/****************************************************************************/

usage()

{

	fprintf(stderr,"\nUsage: %s [-o omftype] [-b buffer size] [-t time delay] [-p portid] [-lrv] <file-name> <slot-id>\n", progname);

}
