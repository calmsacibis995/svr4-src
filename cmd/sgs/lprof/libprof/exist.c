/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libprof:exist.c	1.12"
#include <stdio.h>
#include <string.h>

#include "symint.h"
#include "covfile.h"
#include "retcode.h"
#include "filedata.h"
#include "cov_errs.h"

#include <sys/stat.h>

_CAclose_covf(filedata)
struct caFILEDATA *filedata;
{	
	void free();

	fclose(filedata->cov_obj_ptr);
	fclose(filedata->cov_data_ptr);
	free((char *) filedata);

	return;
 }


struct caFILEDATA *
_CAopen_covf(covfile)
char *covfile;
{
	char *malloc();
	struct caFILEDATA *filedata;


	filedata = (struct caFILEDATA *) malloc(sizeof(struct caFILEDATA));
	if (filedata != NULL)
	{	filedata->cov_obj_ptr = fopen(covfile,"r");
		filedata->cov_data_ptr = fopen(covfile,"r");
		
		if ( ( filedata->cov_obj_ptr == NULL ) ||
	     	     ( filedata->cov_data_ptr == NULL ))
			filedata = NULL;
		else
		{	filedata->use_flag = UPDATE;
			filedata->obj_cnt = 0;
			_CArewind(filedata);
	 	}
	 }
	else
		fprintf(stderr,"_CAopen_covf: %s\n",COV001);

	return(filedata);
 }



short
_CAfind(filedata, searchfunc)
struct caFILEDATA *filedata;
char *searchfunc;
{
	short ret_code;
	unsigned short fname_size;
	char *name;
	char *malloc();
	void free();
#if vax
	char *vaxfunction;
#endif
	long init_loc, file_offset;


#if vax
	/* is there a leading underscore in function name? */
	if(*searchfunc != '_') {
		vaxfunction = malloc((unsigned)strlen(searchfunc)+2);
		sprintf(vaxfunction,"_%s",searchfunc);  /* insert one */
	}
	else {
		vaxfunction = (char *)malloc((unsigned)strlen(searchfunc)+1);
		strcpy(vaxfunction,searchfunc);   /* already there */
	}
#endif

debugp3("_CAfind: seeking function ", searchfunc, "\n")

	init_loc = -1;
	while (1) {
	    file_offset = ftell(filedata->cov_data_ptr);
	    if (init_loc == -1) {
debug(	    fprintf(stderr,"_CAfind: first time, offset==%lo(octal)\n",
		file_offset);
     )
		/* first time through */
		init_loc = file_offset ;
	    }
	    else {
debug(	    	fprintf(stderr,"_CAfind: another time, offset==%lo(octal)\n",
			file_offset);
     )
		/* have we wrapped completely around? */
		if (file_offset == init_loc) {
		    /* searched all functions */
		    ret_code = FUNC_FAIL;
		    break;
		}
	    }
	    if ((fread((char *) &fname_size, sizeof(fname_size), 1, filedata->cov_data_ptr)) != 0) {
		if (fname_size == EOD) {
		    /* wrap around to beginning */
		    _CArewind(filedata);
		    /* go back to top of loop */
		    continue;
		}

		name = (char *) malloc(fname_size+1);
		fread(name, (int)fname_size, 1, filedata->cov_data_ptr);
		/* make null-terminated */
		name[fname_size] = '\0';

debugsd(	"_CAREAD: from file - fname_size==", fname_size)
debugp3(	" AND name is ", name, "\n")

#if vax
		if (strcmp(name,vaxfunction) == 0)
#else
		if (strcmp(name,searchfunc) == 0)
#endif
		{	/* this is the function, move
			  ptr back to beginning of
			  function name           */
		    fseek(filedata->cov_data_ptr,-(long)(fname_size+sizeof(fname_size)),1);
		    ret_code = OK;
		    break;
		}
		else 	/* this is not it, move to next function */
		{
		    if (_CAjump(filedata->cov_data_ptr) == EOF_FAIL)
		    {	/* error - end of file found */
			ret_code = FUNC_FAIL;
			break;
		    }
		 }
		 free(name);
	    }
	    else {
		/* error */
		ret_code = FUNC_FAIL;
		break;
	    }
	}

#if vax
	free(vaxfunction);
#endif

debugp2("_CAfind: ret_code==",
	(ret_code==EOF_FAIL?	"EOF_FAIL\n":
	 (ret_code==OK?		"OK\n" :
				"Other\n"))
      )
	return(ret_code);
}



_CAjump(fileptr)			
FILE *fileptr;
{

    /* move fileptr past coverage array */

	short ret_code;
	    /* for xprof */
	unsigned char xca_size;
	char xunit_buf;
	    /* for lprof */
	caCOVWORD lca_words;

	ret_code = OK;
	if ((fread((char *) &lca_words, sizeof(lca_words), 1, fileptr)) != 0) {
	    if((fseek(fileptr, lca_words*sizeof(caCOVWORD), 1)) != 0) {
		/* end of file */
		ret_code = EOF_FAIL;
	    }
	}
	else	/* end of file */
	    ret_code = EOF_FAIL;

debugp2("_CAjump: ret_code==",
	(ret_code==EOF_FAIL?	"EOF_FAIL":
	 (ret_code==OK?		"OK" :
				"Other"))
      )
	return(ret_code);
 }





