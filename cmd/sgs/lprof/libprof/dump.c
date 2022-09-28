/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libprof:dump.c	1.12"

#include "hidelibc.h"		/* uses "_" to hide libc functions */
#include "hidelibelf.h"		/* uses "_" to hide libelf functions */

#include <stdio.h>
#include <varargs.h>
#include <fcntl.h>
#include <string.h>

#include "symint.h"
#include <syms.h>
#include <sys/stat.h>
#include <signal.h>

#include "covfile.h"
#include "filedata.h"
#include "retcode.h"
#include "profopt.h"
#include "debug.h"

#define CANAME "__coverage."
#define LINELEN 256
#define	XSUFFIX	".cov"
#define	LSUFFIX	".cnt"


static char _optbuf[LINELEN];
static char *_opts;
static char *_tempnew, *_tempmrg;
static int _sigflg;

static _profopterr(), _readopt(), _copyfile();

/* 'signal' handler. 
 *
 */
static
void _dexit(sig)
int sig;	/* signal# why we woke up - ignored */
{
    _sigflg = 1;
    unlink(_tempnew);
    unlink(_tempmrg);
}


/*
*	The object of this routine is to dump the coverage structure
*	to the coverage file.
*
*	- ?
*	- Create coverage file.
*	- For each function (filter functions from profiler symbol array);
*		- Save the function name into the coverage data.  (The
*		function name is NOT null terminated.)
*		- Scan up to the coverage structure entry or, if a function
*		is reached, break back to the main loop.  Save the size
*		and location of the coverage structure and store the
*		information from the structure into the coverage file.
*	- ?
*/
static char *str_openfail = "_symintOpen on failed for '%s'\n";
static char *str_crtfail =
	"*** Unable to create a file to store profiling data.\n";
static char *str_updated = "\tCNTFILE `%s' updated\n";
static char *str_mergefail = "*** Unable to merge results.\n";
static char *str_created = "\tCNTFILE `%s' created\n";
static char *str_writefail = "*** Unable to put profiling data in `%s'.\n";
static char *str_nodata =
	"*** No profiling data retrievable from process `%s'\n";
static char *str_nomemory = "_CApfind: out of memory\n";
static char *str_retrieve =
	"Data from this execution can be retrieved from `%s'.\n";

/*
*	WFM 1/16/89
*	Base address for shared object.
*/
static TYPE_BASEAD	baseaddr;

