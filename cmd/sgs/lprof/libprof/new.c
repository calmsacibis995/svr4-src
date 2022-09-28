/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libprof:new.c	1.12"

#include "hidelibc.h"		/* uses "_" to hide libc functions */
#include "hidelibelf.h"		/* uses "_" to hide libelf functions */

#include <stdio.h>
#include <string.h>

#include "symint.h"
#include "filedata.h"
#include "covfile.h"
#include "retcode.h"
#include "cov_errs.h"
#include "debug.h"

/*
*	These are included for use of the fstat function which
*	gives us access to the time stamp on a file.
*/
#include <sys/stat.h>

struct caFILEDATA *_CAcreate_covf(covfile)
char *covfile;
{
	char *malloc();
	void free();
	char *strcpy();

	struct caFILEDATA *filedata;
	struct caCOV_HDR hdr_buf;
	struct caOBJ_ENTRY obj_buf;

	short count;


	/* allocate filedata structure */
	filedata = (struct caFILEDATA *) malloc(sizeof(struct caFILEDATA));
	if (filedata != NULL)
	{
		filedata->cov_obj_ptr = NULL;
		/* open stream to the file */
		filedata->cov_data_ptr = fopen(covfile,"w+");

		if (filedata->cov_data_ptr == NULL)
		{
			/* open failed */
			free((char *) filedata);
			filedata = NULL;
		 }
		else
		{
			/* fill in rest of filedata structure */
			filedata->obj_cnt = 0;
			filedata->use_flag = CREATE;
			/* fill in covfile header buffer */
			hdr_buf.hdr_size = sizeof(struct caCOV_HDR);
#if vax
			hdr_buf.mach_type = MVAX;
#endif
#if u3b
			hdr_buf.mach_type = MSIMPLEX;
#endif
#if (u3b15 || u3b2)
			hdr_buf.mach_type = M32;
#endif
#if i386
                        hdr_buf.mach_type = M386;
#endif
#if !(vax || u3b || u3b15 || u3b2 || i386)
			hdr_buf.mach_type = UNKNOWN;
#endif
			hdr_buf.ca_ver = VERSION;
			hdr_buf.edit_flag = FALSE;
			hdr_buf.comp_flag = FALSE;
			hdr_buf.no_obj = 0;
			/* write covfile header */
			if (
				fwrite(
					(char *) &hdr_buf,
					sizeof(struct caCOV_HDR),
					1,
					filedata->cov_data_ptr
				) != 0
			) {
				/* build null object file entry */
				(void)strcpy((char *) obj_buf.name, NULLOBJ);
				obj_buf.time = 0;
				strncpy(
					(char *) obj_buf.magic_no.pe_ident,
					PROF_MAGIC_FAKE_STRING,
					EI_NIDENT
				);
				obj_buf.magic_no.pe_type = NULL;
				obj_buf.offset = 0;
				count = OBJMAX;
				/* write OBJMAX null objectfile entries */
				while(count--)
					if(
						fwrite((char *) &obj_buf,
						   sizeof(struct caOBJ_ENTRY),
						   1,
						   filedata->cov_data_ptr
						) == 0
					) {
						/* can't write */
						free((char *) filedata);
						filedata = NULL;
						fprintf(stderr,"_CAcreate_covf: %s\n",COV301);
					}
			 }
			else
			{
				/* header write failed */
				free((char *) filedata);
				filedata = NULL;
				fprintf(stderr,"_CAcreate_covf: %s\n",COV301);
			 }
		 }
	 }
	
	return(filedata);
 }





short
_CAcomp_covf(filedata,no_obj)
struct caFILEDATA *filedata;
short no_obj;
{
	void	rewind();

	struct caCOV_HDR hdr_buf;

	short ret_code;


	ret_code = OK;
	if (filedata->obj_cnt == no_obj)
	{
		long EOD_value = EOD;	/* ``long enough'' for below */

		/* * * * * *
		 * write `EOD`, for length of a ``caCOV_DATA.fname_size.''
		 */
		fwrite(
			(char *) &EOD_value,
			sizeof(((struct caCOV_DATA *)0)->fname_size),
			1,
			filedata->cov_data_ptr
		      );

		rewind(filedata->cov_data_ptr);
		/* read covfile header */
		if (fread((char *) &hdr_buf, sizeof(struct caCOV_HDR),1,filedata->cov_data_ptr) != 0)
		{
			hdr_buf.comp_flag = TRUE;      /* set completion flag */
			hdr_buf.no_obj = no_obj;       /* copy #objectfile entries */
			rewind(filedata->cov_data_ptr);
			/* rewrite header */
			if (fwrite((char *) &hdr_buf, sizeof(struct caCOV_HDR),1,filedata->cov_data_ptr) == 0)
			{
				/* can't rewrite */
				fprintf(stderr,"_CAcomp_covf: %s\n",COV302);
				ret_code = COMP_FAIL;
			 }
		 }
		else
		{
			/* can't read header */
			fprintf(stderr,"_CAcomp_covf: %s\n",COV303);
			ret_code = COMP_FAIL;
		 }
	 }
	else
		ret_code = COMP_FAIL;	/* objectfile counts do not match */

	/* close stream to the file */
	fclose(filedata->cov_data_ptr);

	return(ret_code);
 }



