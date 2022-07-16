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

/*LINTLIBRARY*/
#ident	"@(#)mbus:cmd/bootutils/sgib.d/mkboot.c	1.3"

#include <a.out.h>
#include <sys/types.h>
#include <sys/fdisk.h>
#include <sys/ivlab.h>
#include <ldfcn.h>
#include <memory.h>
#include "sgib.h"

#include <stdio.h>
#include <fcntl.h>
#include <libelf.h>
/*
 * external declarations
 */

extern Elf *elf_begin();
extern int elf_end();
extern unsigned elf_version();
extern off_t elf_update();
extern Elf_Kind elf_kind();
extern Elf32_Ehdr *elf_getehdr();
extern Elf32_Phdr *elf_getphdr();
extern Elf32_Shdr *elf_getshdr();
extern Elf_Data *elf_getdata();
extern Elf_Scn *elf_nextscn();

extern off_t	lseek();
extern void	*malloc();
extern void	free();
extern void	exit();
extern int	debug;
extern char	*boot;
extern char	*rboot;
extern int	rboot_flag;
extern uint	msa_text_size;
extern uint	msa_text_loc;
extern uint	msa_data_size;
extern char	*real_buf_1;
extern char	*real_buf_2;
extern char	*msa_text_buf;
extern struct	btblk btblk;

extern int errno;


int 		bootfd;		/* boot file descriptor */
FILHDR		filehdr;	/* see filehdr.h */
AOUTHDR		aouthdr;	/* see aouthdr.h */

/* structures to be used for the reading of an ELF bootable */
Elf		*elfd;		/* ELF file descriptor */
Elf32_Ehdr	*ehdr;		/* ELF executable header structure */
Elf32_Phdr	*ephdr, *phdr;	/* ELF program header structure */
int		elfboot = 0;    /* flag to designate either ELF or COFF boot */


void
mkboot()
{
	int i;
	Elf_Scn *scn;
	Elf32_Shdr *eshdr;
	int nbytes;
	Elf_Data *data;

	if ((bootfd = open(boot, O_RDONLY)) == -1) { 
		ERR(stderr, "Can not open '%s'\n", boot);
		exit(1);
	}

	(void)lseek(bootfd, 0, 0);
	if (elf_version (EV_CURRENT) == EV_NONE) {
		ERR(stderr, "ELF access library out of date\n");
		exit (31);
	}
	if ((elfd = elf_begin (bootfd, ELF_C_READ, (Elf *)0)) == NULL) {
		ERR(stderr, "can't get elf file descriptor for '%s'\n", boot);
		exit(31);
	}
	if ((elf_kind(elfd) != ELF_K_COFF) && (elf_kind(elfd) != ELF_K_ELF)) {
		ERR(stderr,"Invalid Boot file, neither ELF nor COFF executable \n");
		exit(31);
	}

	if (elf_kind(elfd) == ELF_K_COFF) 
		if ((elf_update(elfd, ELF_C_NULL) == -1)) {
			ERR(stderr,"Cannot update COFF file to use elf library calls\n");
			exit(31);
		}

	if ((ehdr = elf32_getehdr(elfd)) == NULL) {
		ERR(stderr, "can't get ELF header for '%s'\n",boot);
		elf_end(elfd);
		exit(31);
	}
	if ((ephdr = elf32_getphdr (elfd)) == NULL) {
		ERR(stderr, "can't get ELF program header for '%s'\n",boot);
		exit(31);
	}

	/* the following code assumes that there is only one text segment */
	/* in the boot file; there may be multiple data segments          */

	msa_data_size = 0;
	for (i = 0; i < (int)ehdr->e_phnum; i++) {

	if ((ephdr[i].p_type == PT_LOAD) && (ephdr[i].p_filesz > 0)
		&& (ephdr[i].p_flags == (PF_R | PF_X))) {
		/* if this is a text segment */

		msa_text_size = (uint)(ephdr->p_filesz + (DISKBUF - 
						(ephdr->p_filesz % DISKBUF)));
		msa_text_loc = REAL_BOOT_SIZE;

		if ((msa_text_buf = (char *)malloc(msa_text_size)) == NULL) {
			ERR(stderr, "malloc of msa text buffer failed\n");
			exit(1);
		}

		(void)memset(msa_text_buf, 0, (int)msa_text_size);
		phdr = &ephdr[i];
		
		for (scn = NULL; scn = elf_nextscn(elfd, scn);) {
			if ((eshdr = elf32_getshdr (scn)) == NULL) {
				free (msa_text_buf);
				printf ("Invalid '%s' file, empty msa text section\n", boot);
				exit(31);
			}
			if (eshdr->sh_addr >= phdr->p_vaddr && 
			eshdr->sh_addr < phdr->p_vaddr + phdr->p_filesz &&
		    	eshdr->sh_type == SHT_PROGBITS &&
		    	eshdr->sh_flags & (SHF_ALLOC | SHF_EXECINSTR)) {


				if ((data = elf_getdata (scn,(Elf_Data *)0)) == 
     					NULL || data->d_buf == NULL) {
                                	free (msa_text_buf);
					ERR(stderr, "Invalid '%s' file, no msa text data\n", boot);
					exit(31);
				}
				nbytes = eshdr->sh_size;

				if (eshdr->sh_addr + eshdr->sh_size > 
					phdr->p_vaddr + phdr->p_filesz)
                                	nbytes -= eshdr->sh_addr + 
						eshdr->sh_size - 
					  phdr->p_vaddr - phdr->p_filesz;

				if (nbytes >= msa_text_size) {
					ERR(stderr, "Invalid '%s' file, file is too large\n",boot);
					exit(1);
				}
				memcpy (&msa_text_buf[eshdr->sh_addr -  phdr->p_vaddr], (char *) data->d_buf, nbytes); 
				}  /* end if valid text section */
			}  /* end for loop to scan section headers */
		} else  
			/* if this is a data segent */
			if ((ephdr[i].p_type == PT_LOAD) && 
			(ephdr[i].p_memsz > 0) && 
			(ephdr[i].p_flags & PF_W))  
				msa_data_size += (uint)ephdr[i].p_memsz;
	}  /* end for loop to scan segment headers */

	/* if text size is still 0 there was no loadable segment */
	if (msa_text_size == 0)
		ERR(stderr, "Invalid '%s' file, no loadable ELF segment\n", boot);
		
	if(rboot_flag == 0 ) {
		if ((real_buf_2  = (char *)malloc((unsigned)REAL_2_SIZE)) == NULL) {
			ERR(stderr, "malloc of real mode buffer2 for file '%s' failed\n",rboot);
			exit(1);
		}
		if ((real_buf_1  = (char *)malloc((unsigned)REAL_1_SIZE)) == NULL) {
			ERR(stderr, "malloc of real mode buffer1 for file '%s' failed\n",rboot);
			exit(1);
		}
		(void)memset(real_buf_1, 0, REAL_1_SIZE);
		(void)memset(real_buf_2, 0, REAL_2_SIZE);
	}
	(void)elf_end(elfd);
}