/*
*	Warning: _CAdump takes one to three arguments as follows;
*
*		arg_count	- number of arguments to follow (0, 1, or 2)
*		procname	- path name of this process (shared object)
*		opt_p		- pointer to options structure
*
*	See also "va_start" below.
*/
_CAdump (va_alist)
va_dcl
{
    va_list ap;
    int	arg_count;

    char *malloc();
    void free();

    PROF_FILE *objfile;

    struct caCOV_DATA covdata;
    struct caFILEDATA *filedata;

    struct caFILEDATA *_CAcreate_covf();

    PROF_SYMBOL *sym_p;
    PROF_SYMBOL *lastsym_p;

    struct options opt;
    struct options *opt_p;
    char *sym_name;
    char *procname;
    static char *_CApfind();
    short covflag;

    extern char *_CAproc;		/* name of this UNIX process */

#ifdef DUMP_DEBUG
    caCOVWORD size;
    caCOVWORD *lcaptr;
#endif

#ifdef __STDC__
    void (*oldi)(int), (*oldh)(int), (*oldg)(int);
#else
    void (*oldi)(), (*oldh)(), (*oldg)();
#endif

    /* read in options */
    opt_p = &opt;
    _CAgetprofopts(opt_p);
    if (opt_p->noprofile == TRUE) {
	goto end_fail;
    }

    /* catch signals to make sure temp files deleted */
    _sigflg = 0;
    oldi = signal(SIGINT, _dexit);
    oldh = signal(SIGHUP, _dexit);
    oldg = signal(SIGQUIT, _dexit);

    /*
    *	WFM 1/16/89
    *	If there are no arguments (the first argument is zero)
    *	then we run the default procedure.  This enhancement allows
    *	dump to be called for shared objects by passing in the procname
    *	and the option structure.
    */
    baseaddr = 0;
    va_start(ap);
    arg_count = va_arg(ap, int);
    if (arg_count > 0) {
	procname = va_arg(ap, char *);
	if (!(baseaddr = _CAget_base_address(procname))) {
	    _err_exit("Cannot find base address for %s.",procname);
	}
	DEBUG(printf("Base address for %s is 0x%x.\n",procname,baseaddr));
	if (arg_count > 1) {
	    opt_p = va_arg(ap, struct options *);
	}
    } else {
	/* get name of this process */
	if (strchr(_CAproc, '/') == NULL)
	    procname = _CApfind(_CAproc);
	else {
	    procname = _CAproc;
	}
    }
    va_end(ap);

    /* make initializations */
    covflag = FALSE;

    if ((objfile = _symintOpen(procname)) == NULL) {
	fprintf(stderr, str_openfail, procname);
	goto end_cleanup;
    }
    if (elf_kind(objfile->pf_elf_p) == ELF_K_COFF) {
	    fprintf(stderr,
	    "libprof: %s: Warning - internal conversion of COFF file to ELF\n",
	    procname
	    );
    }

    /* create temporary file */
    _tempnew = tempnam(NULL, "lxp");
    DEBUG(printf("dump: temp file name (_tempnew) = %s\n", 
	(_tempnew ? _tempnew : "<null>")));

    /* create COVFILE, and add entry for this object file */

    /*
    *	The second parameter to _CAobj_entry used to be "procname".
    *	See _CAobj_entry in new.c for a explanation of the change.
    *	"procname" was retained as the third parameter to give
    *	_CAobj_entry easy access to the file name.
    *
    *	WFM 12/20/88
    */
    if (
	((filedata = _CAcreate_covf(_tempnew)) == NULL)
    ||  (_CAobj_entry(filedata, objfile, procname) != OK)
    ) {
	fprintf (stderr, str_crtfail);
	if (_tempnew != NULL)
	    free(_tempnew);
	goto end_cleanup;
    }

#ifdef DUMP_DEBUG
	printf("number of symbols = %d\n\n", objfile->pf_nsyms);
	printf("func\tfunc\tarray\tarray\tcoverage\n");
	printf("name\tindex\taddr\tsize\tarray\n\n");
#endif

	sym_p = objfile->pf_symarr_p;
	lastsym_p = &sym_p[objfile->pf_nsyms - 1];
	DEBUG_LOC("debug: top of for-loop to consider symbols");
	for (; sym_p <= lastsym_p; sym_p++) {
		DEBUG(printf("symbol - name = %.8s", SYMBOL_NAME(sym_p)));
		DEBUG(printf(", tag = %d\n", sym_p->ps_dbg.pd_symtag));
		DEBUG(printf(", number = %ld\n", sym_p - objfile->pf_symarr_p));
		if (_sigflg == 1) {
			goto end_return;
		}
		if (!SYMBOL_IS_FUNC(sym_p)) {
			continue;
		}

		DEBUG(printf("symbol is a function\n"));

		sym_name = sym_p->ps_dbg.pd_name;
		covdata.fname_size = strlen(sym_name);
		covdata.func_name
			= (unsigned char *)malloc(covdata.fname_size+1);
		strncpy((char *)covdata.func_name,sym_name,covdata.fname_size);
		covdata.func_name[covdata.fname_size] = '\0';

		while (++sym_p <= lastsym_p) {
			if (SYMBOL_IS_FUNC(sym_p)) {
				sym_p--;
				break;
			}
			DEBUG(printf("inside while loop: name = %s\n",
				sym_p->ps_dbg.pd_name));
			if (
				strncmp(
					CANAME,
					sym_p->ps_dbg.pd_name,
					sizeof(CANAME)-1
				) != 0
			) {
				continue;
			}
			DEBUG(printf("_CAdump: Size of coverage array = %d\n",
				sym_p->ps_sym.st_size
			));
			DEBUG(printf("First value in coverage array is %d\n",
				*((caCOVWORD *)
				(((char *) sym_p->ps_sym.st_value) + baseaddr)
				)
			));
			/*
			*	Point past the first entry in the coverage
			*	structure (see next comment).
			*/
			covdata.lca_counts =
				((caCOVWORD *)
				(((char *) sym_p->ps_sym.st_value) + baseaddr)
				) + 1;
			/*
			*	The first entry in the coverage structure
			*	is the number of entries in each of the
			*	two arrays that follow.  If we multiply this
			*	number by two, we have the total number of
			*	entries in the array (lca_words).
			*/
			covdata.lca_words = *(
				(caCOVWORD *) 
				(((char *) sym_p->ps_sym.st_value) + baseaddr)
			);
			covdata.lca_words *= 2;

			DEBUG(printf("_CAdump: calling _CAdata_entry\n"));
			DEBUG(dump_filedata(filedata));
			DEBUG(dump_covdata(&covdata));
			if (_CAdata_entry(filedata,&covdata) != OK) {
				_err_warn("***unable to write profiling data\n");
				goto complete;
			}
			DEBUG(printf("_CAdump: back from _CAdata_entry\n"));
			covflag = TRUE;
#ifdef DUMP_DEBUG
			size = covdata.lca_words;
			lcaptr = covdata.lca_counts;
			printf ("\t%ld",
				(((char *) sym_p->ps_sym.st_value) + baseaddr)
			);
			printf("\t%d",size);
			while (size--) 
				printf("\t%cd\n", *lcaptr++);
			printf("\n");
#endif
		}
		free(covdata.func_name);
	}

complete:;

    /* complete covfile */
    DEBUG(printf("_CAdump: calling _CAcomp_covf\n"));
    _CAcomp_covf(filedata,1);
    free(filedata);
    if (_sigflg == 1)
	goto end_return;

    DEBUG(printf("_CAdump: returned from _CAcomp_covf\n"));

    /* was any coverage data dumped? */
    if (!covflag) {
	/* if covflag */
	fprintf(stderr, str_nodata, procname);
	unlink(_tempnew);
	goto end_cleanup;
    }

    _CAproc_opts(_tempnew, procname, opt_p);
    if (_sigflg == 1)
	goto end_return;

end_cleanup:;

    (void) signal(SIGINT, oldi);
    (void) signal(SIGHUP, oldh);
    (void) signal(SIGQUIT, oldg);

    _symintClose(objfile);
    return;

end_return:;
    return;

end_fail:;
    return(0);
}


