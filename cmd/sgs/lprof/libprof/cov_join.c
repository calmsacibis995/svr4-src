/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libprof:cov_join.c	1.8"
#include <stdio.h>
#include <string.h>
#include <filehdr.h>
/*
#include <ldfcn.h>
*/

#include "symint.h"
#include "covfile.h"
#include "retcode.h"
#include "filedata.h"
#include "lst_str.h"
#include "cov_errs.h"

#define		MRG_fail	99

#define		NOT_OK		1

#define		BADMASK		1
#define		BADCOMP		2
#define		OVREDIT		3
#define		OVRCOMP		4
#define		EDITFLG		5

#define		OVER		1
#define		NAME		2
#define		NOVR		3
#define		C_OK		5

#define	CAerror(message)	fprintf(stderr, "\n_CAcov_join: %s\n", message)

int	_tso_flag;	/*  Time Stamp Override Flag */
int	_com_flag;	/*   COMplete Override Flag  */


    /* Time Stamp: If the time stamps of the object file entries in the two
	COVFILEs differ, there is a possibility that the source for that file
	was changed between the times that the two COVFILEs were generated.
	If the override is set, the routine looks for new portions in each */

    /* COMP Flag:  If the COMPlete flag is not set on a COVFILE, it may be
	because an aborted attempt to write to the COVFILE, thus we cannot be
	assured that the proper EOF and EOD bytes can be found in the COVFILE.
	If the override is set, any faults due to missing EOD or EOF will be
	handled with a warning to the user. If not, errors will be reported and   
	there will be no merge for the function(s) in question.  */