char *
_CAleaf(path)
char *path;
{
	/* look for objectfile of same name - ignore path */
	char *s;

	return(((s = strrchr(path, '/')) == NULL) ? path : s + 1);
}

static char *string_warn =
"***Warning: Profiled object file names are different, but time stamps match.\n\
\tAssuming correct object file.***\n";

short
_CAfind_obj(filedata, objectfile,obj_entry)
struct caFILEDATA *filedata;
char *objectfile;
struct  caOBJ_ENTRY *obj_entry;
{
	struct caCOV_HDR	hdr_buf;
	PROF_FILE		*objfile;
	short 			_CAobj_test();

	/* from the beginning of file, seek past COVFILE header */
	rewind(filedata->cov_obj_ptr);
	fread(
		(char *) &hdr_buf,
		sizeof(struct caCOV_HDR),
		1,
		filedata->cov_obj_ptr
	);

	/* read object file entry */
	if (
		fread(
			(char *) obj_entry,
			sizeof(struct caOBJ_ENTRY),
			1,
			filedata->cov_obj_ptr
		) != 0
	) {
		if (
			(objectfile == NULL)
			|| (
				strcmp(
					_CAleaf(obj_entry->name)
					, _CAleaf(objectfile)
				) == 0
			)
		) {
			goto success;
		} else if (_CAobj_test(objfile, obj_entry) == OK) {
			fprintf(stderr, string_warn);
			goto success;
		}
	} else {
		fprintf(stderr,"_CAfind_obj: '%s'\n",COV002);
	}
	return(COVOBJ_FAIL);

success:;
	/* seek data ptr to the offset specified by this entry*/
	fseek(filedata->cov_data_ptr, obj_entry->offset, 0);
	return(OK);
}




short
_CAtraverse(filedata, name)
struct caFILEDATA *filedata;
char **name;

    /*  _CAtraverse steps through COVFILE returning name of next function */
    /*  Assumes data_ptr is on the next caCOV_DATA to be read */

{
    short ret_code;
    unsigned short fname_size;
    char *malloc();

    if ((fread((char *) &fname_size, sizeof(fname_size), 1, filedata->cov_data_ptr)) != 0) {
	*name = (char *) malloc(fname_size+1);
	fread((*name), (int)fname_size, 1, filedata->cov_data_ptr);
	/* make null-terminated */
	(*name)[fname_size] = '\0';
	/* move file pointer to next function */
	if (_CAjump(filedata->cov_data_ptr) == EOF_FAIL)
	{
	    /* end of file found */
	    ret_code = EOF_FAIL;
	}
	ret_code = OK;
    }
    else {
    /* end of file found before expected */
	ret_code = COV_FAIL;
    }
    return(ret_code);
}
	








short
_CAread(filedata,dataptr)
struct caFILEDATA *filedata;
struct caCOV_DATA *dataptr;
{
	short ret_code;
	char *malloc();
	void free();
debug(	long file_offset;)

debugp1("_CAread: entry.\t")
debug(
	file_offset = ftell(filedata->cov_data_ptr);
	fprintf(stderr,"file ptr==%-x, offset==%-x\n",
		filedata->cov_data_ptr, file_offset );
     )

	ret_code = OK;
	/* read function size and name into buffer */
	if (
		fread(
			(char *) &dataptr->fname_size,
			sizeof(dataptr->fname_size),
			1,
			filedata->cov_data_ptr
		) == 0
	) {
		ret_code = EOF_FAIL;
		goto theend;
	}

debugsd("_CAread: got fname_size==",dataptr->fname_size)
debugp1("\n")

	/* is this the 'end of data' ? */
	if (dataptr->fname_size == EOD) {
		ret_code = EOD_FAIL;
		goto theend;
	}

	dataptr->func_name = (unsigned char *)malloc(dataptr->fname_size+1);
	if (
		fread(
			(char *) dataptr->func_name,
			(int)dataptr->fname_size,
			1,
			filedata->cov_data_ptr
		) == 0
	) {
		free((char *)dataptr->func_name);
		ret_code = EOF_FAIL;
		goto theend;
	}

	/* make sure name string ends with a null */
	dataptr->func_name[dataptr->fname_size] = '\0';

debugp3("_CAread: function name read; is: ", dataptr->func_name, "\n")