/*
*	_CAproc_opts	- Process given procname according to given options.
*
*	Callers: _CAdump (above), routine(s) in soqueue.c
*/
_CAproc_opts(cntpath, procname, opt_p)
char *cntpath;
char *procname;
struct options *opt_p;
{
	char covfile[LINELEN];
	char *stringptr;
	int fildes;
	extern int _tso_flag;		/* time stamp flag */
	extern int _com_flag;		/* complete override flag */


	DEBUG(printf("_CAproc_opts: cntpath = '%s'\n", cntpath));
	strcpy(covfile, (opt_p->dir ? opt_p->dir : ""));
	if (opt_p->pid == TRUE) {
	    int pid, n;

	    /* cfile points to end of covfile name */
	    char *cfile = strrchr(covfile, '\0');
	    if ((pid = getpid()) <= 0) /* just in case getpid */
		pid = 1; 	/* returns something inappropriate */
	    for (n = 10000; n > pid; n /=10)
		;  /* suppress leading zeros */
	    for ( ; ; n /=10) {
		*cfile++ = pid/n + '0';
		if (n == 1)
		    break;
		pid %= n;
	    }
	    *cfile++ = '.';
	    *cfile = '\0';
	}

	if (opt_p->file != NULL) {
	    strcat(covfile, opt_p->file);
	} else {
	    /* use process name in the resultant COVFILE name */
	    if ((stringptr = strrchr(procname,'/')) == NULL)
		strcat(covfile,procname);   /* not a full path name */
	    else
		strcat(covfile,++stringptr);  /* ignore path name */
	    strcat(covfile,LSUFFIX);   /* add suffix */
	}
	if (_sigflg == 1)
	    return;

	if ((opt_p->merge == TRUE) && (access(covfile,0) == 0)) {
	    _tso_flag = FALSE;
	    _com_flag = FALSE;
	    _tempmrg = tempnam(NULL, "lxp");
	    DEBUG(printf("_CAdump: calling _CAcov_join\n"));
	    if(_CAcov_join(covfile,cntpath,_tempmrg) == MRG_OK) {
		_copyfile(_tempmrg,covfile);
		if (opt_p->msg == TRUE) {
			fprintf(stderr, str_updated, covfile);
		}
		unlink(cntpath);
	    } else {
		fprintf(stderr, str_mergefail);
		fprintf(stderr, str_retrieve, cntpath);
	    }
	} else {
	    /* copy results to destination file */
	    /* create destination file */
	    fildes = open(covfile,O_CREAT,0644);
	    close(fildes);
	    /* copy results */
	    DEBUG(printf("Before _copyfile, cntpath = '%s'\n", cntpath));
	    if (_copyfile(cntpath, covfile) == 0) {
		if (opt_p->msg == TRUE) {
			fprintf(stderr, str_created, covfile);
		}
	    } else {
		fprintf(stderr, str_writefail, covfile);
		fprintf(stderr, str_retrieve, cntpath);
	    }
	}
}