short
_CAdata_entry(filedata,data)
struct caFILEDATA *filedata;
struct caCOV_DATA *data;
{
	short ret_code;

	DEBUG(printf("_CAdata_entry: top\n"));

	ret_code = OK;
	if ((unsigned int)filedata->obj_cnt >= 1) {
		/* write data entry to COVFILE */
		DEBUG(printf("_CAdata_entry: writing fname_size\n"));
		if (
			fwrite(
				(char *) &data->fname_size,
				sizeof(data->fname_size),
				1,
				filedata->cov_data_ptr
			) != 0
		) {
			DEBUG(printf("_CAdata_entry: writing func_name\n"));
			fwrite(
				(char *)data->func_name,
				1,
				(int)data->fname_size,
				filedata->cov_data_ptr
			);
			DEBUG(printf("_CAdata_entry: func_name written\n"));
			DEBUG(printf("_CAdata_entry: writing lca_words\n"));
			fwrite(
				(char *) &data->lca_words,
				sizeof(data->lca_words),
				1,
				filedata->cov_data_ptr
			);
			/* write coverage array */
			DEBUG(printf("_CAdata_entry: writing array\n"));
			if (
				data->lca_words != fwrite(
					(char *) data->lca_counts,
					sizeof(caCOVWORD),
					data->lca_words,
					filedata->cov_data_ptr
				
				)
			  )
			{
				ret_code = BUG_FAIL;
				fprintf(stderr,"_CAdata_entry: %s\n",COV301);
			}
		 } else {
			ret_code = BUG_FAIL;
			fprintf(stderr,"_CAdata_entry: %s\n",COV301);
		 }
	} else	{
		/* there is no objectfile entry in the covfile yet */
		ret_code = COVOBJ_FAIL;
	}

	DEBUG(printf("_CAdata_entry: bottom\n"));
 	return(ret_code);
 }



/*
*	The second parameter to _CAobj_entry used to be a pointer
*	to the name of the object (executable) file.  _CAobj_entry
*	would ldopen the file, extract the required information
*	(date, time, magic number, etc) and then do an ldclose.
*	Because the information from _symintOpen is more easily
*	accessed, this open process is no longer required.  Instead,
*	_CAobj_entry can read the information directly.  Thus,
*	the second parameter becomes a pointer to the PROF_FILE
*	which was taken from the first _symintOpen.
*
*	WFM 12/20/88
*/
short
_CAobj_entry(filedata, objfile, objname)
struct caFILEDATA *filedata;
PROF_FILE *objfile;
char *objname;
{
	short ret_code;
	PROF_FILE *ofileptr;

	struct caOBJ_ENTRY obj_buf;
	struct stat buf;

	ret_code = OK;
	ofileptr = objfile;
	if (ofileptr != NULL)
	{
	   if((unsigned int)filedata->obj_cnt < OBJMAX)
	   {
		/* prepare objectfile entry */
		strncpy(obj_buf.name,objname,ONAMESIZE);
		fstat(ofileptr->pf_fildes,&buf);
		obj_buf.time = buf.st_mtime;
		strncpy(
			(char *) obj_buf.magic_no.pe_ident,
			(char *) ofileptr->pf_elfhd_p->e_ident,
			EI_NIDENT
			);
		obj_buf.magic_no.pe_type = ofileptr->pf_elfhd_p->e_type;
		obj_buf.offset = ftell(filedata->cov_data_ptr);
		/* write objectfile entry */
		/*  must reposition cov_data_ptr to write obj ent */
		fseek(filedata->cov_data_ptr, sizeof(struct caCOV_HDR),0);
		if (
			(fwrite(
				(char *) &obj_buf,
				sizeof(struct caOBJ_ENTRY),
				1,
				filedata->cov_data_ptr
			)) != 0
		) {
			/* increment objectfile count */
			filedata->obj_cnt++;
 		} else {
			ret_code = BUG_FAIL;
			fprintf(stderr,"_CAobj_entry: %s\n",COV301);
		}
	    }
	    else
	    {   fprintf(stderr,"_CAobj_entry: %s\n",COV304);
		ret_code = FULL_FAIL;
   	    }
	}
	else
	{	fprintf(stderr,"_CAobj_entry: %s\n",COV305);
		ret_code = OBJ_FAIL;
	}

	return(ret_code);
 }