_CAcov_join(cov1,cov2,cov3)
char  *cov1, *cov2, *cov3;			/* COVFILE names	*/
{
	char   *malloc(),
	       *strncpy();
	int    strncmp();
	void   free();

	struct caFILEDATA *_CAopen_covf();
	struct caFILEDATA *_CAcreate_covf();
	short  _CAread(),
	       _CAfind_obj(),
	       _CAcomp_covf(),
	       _CAdata_entry(),
	       _CAobj_merge(),
	       _CAfind();
	short  ret_code, head_code, obj_code;	/* Return Code variables*/
	short  over_flag;			/* Various Flags        */
	short  oused_flag, null_arg, null2_arg;	/*    "      "		*/
	short  entry_flag;			/* Target Created Flag	*/
	short  obj_fcount;			/* Object File Counter  */
	short  i;				/*     Loop Counter     */

	/* Object File Entry buffers:	*/
	struct caOBJ_ENTRY  *obj1_buff;		/* Entry for COVFILE #1 */
	struct caOBJ_ENTRY  *obj2_buff;		/* Entry for COVFILE #2 */

	/* Coverage Data buffers:	*/
	struct caCOV_DATA   *dbuff_1;		/* Data from COV #1   	*/
	struct caCOV_DATA   *dbuff_2;		/* Data from COV #2	*/
	struct caCOV_DATA   *dbuff_3;		/* Data for COV #3      */

	/* COVFILE Filedata buffers:	*/
	struct caFILEDATA   *data1_file;	/* Filedata of COV #1	*/
	struct caFILEDATA   *data2_file;	/* Filedata of COV #2	*/
	struct caFILEDATA   *data3_file;	/* Filedata for COV #3	*/
	struct caFILEDATA   *data_temp;		/* Temporary for compare*/

	/* Used Function Name list buffers:  */
	struct caFUNCLIST   *used_funcs;	/* First element in list*/
	struct caFUNCLIST   *f_entry;		/* New element pointer  */

	char *lx_name = "CNTFILE";


	/* set up the buffers 			*/
	/* check to make sure user doesn't try to have the target 
	    name the same as either of the two source COVFILEs 
	    (because of problems in opening a file for write and read
	    simultaneously).
	*/

	/* open the two old COVFILEs and compare their headers */

	ret_code   = MRG_OK;
	over_flag  = FALSE;
	entry_flag = FALSE;		/* No object file entries yet! */
	null_arg   = FALSE;		/* Preset null_arg flag to 0   */

	obj1_buff  = (struct caOBJ_ENTRY *) malloc(sizeof(struct caOBJ_ENTRY));
	obj2_buff  = (struct caOBJ_ENTRY *) malloc(sizeof(struct caOBJ_ENTRY));

	used_funcs = NULL;		/* Set up the used function list  */


	data3_file = NULL;	       /* NULL out the target filedata   */
				       /* structure for safety.		 */


	if ((strcmp(cov1,cov3) == 0) || (strcmp(cov2,cov3) == 0)) {
	    fprintf(stderr,"\nFATAL ERROR: '%s'\n", cov3);
	    fprintf(stderr,"   Target %s is the same as a source %s.\n", lx_name, lx_name);
	    fprintf(stderr,"   No merge performed.\n");
	    ret_code = MRG_FA3;
	}

	data1_file = _CAopen_covf(cov1);

	if (data1_file == NULL) {
	    fprintf(stderr,"\nERROR: '%s'\n", cov1);
	    fprintf(stderr,"   Unable to find %s\n", lx_name);
	    ret_code = MRG_FA1;
	}

	data2_file = _CAopen_covf(cov2);

	if (data2_file == NULL) {
	    fprintf(stderr,"\nERROR: '%s'\n", cov2);
	    fprintf(stderr,"   Unable to find %s\n", lx_name);
	    if (ret_code == MRG_OK)
		_CAclose_covf(data1_file);
	    ret_code = MRG_FA2;
	}

	data_temp  = data1_file;
	i = 1;

	while ((ret_code == MRG_OK) && (i < 3))
           {
	     head_code=_CAhead_compare(data_temp,_com_flag);

	     switch  (head_code) {

	          case BADMASK:  fprintf(stderr,"\nERROR: '%s', '%s'\n",cov1,cov2);
			         fprintf(stderr,"   Bad %s entry.\n", lx_name);
				 fprintf(stderr,"   Header failure.\n");
				 if (i == 1)
				       ret_code = MRG_FA1;
				 else  ret_code = MRG_FA2;
				 _CAclose_covf(data1_file);
				 _CAclose_covf(data2_file);
			         break;
	          case BADCOMP:  fprintf(stderr,"\nERROR: '%s', '%s'\n",cov1,cov2);
			         fprintf(stderr,"   COMP Flag fail; no override\n");
				 if (i == 1)
                  		       ret_code = MRG_FA1;
				 else  ret_code = MRG_FA2;
				 _CAclose_covf(data1_file);
				 _CAclose_covf(data2_file);
			         break;
	          case OVRCOMP:  fprintf(stderr,"\nWARNING: '%s', '%s'\n",cov1,cov2);
			         fprintf(stderr,"   COMP Flag fail; override specified\n");
			         over_flag = TRUE;
			         break;
	          case OK:       break;
	    
	          /* DEBUG    DEBUG    DEBUG */

	          default:	 CAerror(COV350);

	          /*    END        DEBUG     */

	        }
	     data_temp = data2_file;
	     i++;
	    }

	    /* look at first object file entry in COV #1
	       and try to find its match in COV #2. */

	    if (ret_code == MRG_OK)
		{   obj_fcount = 0;

		    obj_code = OK;
		    if (fread((char *) obj1_buff, sizeof(struct caOBJ_ENTRY),1,data1_file->cov_obj_ptr) == 0)
			obj_code = NOT_OK;
		    if (fread((char *) obj2_buff, sizeof(struct caOBJ_ENTRY),1,data2_file->cov_obj_ptr) == 0)
			obj_code = NOT_OK;


		        if (obj_code != OK)
		    	    { switch (obj_code) {
			    
			        case NOT_OK:   fprintf(stderr,"\nERROR: '%s'\n",cov1);
					       fprintf(stderr,"   Unable to access object file data.\n");
					       fprintf(stderr,"   No merge performed.\n");
					       ret_code = MRG_FA1;
					       _CAclose_covf(data1_file);
					       _CAclose_covf(data2_file);
					       break;

			        /* DEBUG   DEBUG   DEBUG */

			        default:       CAerror(COV351);
					       ret_code = MRG_FA1;
					       _CAclose_covf(data1_file);
					       _CAclose_covf(data2_file);

			        /*      END    DEBUG     */
                             
                               }     /* End switch */
			
                             }       /*   End if   */


	                /* compare object file entries of the pair found to
	                   assure they are compatible. */

	                if ((ret_code == MRG_OK) && (null_arg == FALSE))
	    	            { switch (obj_code = _CAobj_compare(obj1_buff,obj2_buff,_tso_flag))
		              {
		                case OVER: over_flag = TRUE;
					   fprintf(stderr,"\nWARNING: '%s', '%s'\n",obj1_buff->name,obj2_buff->name);
					   fprintf(stderr,"   Object file entry time stamps don't match.\n");
					   fprintf(stderr,"   Override assumed.\n");
		                case C_OK: ret_code = MRG_OK;
					   entry_flag = TRUE;
			                   break;
				case NOVR: null_arg = TRUE;
					   entry_flag = FALSE;
					   fprintf(stderr,"\nERROR: '%s', '%s'\n",obj1_buff->name,obj2_buff->name);
					   fprintf(stderr,"   Object file entry time stamps don't match.\n");
					   fprintf(stderr,"   No override specified.\n");
					   break;
		                case NULL: null_arg = TRUE;
					   entry_flag = FALSE;
			                   fprintf(stderr,"\nERROR: '%s', '%s'\n",obj1_buff->name,obj2_buff->name);
			                   fprintf(stderr,"   Object file entry names & timestamps don't match.\n");
			                   break;
	                        case NAME: fprintf(stderr,"\nWARNING: '%s', '%s'\n",obj1_buff->name,obj2_buff->name);
			                   fprintf(stderr,"   Object file names don't match,time stamps do.\n");
					   entry_flag = TRUE;
                                           ret_code = MRG_OK;
			                   break;

		                /*    DEBUG    DEBUG    */

		                default:   CAerror(COV352);
					   ret_code = MRG_FA1;
					   _CAclose_covf(data1_file);
					   _CAclose_covf(data2_file);
			                   break;
                   
		                /*      END   DEBUG     */

		               }

		             }	/* End if MRG_OK */

		        /* Now begin actual merge of the two COVFILEs.
			   Look at each function entry and search for a match
			   in COV #2.
			   If none is found, use NULL second argument in the
			   ORing function.
			   When finished with COV #1 fuctions,
		           look at the COV #2 entries and if any have not been
			   used in the first pass, the data for their functions
			   is placed in the new COVFILE. */

			/* If the header comparison was OK,  go on and create the
			    target COVFILE and its data file buffer.  */

			if ((ret_code == MRG_OK) && (entry_flag == TRUE))
			{
			    if (data3_file == NULL) 	   /* If we haven't made a new COVFILE yet,*/
							   /* do so now!			   */
			    {
			        data3_file=_CAcreate_covf(cov3);
			        if (data3_file == NULL)	    /* Was the COVFILE created?  */
			        {   fprintf(stderr,"\nERROR: '%s'\n",cov3);
				    fprintf(stderr,"   Unable to create COVFILE\n");
    				    fprintf(stderr,"   FATAL Error\n");
				    ret_code = MRG_FA3;
				    _CAclose_covf(data1_file);
				    _CAclose_covf(data2_file);
			         }
			     }
			 }


			/* Allocate space for the data buffers now 	*/

			dbuff_1 = (struct caCOV_DATA *) malloc(sizeof(struct caCOV_DATA));
			dbuff_2 = (struct caCOV_DATA *) malloc(sizeof(struct caCOV_DATA));
			dbuff_3 = (struct caCOV_DATA *) malloc(sizeof(struct caCOV_DATA));

		        oused_flag = FALSE;
			/* for each function in COVFILE 1 */
		        while (((obj_code=_CAread(data1_file,dbuff_1)) != EOD_FAIL) &&
			       ((ret_code == MRG_OK) || (ret_code == MRG_fail)) &&
				(entry_flag == TRUE))
			  {  
			     null2_arg = FALSE;
			     if (obj_code == EOF_FAIL)
			 	{ if (!_com_flag)
				      { fprintf(stderr,"\nERROR: '%s', '%s'\n",cov1,obj1_buff->name);
					fprintf(stderr,"   Unexpected EOF encountered.\n");
					fprintf(stderr,"   No merge performed for this object file.\n");
					ret_code = MRG_fail;
				       }
				  else
				      { fprintf(stderr,"\nWARNING: '%s', '%s'\n",cov1,dbuff_1->func_name);
					fprintf(stderr,"   Incomplete %s entry, override assumed.\n", lx_name);
					over_flag = TRUE;
				       }
				 }


			     if (ret_code != MRG_fail)
				{ if (null_arg != TRUE)
				      { obj_code = _CAfind(data2_file,(char *)dbuff_1->func_name);
					if (obj_code != OK)
					    {  fprintf(stderr,"\nWARNING: '%s', '%s'\n",cov2,dbuff_1->func_name);
					       fprintf(stderr,"   Function not found in object file: '%s'\n",obj2_buff->name);
					       null2_arg = TRUE;
					     }
					else { obj_code=_CAread(data2_file,dbuff_2);
					       if (obj_code != OK)
						   { fprintf(stderr,"\nERROR: '%s', '%s'\n",cov2,dbuff_1->func_name);
						     fprintf(stderr,"   Unable to merge data.\n");
						     fprintf(stderr,"   Data copied from '%s'\n", cov1);
						     null2_arg = TRUE;
						    }
					      }
				       }

				  /* If we do not have a NULL second arg., 
				       merge the 2 sets of data.  */
				  
				  if ((null_arg != TRUE) && (null2_arg != TRUE))
				      {
					obj_code = _CAadd(dbuff_1,dbuff_2,dbuff_3,over_flag);
					if (obj_code != OK)
	   /* DEBUG */			    { CAerror(COV353);
					      ret_code = MRG_fail;
					     }

				        /* don't need dbuff_2 anymore so */
				        /* free up used space in dbuff_2 */
					(void) free((char *)dbuff_2->func_name);
					(void) free((char *)dbuff_2->lca_counts);
				       }

				  /* Otherwise, fill the target buffer with
				      the data from dbuff_1 (NULL 2nd arg.) */

				  else {
					 dbuff_3->fname_size = dbuff_1->fname_size;
					 dbuff_3->func_name = (unsigned char *) malloc(dbuff_3->fname_size+1);
					 (void)strcpy((char*)dbuff_3->func_name,(char*)dbuff_1->func_name);
					 dbuff_3->lca_words = dbuff_1->lca_words;
					 dbuff_3->lca_counts = (caCOVWORD *) malloc(dbuff_3->lca_words*(sizeof(caCOVWORD)));

					 for (i=0; i<(unsigned int)dbuff_1->lca_words; i++)
					 {
     					    dbuff_3->lca_counts[i] = dbuff_1->lca_counts[i];
					 }
				  }


				  /* If we haven't had a MRG_fail occur yet make an
				      entry of the data in the target COVFILE
				      datafile.  If this is the first function from
				      the current object file entry, create an
				      object file entry in the target datafile and
				      add 1 to the count of object files used.  */

				  if (ret_code == MRG_OK)
				      { if (oused_flag != TRUE)
					    { oused_flag = TRUE;
					      obj_code = _CAobj_merge(data3_file,obj1_buff, obj2_buff);
					      if (obj_code != OK)
					      {    ret_code = MRG_fail;
						   fprintf(stderr,"\nERROR: '%s', '%s'\n",cov1,obj1_buff->name);
						   fprintf(stderr,"   Unable to create new object file entry.\n");
					       }
					      else obj_fcount++;
					     }

				        /* Next, make the data entry into the
					    filedata structure.  If the Time Stamp
					    override has been selected, maintain
				            a list of the functions used in the
					    merge and add this one to it.  */

					if (ret_code != MRG_fail)
					{
				             obj_code = _CAdata_entry (data3_file,dbuff_3);
				             if (obj_code != OK)
				                 { ret_code = MRG_fail;
					           fprintf(stderr,"\nERROR: '%s', '%s'\n",cov1,dbuff_3->func_name);
				     	           fprintf(stderr,"   Unable to create new function entry.\n");
				       	          }

				             if (_tso_flag == TRUE)     /* Might there be new functions? */
				                 { f_entry = (struct caFUNCLIST *) malloc(sizeof(struct caFUNCLIST));
					           obj_code = _CAadd_flist(&used_funcs,f_entry,(char *)dbuff_1->func_name);
					           if (obj_code != OK)
					               { CAerror(COV355);
					                }
				                  }
						  /* now free dbuff_3 entries */
						  (void)free((char *)dbuff_3->func_name);
						  /* and dbuff_1 */
						  (void)free((char *)dbuff_1->func_name);
						  (void)free((char *)dbuff_3->lca_counts);
						  (void)free((char *)dbuff_1->lca_counts);
					 }		/* End if ret_code = MRG_fail */


				       }        	/* End if ret_code = MRG_OK */

				 }			/* End if ret_code = fail  */

			   }				/* End while _CAread */


		        /* Free up the used data buffers now. */
		        free((char *) dbuff_1);
		        free((char *) dbuff_2);

			/* Now, we are out of the search loop for the functions
			    in COV #1's obj. file entry.  Next, if Time Stamp   
			    override is selected, we'll search the second COVFILE
			    obj file entry for additional functions, by checking
			    the names against the list made in the above loop. */

			if ((_tso_flag == TRUE) && (null_arg == FALSE)) {
			    _CArewind(data2_file);
			    while ((obj_code = _CAread(data2_file,dbuff_3)) == OK) {
				if ((obj_code = _CAf_search((char *)dbuff_3->func_name,used_funcs)) != TRUE) {
				    if (oused_flag != TRUE) {
					oused_flag = TRUE;
					obj_code = _CAobj_merge(data3_file,obj1_buff,obj2_buff);
				        if (obj_code != OK) {
					    fprintf(stderr,"\nERROR: '%s', '%s'\n",cov2,obj2_buff->name);
					    fprintf(stderr,"   Unable to create new object file entry.\n");
					}
				        else obj_fcount++;
				    }
				    fprintf(stderr,"\nWARNING: '%s', '%s'\n",cov1,dbuff_3->func_name);
				    fprintf(stderr,"   Function not found in object file: '%s'\n",obj1_buff->name);
				    obj_code = _CAdata_entry(data3_file,dbuff_3);
				    if (obj_code != OK) {
					fprintf(stderr,"\nERROR: '%s', '%s'\n",cov2,dbuff_3->func_name);
					fprintf(stderr,"   Unable to create new function entry.\n");
				    }
				}
			    }	/* End while _CAread */
			}			/* End if _tso_flag TRUE */

			/* Free up the object file entry buffers here, before
			    going through the loop again.  Also reset the internal
			    flags to the FALSE state. */
			free((char *) dbuff_3);
			free((char *) obj1_buff);
			free((char *) obj2_buff);

			oused_flag = FALSE;
			null_arg = FALSE;

			/* reset the ret_code for the next pass */

			if ((ret_code != MRG_OK) && (ret_code != MRG_FA1) &&
			   (ret_code != MRG_FA2) && (ret_code != MRG_FA3))
			     if (ret_code == MRG_fail)
				 ret_code =  MRG_OK;
			     else CAerror(COV360);
		     

		  /* process is complete. Close up the open COVFILEs */

	         }   /* End if ret_code before checking if objs match */


	     if ((ret_code != MRG_FA1) && (ret_code != MRG_FA2) && (ret_code != MRG_FA3) &&
		 (data3_file != NULL)) {
		if ((obj_code = _CAcomp_covf(data3_file,obj_fcount)) != OK)
		    CAerror(COV357);
		_CAclose_covf(data3_file); 
	     }


	if (ret_code == MRG_OK)
	{
	     _CAclose_covf(data1_file); 
	     _CAclose_covf(data2_file); 

	     if (obj_fcount == 0)
		   ret_code = MRG_FA1;

	     if ((obj_code = _CAfree_list(used_funcs)) != OK)
	           CAerror(COV359);
	}

	return(ret_code);

}	/* END OF ROUTINE */