#ifdef ddt
dump_filedata(fd_p)
struct caFILEDATA *fd_p;
{
	printf("Coverage Array File Data (caFILEDATA)\n");
	printf("  object stream (cov_obj_ptr) = 0x%lx\n", fd_p->cov_obj_ptr);
	printf("  data stream (cov_data_ptr) = 0x%lx\n", fd_p->cov_data_ptr);
	printf("  number of obj files (obj_cnt) = %d\n", fd_p->obj_cnt);
	printf("  usage flag (use_flag) = %s\n",
		(fd_p->use_flag == CREATE ? "CREATE" : "UPDATE")
	);
}

dump_covdata(cd_p)
struct caCOV_DATA *cd_p;
{
	caCOVWORD	num;
	caCOVWORD	*count_p;
	caCOVWORD	*line_p;
	int		i;

	num = cd_p->lca_words / 2;
	count_p = (caCOVWORD *) cd_p->lca_counts;
	line_p = count_p + num;

	printf("Coverage Array Coverage Data (caCOV_DATA):\n");
	printf("  length of name (fname_size) = %d\n",cd_p->fname_size);
	printf("  name (func_name) = %s\n",cd_p->func_name);
	printf("  words in coverage array (lca_words) = %d\n",
						cd_p->lca_words);
	printf("  pointer to array (lca_counts) = 0x%x\n", cd_p->lca_counts);
	printf("Coverage Array Contents: (%d entries)\n", num);
	printf("\tLINE\t\tCOUNT\n");
	for (i = 0; i < num; i++) {
		printf("\t%4d\t\t%4d\n", line_p[i], count_p[i]);
	}
	printf("\n");

}
#endif



#define MAXNAME		121
#define PATH		"PATH"

static
char *
_CApfind(filename)
char *filename;
{
	char namebuf[MAXNAME];
	char *fullname;
	struct stat statbuf;
	int result, n;
	char *p, *path;
	extern int getpth();
	extern char *getenv();
	extern char *malloc();
	extern int stat();

	fullname = NULL;
	if (filename == NULL) return(NULL);
	if (filename[0] == '/')
		fullname = filename;
	else if ((path = getenv(PATH)) != NULL) {
		do {
			p = path;
			n = 0;
			while (*p && *p++ != ':') n++;
			if (n > 0) {
				strncpy(namebuf, path, n);
				namebuf[n] = '\0';
				strcat(namebuf, "/");
			} else
				strcpy(namebuf, "\0");
			strcat(namebuf, filename);
			result = stat(namebuf, &statbuf);
			if (result != -1) {
			    /* make sure it is executable,
				but not a directory */
			    result = access(namebuf, 01) ||
				(statbuf.st_mode & (0040000));
			}
		} while ((result != 0) && *(path = p));
		if (result != -1)
			fullname = namebuf;
	}
	if (fullname != NULL) {
		if ((p = malloc(strlen(fullname)+1)) == NULL) {
			fprintf(stderr, str_nomemory);
			exit(1);
		} else
			strcpy(p, fullname);
		fullname = p;
	}
	return(fullname);
}




_CAgetprofopts(opt_p)
struct options *opt_p;
{
    extern char *getenv();
    extern char *malloc();