void
mkrboot()
{
	char * tempbuf;
	uint real_text_size;
	int i;
	Elf_Scn *scn;
	Elf32_Shdr *eshdr;
	int nbytes;
	Elf_Data *data;


	if ((bootfd = open(rboot, O_RDONLY)) == -1) { 
		ERR(stderr, "Can not open '%s'\n", rboot);
		exit(1);
	}

	lseek(bootfd, 0, 0);
	if (elf_version (EV_CURRENT) == EV_NONE) {
		ERR(stderr, "ELF access library out of date\n");
		exit (31);
	}
	if ((elfd = elf_begin (bootfd, ELF_C_READ, (Elf *)0)) == NULL) {
		ERR(stderr, "can't get elf file descriptor for '%s'\n", rboot);
		exit(31);
	}
	if ((elf_kind(elfd) != ELF_K_COFF) && (elf_kind(elfd) != ELF_K_ELF)) {
		ERR(stderr,"Invalid Boot file, neither ELF nor COFF executable \n");
		exit(31);
	}

	if (elf_kind(elfd) == ELF_K_COFF) 
		if ((elf_update(elfd, ELF_C_NULL) == -1)) {
			ERR(stderr,"Cannot update COFF file to use elf library calls\n");
			exit(31);
		}

	if ((ehdr = elf32_getehdr(elfd)) == NULL) {
		ERR(stderr, "can't get ELF header for '%s'\n",rboot);
		elf_end(elfd);
		exit(31);
	}
	if ((ephdr = elf32_getphdr (elfd)) == NULL) {
		ERR(stderr, "can't get ELF program header for '%s'\n",rboot);
		exit(31);
	}

	if ((tempbuf  = (char *)malloc((unsigned)REAL_BOOT_SIZE)) == NULL) {
		ERR(stderr, "malloc of temporary real boot buffer failed\n");
		exit(1);
	}

	/* the following code assumes that there is only one text segment */
	/* in the boot file; there may be multiple data segments          */

	real_text_size = 0;
	for (i = 0; i < (int)ehdr->e_phnum; i++) {

	if ((ephdr[i].p_type == PT_LOAD) && (ephdr[i].p_filesz > 0)
		&& (ephdr[i].p_flags == (PF_R | PF_X))) {
		/* if this is a text segment */

		real_text_size = ephdr[i].p_filesz;
		phdr = &ephdr[i];
		
		for (scn = NULL; scn = elf_nextscn(elfd, scn);) {
			if ((eshdr = elf32_getshdr (scn)) == NULL) {
				free (tempbuf);
				printf ("Invalid '%s' file, empty text section\n", rboot);
				exit(31);
			}
			if (eshdr->sh_addr >= phdr->p_vaddr && 
			eshdr->sh_addr < phdr->p_vaddr + phdr->p_filesz &&
		    	eshdr->sh_type == SHT_PROGBITS &&
		    	eshdr->sh_flags & (SHF_ALLOC | SHF_EXECINSTR)) {


				if ((data = elf_getdata (scn,(Elf_Data *)0)) == 
     					NULL || data->d_buf == NULL) {
                                	free (tempbuf);
					ERR(stderr, "Invalid '%s' file, no  text data\n", rboot);
					exit(31);
				}
				nbytes = eshdr->sh_size;

				if (eshdr->sh_addr + eshdr->sh_size > 
					phdr->p_vaddr + phdr->p_filesz)
                                	nbytes -= eshdr->sh_addr + 
						eshdr->sh_size - 
					  phdr->p_vaddr - phdr->p_filesz;

				if (nbytes >= REAL_BOOT_SIZE) {
					ERR(stderr, "Invalid '%s' file, file is too large\n",rboot);
					exit(1);
				}
				memcpy (&tempbuf[eshdr->sh_addr -  phdr->p_vaddr], (char *) data->d_buf, nbytes); 

				}  /* end valid text sections */
			}  /* end for loop to scan text sections */
		} else  
			/* if this is a data segment */ 
			if ((ephdr[i].p_type == PT_LOAD) && 
			(ephdr[i].p_memsz > 0) && 
			(ephdr[i].p_flags & PF_W)) { 
		phdr = &ephdr[i];
		
		for (scn = NULL; scn = elf_nextscn(elfd, scn);) {
			if ((eshdr = elf32_getshdr (scn)) == NULL) {
                                	free (tempbuf);
					printf ("Invalid '%s' file, empty data section\n", rboot);
					exit(31);
			}
			if (eshdr->sh_addr >= phdr->p_vaddr && 
			eshdr->sh_addr < phdr->p_vaddr + phdr->p_memsz &&
		    	eshdr->sh_type == SHT_PROGBITS &&
		    	eshdr->sh_flags & (SHF_ALLOC | SHF_WRITE)) {

				if ((data = elf_getdata (scn, (Elf_Data *)0)) == 
     					NULL || data->d_buf == NULL) {
					printf ("Empty data section found\n");
					continue;
				}
				nbytes = eshdr->sh_size;
				if (eshdr->sh_addr + eshdr->sh_size > 
					phdr->p_vaddr + phdr->p_memsz)
                                	nbytes -= eshdr->sh_addr + 
						eshdr->sh_size - 
					  phdr->p_vaddr - phdr->p_memsz;

				if (nbytes >= REAL_BOOT_SIZE) {
					ERR(stderr, "Invalid '%s' file, file is too large\n",rboot);
					exit(1);
				}
				memcpy (&tempbuf[eshdr->sh_addr -  phdr->p_vaddr], (char *) data->d_buf, nbytes); 
				}  /* end if valid data section */
			}  /* end for loop to scan data sections */
		}  /* end if valid data segment */
	}  /* end for loop to scan segment headers */

	/* if text size is still 0 there was no loadable segment */
	if (real_text_size == 0)
		ERR(stderr, "Invalid '%s' file, no loadable ELF segment\n", rboot);

	if ((real_buf_2  = (char *)malloc((unsigned)REAL_2_SIZE)) == NULL) {
		ERR(stderr, "malloc of second stage boot buffer for file '%s' failed\n",rboot);
		exit(1);
	}
	if ((real_buf_1  = (char *)malloc((unsigned)REAL_1_SIZE)) == NULL) {
		ERR(stderr, "malloc of first stage boot buffer for file '%s' failed\n",rboot);
		exit(1);
	}
	(void)memset(real_buf_1, 0, REAL_1_SIZE);
	(void)memset(real_buf_2, 0, REAL_2_SIZE);
	(void)memcpy(real_buf_1, tempbuf,  REAL_1_SIZE);
	(void)memcpy(real_buf_2, &tempbuf[REAL_1_SIZE], REAL_2_SIZE);

	(void)elf_end(elfd);
}