_CAhead_compare(data,flag)
struct 	caFILEDATA *data;
int	flag;
{

	short ret_code, value;
     	short temp;
	short _CAvalid_test();

	ret_code = OK;
	temp = FALSE;
 
	value = _CAvalid_test(data); 

	if ((value & MACH_MASK) || (value & VER_MASK) || (value == BUG_FAIL))
	       ret_code = BADMASK;

	else {
	    if (value & EDIT_MASK)
		temp = TRUE;
	    if (value & COMP_MASK)
		if (flag == TRUE)
		    if (temp == TRUE)
			 ret_code = OVREDIT;
		    else ret_code = OVRCOMP;
		   
		else ret_code = BADCOMP;

	    else if (temp == TRUE)
		ret_code = EDITFLG;
	    else ret_code = OK;
	}

	return(ret_code);
}


_CAobj_compare(obj1,obj2,flag)
struct caOBJ_ENTRY *obj1, *obj2;
short  flag;
{
/* check Time Stamps, Magic Numbers, Names of the two entries,
   using the 'flag' input in case there is a difference in time stamps. */

	int   strncmp();

	short ret_code;

	ret_code = C_OK;

	if (strncmp(obj1->magic_no,obj2->magic_no,sizeof(obj2->magic_no)) != 0)
	     ret_code = NULL;

	else if (strncmp(obj1->name,obj2->name,ONAMESIZE) ==0)
	    if (obj1->time == obj2->time)
		ret_code = C_OK;
	    else if (flag == TRUE)
		ret_code = OVER;
	    else ret_code = NOVR;

	else if (obj1->time == obj2->time)
	    ret_code = NAME;
	else ret_code = NULL;


	return(ret_code);
}