    opt_p->noprofile = FALSE;
    opt_p->merge = FALSE;
    opt_p->dir = NULL;
    opt_p->file = NULL;
    opt_p->msg = TRUE;
    opt_p->pid = FALSE;
    if ((_opts = getenv("PROFOPTS")) != NULL) {
	/* if PROFOPTS="", don't profile this run */
	if (strcmp(_opts, "") == 0) {
	    opt_p->noprofile = TRUE;
	}
	while (*_opts != '\0') {
	    while ((*_opts == ' ') || (*_opts == '	')) _opts++;
	    switch (*_opts) {
		case 'd':   if (strncmp(_opts, "dir=", 4) == 0) {
				_opts = _opts + 4;
				_readopt();
				opt_p->dir = (char *) malloc(strlen(_optbuf) + 2);
				strcpy(opt_p->dir, _optbuf);
				opt_p->dir[strlen(_optbuf)] = '/';
				opt_p->dir[strlen(_optbuf)+1] = '\0';
			    }
			    else {
				_profopterr();
			    }
			    break;
		case 'f':   if (strncmp(_opts, "file=", 5) == 0) {
				_opts = _opts + 5;
				_readopt();
				opt_p->file = (char *) malloc(strlen(_optbuf) + 1);
				strcpy(opt_p->file, _optbuf);
			    }
			    else {
				_profopterr();
			    }
			    break;
		case 'm':   if (strncmp(_opts, "merge=", 6) == 0) {
				_opts = _opts + 6;
				_readopt();
				if ((_optbuf[0] == 'y') || (_optbuf[0] == 'Y'))
				    opt_p->merge = TRUE;
				else if ((_optbuf[0] == 'n') || (_optbuf[0] == 'N'))
				    opt_p->merge = FALSE;
				else
				    _profopterr();
			    }
			    else if (strncmp(_opts, "msg=", 4) == 0) {
				_opts = _opts + 4;
				_readopt();
				if ((_optbuf[0] == 'y') || (_optbuf[0] == 'Y'))
				    opt_p->msg = TRUE;
				else if ((_optbuf[0] == 'n') || (_optbuf[0] == 'N'))
				    opt_p->msg = FALSE;
				else
				    _profopterr();
			    }
			    else
				_profopterr();
			    break;
		case 'p':   if (strncmp(_opts, "pid=", 4) == 0) {
				_opts = _opts + 4;
				_readopt();
				if ((_optbuf[0] == 'y') || (_optbuf[0] == 'Y'))
				    opt_p->pid = TRUE;
				else if ((_optbuf[0] == 'n') || (_optbuf[0] == 'N'))
				    opt_p->pid = FALSE;
				else
				    _profopterr();
			    }
			    else
				_profopterr();
			    break;
		default:    _profopterr();
	    } /* end switch */
	}
    }
}


static
_readopt()
{
    int i;

    i = 0;
    while (*_opts != '\0') {
	_optbuf[i++] = *_opts++;
	if (*_opts == ',') {
	    _opts++;
	    break;
	}
    }
    _optbuf[i] = '\0';
}

static
_profopterr()
{
    _readopt();
    fprintf(stderr, "unrecognized PROFOPTS option: %s\n", _optbuf);
}


static
_copyfile(tempfile, oldfile)
char *tempfile, *oldfile;
{
	register fi,fo,ln;
	struct	stat	st;
	char buf[BUFSIZ];

#ifdef __STDC__
    void (*oldi)(int), (*oldh)(int), (*oldg)(int);
#else
    void (*oldi)(), (*oldh)(), (*oldg)();
#endif

	oldi = signal(SIGINT, SIG_IGN);
	oldh = signal(SIGHUP, SIG_IGN);
	oldg = signal(SIGQUIT, SIG_IGN);

	if (stat(oldfile, &st) != 0)
		return(EOF);
	if (unlink(oldfile) != 0)
		return(EOF);
	if (link(tempfile, oldfile) != 0) {
		if ((fi = open(tempfile, 0)) < 0)
			return(EOF);
		if ((fo = creat(oldfile, 0644)) < 0)
			return(EOF);
		while ((ln = read(fi, buf, sizeof(buf))) > 0)
			if(write(fo, buf, ln) != ln)
				return(EOF);
		close(fi);
		close(fo);

	}
	if (chmod(oldfile, st.st_mode) != 0)
		return(EOF);
	if (chown(oldfile, st.st_uid, st.st_gid) != 0)
		return(EOF);
	if (unlink(tempfile) != 0)
		return(EOF);
	(void) signal(SIGINT, oldi);
	(void) signal(SIGHUP, oldh);
	(void) signal(SIGQUIT, oldg);
	return(0);
}

#ifdef DUMP_DEBUG
#include <math.h>

bitdump(byte)
unsigned char byte;
{	
	char *bitstring[16];
	short remain,quo;

	/** define 'backwards' bit strings **/
	bitstring[0] =  "0000";
	bitstring[1] =	"1000";
	bitstring[2] = 	"0100";
	bitstring[3] =	"1100";
	bitstring[4] =	"0010";
	bitstring[5] =	"1010";
	bitstring[6] =	"0110";
	bitstring[7] =	"1110";
	bitstring[8] =	"0001";
	bitstring[9] =	"1001";
	bitstring[10] =	"0101";
	bitstring[11] =	"1101";
	bitstring[12] =	"0011";
	bitstring[13] =	"1011";
	bitstring[14] =	"0111";
	bitstring[15] =	"1111";


	remain = (short) fmod((double) byte,(double) 16);
	quo = byte/16;
	printf("%s ",bitstring[remain]);
	printf("%s",bitstring[quo]);
 }
#endif