	/* read array size into buffer */
	if (
		fread(
			(char *) &dataptr->lca_words,
			sizeof(dataptr->lca_words),
			1,
			filedata->cov_data_ptr
		) != 0
	) {
		/* allocate coverage array */
		dataptr->lca_counts = (caCOVWORD *)
			malloc(dataptr->lca_words*(sizeof(caCOVWORD)));

		/* read the coverage array */
		if (
			fread(
				(char *)dataptr->lca_counts,
				sizeof(caCOVWORD),
				dataptr->lca_words,
				filedata->cov_data_ptr
			) != dataptr->lca_words
		) {
			/* unexpected 'end of file' */
			free((char *)dataptr->func_name);
			free((char *)dataptr->lca_counts);
			ret_code = EOF_FAIL;
		} else {
			dataptr->lca_bblks = dataptr->lca_words/2;
			dataptr->lca_lineos =
				dataptr->lca_counts + dataptr->lca_bblks;
		}
	}

debugsd("_CAread: got #words==", dataptr->lca_words)
debugp1(" \n")


theend:;

debug(	fprintf(stderr,"_CAread, returncode=%s\n",
		((ret_code == EOF_FAIL ? "EOF_FAIL" :
		 (ret_code == EOD_FAIL ? "EOD_FAIL" : 
		 (ret_code == OK ?       "OK" : "other_FAIL" ))))
		);
     )
	return(ret_code);
}


short
_CAobj_test(objfile,obj_entry)
PROF_FILE *objfile;
struct caOBJ_ENTRY *obj_entry;
{
	short ret_code = OK;
	struct stat buf;
	
	/* do times match? */
	fstat(objfile->pf_fildes,&buf);
	if (obj_entry->time != buf.st_mtime)
		ret_code = TIME_FAIL;

	return(ret_code);
}




_CArewind(filedata)
struct caFILEDATA *filedata;
{
	struct caOBJ_ENTRY obj_entry;

	/* rewind and seek past header */
	fseek(filedata->cov_obj_ptr,sizeof(struct caCOV_HDR),0);
	/* read first objectfile entry */
	if(fread((char *) &obj_entry,sizeof(struct caOBJ_ENTRY),1,filedata->cov_obj_ptr) != 0)
		/* rewind data pointer */
		fseek(filedata->cov_data_ptr,obj_entry.offset,0);
	/* back-up objectfile entry  pointer */
	fseek(filedata->cov_obj_ptr,-sizeof(struct caOBJ_ENTRY),1);
 }



short
_CAvalid_test(filedata)
struct caFILEDATA *filedata;
{
	struct caCOV_HDR hdrbuf;

	short ret_code;

	ret_code = OK;

	/* rewind and read header */
	rewind(filedata->cov_obj_ptr);
	if(fread((char *) &hdrbuf,sizeof(struct caCOV_HDR),1,filedata->cov_obj_ptr) != 0)
	{
#if vax
		if(hdrbuf.mach_type != MVAX)
			ret_code = ret_code | MACH_MASK;
#endif
#if u3b
		if(hdrbuf.mach_type != MSIMPLEX)
			ret_code = ret_code | MACH_MASK;
#endif
#if (u3b15 || u3b2)
		if(hdrbuf.mach_type != M32)
			ret_code = ret_code | MACH_MASK;
#endif
#if (i386)
                if(hdrbuf.mach_type != M386)
                        ret_code = ret_code | MACH_MASK;
#endif
#if (!(vax) && !(u3b) && !(u3b15) && !(u3b2) && !(i386))
			ret_code = ret_code | MACH_MASK;
#endif
		if(hdrbuf.ca_ver != VERSION)
			ret_code = ret_code | VER_MASK;
		if(hdrbuf.comp_flag == FALSE)
			ret_code = ret_code | COMP_MASK;
	 }
	else
	{
		fprintf(stderr,"_CAvalid_test: %s\n",COV303);
		ret_code = BUG_FAIL;
	 }

	return(ret_code);
 }



short
_CAobj_merge(filedata, obj_ent1, obj_ent2)
struct caFILEDATA *filedata;
struct caOBJ_ENTRY *obj_ent1, *obj_ent2;
{
    short ret_code = OK;

    struct caOBJ_ENTRY obj_buf, *newer;

    /* prepare objectfile entry */
    /* we want the more recent obj_ent */
    if (obj_ent1->time <= obj_ent2->time)
	newer = obj_ent2;
    else
	newer = obj_ent1;
    strncpy(obj_buf.name, newer->name, ONAMESIZE);
    obj_buf.time = newer->time;
    obj_buf.magic_no = newer->magic_no;
    obj_buf.offset = newer->offset;
    /* write objectfile entry */
    fseek(filedata->cov_data_ptr, sizeof(struct caCOV_HDR),0);
    if ((fwrite((char *) &obj_buf, sizeof(struct caOBJ_ENTRY),1,filedata->cov_data_ptr)) != 0)
    {
	/* increment object file count */
	filedata->obj_cnt++;
    }
    else
    {
	ret_code = BUG_FAIL;
	fprintf(stderr, "_CAobj_entry: %s\n", COV301);
    }
    return(ret_code);
}