_CAadd(data1,data2,target,flag)
struct caCOV_DATA *data1;
struct caCOV_DATA *data2;
struct caCOV_DATA *target;
short  flag;		/* TRUE -> allow different sized cov. structures */
{ 

/* _CAadd performs the word-by-word ADD on the
   data for the functions currently being merged.

   The basic idea is to fill out ``*target'' with
   data representing the sum of ``*data1'' and ``*data2''.

   It first allocates a duplicate of the fcn name,
   then allocates an array large enough for the bigger
   of the two given, then puts the sum of the (common)
   entries in the newly allocated array.  Finally it
   copies the line numbers from the larger array.

   Note well that the two arrays very well OUGHT
   to be the same size!!  but we're allowing for
   functions that are out of sync but in a (hopefully)
   harmless way (e.g. code added to the end of some
   functions, so that the pre-existing basic blocks
   still exist in the newer executable, and correspond
   to those in the functions of the older executable).
*/

/* * * * * *
 * Note Well: rjp Dec-27-1988
 * 
 * with C Issue 5.0, the coverage array (i.e. an array of 1perbblk counts)
 * became the coverage Structure, with three elements:
 * 	a caCOVWORD -> the number of bblks for this structure (NBBLK)
 * 	an NBBLK-element array of caCOVWORD, holding execution cts as of old 
 * 	an NBBLK-element array of caCOVWORD, holding bblk starting src linenos
 */

	char	*strncpy();
	char	*malloc();
	void	free();
	short	ret_code;

	caCOVWORD commonnumof_bblks;
	caCOVWORD   truenumof_bblks;

	ret_code = OK;

	/* * * * * *
	 * (both names are presumed to be the same.)
	 */
	target->fname_size = data1->fname_size;
	target->func_name = (unsigned char *)malloc(target->fname_size+1);
	strcpy((char *)target->func_name, (char *)data1->func_name);
	/* is null-terminated */

	/* determine the size of the new array to be formed (target).  */

	if (flag == FALSE)
	{
		if (data1->lca_bblks != data2->lca_bblks) { 
			ret_code = NOT_OK;
			free((char *)target->func_name);
		}
	}

   if (ret_code == OK)
   {
	/* * * * * *
	 * goal here is to fill the three components of
	 * the target coverage structure:
	 * 1 - the number of bblks in the target (lca_bblks, or N)
	 * 2 - the merged counts (first N entries in lca_counts)
	 * 3 - the line numbers (subsequent N entries in lca_counts)
	 * 
	 * since the numbers of bblks in the input cov structures
	 * may not be equal, we'll try not to discard data, and
	 * use the bblk count and the line number array from
	 * the larger cov structure.
	 * 
	 * (NB you can get weird results if the smaller structure is
	 * the ignored owner of the ``correct'' line numbers.. i.e.
	 * if they DELETE code then run, and try to merge the data
	 * derived in with that from a prior run.. but seriously,
	 * that implies that the object files are Significantly different,
	 * and that the person demanding this merge is indeed asking for 
	 * a strange combined result... rjp)
	 * 
	 * BASIC ALGM:
	 * allocate size large enough for bigger of two cov structs;
	 * set the word, bblk counts for the larger cov struct;
	 * add the smaller ct list with the larger, into the target;
	 * copy the remainder of the longer list, to the target; and
	 * copy the line number list to the target, from the larger struct.
	 */
	/* Net result: common# added&stored, true#-common#+true# copied. */

	struct caCOV_DATA *src;
	caCOVWORD i;
debug(	char buf1[128];)


	/* * * * * *
	 * determine which will be the ``source'' structure:
	 * i.e. which one is larger.  when we need to copy
	 * the ``extra data'' (i.e. stuff in one but not the other),
	 * this pointer tells us From Which to copy stuff.
	 */

	if (data1->lca_bblks > data2->lca_bblks)
	{
		src = data1;
		commonnumof_bblks = data2->lca_bblks;
		truenumof_bblks = data1->lca_bblks;
	} else {
		src = data2;
		commonnumof_bblks = data1->lca_bblks;
		truenumof_bblks = data2->lca_bblks;
	}
	target->lca_bblks = truenumof_bblks;


debug(	sprintf(buf1,"overlapping bblks==%d, larger size==%d\n", 
		commonnumof_bblks, truenumof_bblks);
     )
debugp2("in _CAadd\n",buf1)

	/* set counts. */
	target->lca_words = truenumof_bblks * 2;
	target->lca_bblks = truenumof_bblks;

	/* Allocate & set ptrs. */
	target->lca_counts =
		(caCOVWORD *) malloc(
			(truenumof_bblks*2)*(sizeof(caCOVWORD))
		);

	target->lca_lineos = target->lca_counts + target->lca_bblks ; 


	/* Add (common) basic blk execution counts, copy others. */
	/* Also, copy (longer) list of line numbers, while yer at it. */

	for ( i=0 ; i < commonnumof_bblks ; i++ )
	    target->lca_counts[i] = data1->lca_counts[i] + data2->lca_counts[i]; 


	for ( i=commonnumof_bblks ; i < truenumof_bblks*2 ; i++ )
		target->lca_counts[i] = src->lca_counts[i];

debug(	{
		caCOVWORD i = truenumof_bblks * 2;

		fprintf(stderr,
			"\nDump of Summed Coverage Structure:\n");
		fprintf(stderr,
			"  ``lca_words'' in structure: %lu\n",
			target->lca_words);
		fprintf(stderr,
			"  ``lca_bblks'' in structure: %lu\n",
			target->lca_bblks);
		fprintf(stderr,
			"  Type\t\tCountORLine\n");
		for ( i=0; i< truenumof_bblks*2; i++) {
			fprintf(stderr,"  %s\t\t%d\n",
				(i<truenumof_bblks?"Xcount":"Line"),
				target->lca_counts[i] );
		}
	}
     )

    }

	    return(ret_code);
}
