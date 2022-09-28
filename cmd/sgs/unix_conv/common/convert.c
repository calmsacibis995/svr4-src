/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1988 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)unix_conv:common/convert.c	2.17.2.1"

#include <stdio.h>
#include <signal.h>

#include "old.a.out.h"
#include "old.ar.h"
#include "5.0.ar.h"

#include <ar.h>
#include <aouthdr.h>
#include <filehdr.h>
#include <scnhdr.h>
#include <linenum.h>
#include <reloc.h>
#include <syms.h>

#include <sys/types.h>
#include <sys/stat.h>
#include "paths.h"

#define	EVEN(x)	(((x) & 01) ? ((x) + 1) : (x))

void abort();        /* interrupt handler, defined below */
char *tempnam();/* C library routine for creating temp file name */
long time(); 	/* C library routine which gives you the time of day */
extern long sgetl();
extern int sputl();

long genarc(), genobj(), xgenarc();	/* routines for converting archives 
					   and object files.  the routines 
					   return the number of bytes in the 
					   converted file.  the converted file
					   is assumed to be in tmpfil.  a 
					   returned value of 0 implies that 
					   something has gotten screwed up. 
					*/
long genparc();		/* convert a 5.0 archive to a 6.0 archive file */

long gencpy(); /* routine for copying a file which does not (or can not)
		  need to be converted.  the tmpfil file is made a copy of
		  the infil input file.  */

long gen_headers(), gen_scns(), gen_text(), /* procedures for generating  */
     gen_data(), gen_reloc(), gen_syms();   /* object file components,    */
					    /* all return number of bytes */
					    /* generated in tmpfil file   */


/* input/output/temporary files */

char *infilname, *outfilname, *tmpfilname, *nm_newarc, *nm_member, *xfilname;

FILE	*infil, *outfil, *tmpfil, *newarc, *member, *xfil;

char buffer[BUFSIZ]; /* file copy buffer */


/* relocation entry patch list: patch[MAXSYMS]

	patch[i].sym = n ==> old index i is new index n
	patch[i].adj == ADJUST ==> relocatable data value must be adjusted
	patch[i].adj == NOADJUST ==> relocatable data value is okay as is
*/

#define MAXSYMS 2500 /* maximum number of symbols per object file */
#define ADJUST 1
#define NOADJUST 0

struct {
	short sym;
	short adj;
	} patch[MAXSYMS];


int arcflag = 0;	/* flag indicates that an archive is being processed */
int flag5 = 0;		/* when set, use 5.0 convert behavior */
int xflag = 0;		/* when set, convert a xenix archive into 6.0 format */


/* main program for convert 

   UNIX 5.0 transition tool for converting pre 5.0 UNIX archives and object
   files to the 5.0 format.
	
   usage is 'convert infile outfile'

   input file infile is interpreted as a pre 5.0 archive or object file and
   output file outfile is generated as an equivalent 5.0  archive or object
   file.

   Update for 6.0 -----------------------

   UNIX 6.0 transition tool for converting 5.0 archive files to 6.0 archive
   files.  The usage is now:

   convert [-5 | -x] infile outfile

   where the -5 option allows for the 5.0 behavior (as is reasonable)
   and the -x option allows for conversion of XENIX archives.

*/

main ( argc, argv )

int argc;
char * argv[];

{
	extern int	stat( );
	unsigned int in_magic; /* magic number of input file */

	long bytes_out;
	int  bytes_in;
	unsigned short  fmode;
	struct stat  statrec;


	/* trap for interrupts */

	if ((signal(SIGINT, SIG_IGN)) == SIG_DFL)
		(void) signal(SIGINT, abort);
	if ((signal(SIGHUP, SIG_IGN)) == SIG_DFL)
		(void) signal(SIGHUP, abort);
	if ((signal(SIGQUIT, SIG_IGN)) == SIG_DFL)
		(void) signal(SIGQUIT, abort);
	if ((signal(SIGTERM, SIG_IGN)) == SIG_DFL)
		(void) signal(SIGTERM, abort);


	if (strcmp(argv[1], "-5") == 0)
	{
		flag5 = 1;
		argv++;
		argc--;
	} else if (strcmp(argv[1], "-x") == 0) {
		xflag = 1;
		argv++;
		argc--;
	}

	if (argc != 3) {
		fprintf(stderr,"usage: convert [-5 | -x] infile outfile\n");
		exit(1);
	}

	infilname = argv[1]; /* get input file name from command line */
	outfilname = argv[2];/* get output file name from command line*/

	if (strcmp(infilname,outfilname) == 0) {
		/* input cannot be the same as output */
		fprintf(stderr,
		   "convert: input file cannot be the same as output file\n");
		exit(1);
	}

	if ((infil = fopen(infilname,"r")) == NULL) 
		/* can't get the input file */
		readerr(infilname);
	if (stat(infilname, &statrec) != 0) {
		fprintf(stderr,"convert: can not obtain mode of file %s",
		    infilname);
		exit(1);
	}
	fmode=statrec.st_mode;

	/*
	* Check for accidental overwrite of possibly precious file
	*/
	if (access(outfilname, 0) == 0)
	{
		fprintf(stderr,
			"convert: output file \"%s\" already exists!\n",
			outfilname);
		exit(1);
	}
	if ((outfil = fopen(outfilname,"w")) == NULL) 
		/* can't write onto the output file */
		writerr(outfilname);

	tmpfilname = tempnam(TMPDIR,"conv");
	if ((tmpfil = fopen(tmpfilname,"w")) == NULL)
		/* can't write onto the temporary file */
		writerr(tmpfilname);


	/* let's get down to business: are we dealing with an archive or
	   an object file?  answer: look at the 'magic number' */

	if (fread(&in_magic,1,sizeof(in_magic),infil) != sizeof(in_magic))
		readerr(infilname);
	in_magic &= 0177777;
	fseek(infil,0L,0);

	switch (in_magic) {

		case OARMAG:			/* archive input */
			if (!flag5 && !xflag)
			{
				fprintf(stderr, "convert: \"%s\": must specify -5 or -x option to convert pre-5.0 archive\n", infilname);
				abort();
			}
			arcflag++;
			if (xflag) {
				xfilname = tempnam(TMPDIR, "conv");
				if ((xfil = fopen(xfilname, "w")) == NULL)
					writerr(xfilname);
				bytes_out = xgenarc(infil);
				if ((xfil = fopen(xfilname, "r")) == NULL)
					readerr(xfilname);
				fclose(tmpfil);
				if ((tmpfil = fopen(tmpfilname, "w")) == NULL)
					writerr(tmpfilname);
				bytes_out = genparc(xfil);
			} else
				bytes_out = genarc(infil);
			break;

#if vax
		case A_MAGIC1:			/* object file */
		case A_MAGIC2:
			if (!flag5)
			{
				fprintf(stderr, "convert: \"%s\": must specify -5 option to convert pre-5.0 object files\n", infilname);
				abort();
			}
			bytes_out = genobj(infil);
			break;
#endif

		default:			/* questionable input */
			/*
			* See if it is a 5.0 archive to be converted
			*/
			if (fread(buffer, 1, sizeof(char) * SARMAG5, infil)
				!= sizeof(char) * SARMAG5)
			{
				readerr(infilname);
			}
			rewind(infil);
			if (strncmp(buffer, ARMAG5, SARMAG5) == 0)
			{
				if (flag5)
				{
					fprintf(stderr, "convert: \"%s\": cannot convert 5.0 archive with -5 option\n", infilname);
					abort();
				}
				bytes_out = genparc(infil);
			}
			else
				bytes_out = gencpy(infil, infilname);
			break;
		};


	/* copy converted output, which resides in the tmpfil file to the
	   outfil file */

	fclose(infil);
	if ((outfil = fopen(outfilname,"w")) == NULL)
		writerr(outfilname);

	/* always begin at the beginning */
	fclose(tmpfil);
	if ((tmpfil = fopen(tmpfilname,"r")) == NULL)
		/* can't read the temporary file */
		readerr(tmpfilname);

	while (bytes_out) {
		bytes_in = fread(buffer,1,BUFSIZ,tmpfil);
		bytes_out -= (long)fwrite(buffer,1,bytes_in,outfil);
		if (bytes_in < BUFSIZ && bytes_out > 0)
			writerr(outfilname);
	};

	/* we are all done, so clean up! */

	fclose(outfil);
	chmod(outfilname, fmode);
	fclose(tmpfil);
	unlink(tmpfilname);
	if (xflag) {
		fclose(xfil);
		unlink(xfilname);
	}
	exit(0);
}


/* interrupt handler: delete all the temp files, unlink the output
   and then abort with an error code of 1 */

void
abort ()
{
	if (infil)
		fclose(infil);		/* close all files */
	if (outfil)
		fclose(outfil);
	if (tmpfil)
		fclose(tmpfil);
	if (newarc)
		fclose(newarc);
	if (member)
		fclose(member);
	if (xfil)
		fclose(xfil);
	if (tmpfilname)
		unlink(tmpfilname);	/* delete temp file and output */
	if (outfilname)
		unlink(outfilname);
	if (nm_newarc)
		unlink(nm_newarc);
	if (nm_member)
		unlink(nm_member);
	if (xfilname)
		unlink(xfilname);
	exit(1);	/* abort! */
}


readerr(filname) /* input read error, give the user a reason and abort */

char *filname;

{
	fprintf(stderr,"convert: input error on file %s ",filname);
	perror(" ");
	abort();
}


writerr(filname) /* output write error, give the user a reason and abort */

char *filname;

{
	fprintf(stderr,"convert: output error on file %s ",filname);
	perror(" ");
	abort();
}

long
genarc (arcfile) /* procedure to convert a pre 5.0 archive into
		   a 5.0 archive format.  the converted file
		   is written onto the temp file tmpfil.  the
		   genarc function returns the number of bytes
		   in the converted file tmpfil. */

FILE *arcfile;

{
	long arc_size; /* size of converted archive file on output */
	long mem_size; /* size of an archive member file */
	unsigned short magic_nbr;
	short incr;

	struct ar_hdr5  new_header;
	struct oar_hdr oldf_header;
	struct arf_hdr5 newf_header;

	/* set up temporary files */

	nm_newarc = tempnam(TMPDIR,"conv");
	nm_member = tempnam(TMPDIR,"conv");

	if ((newarc = fopen(nm_newarc,"w")) == NULL)
		writerr(nm_newarc);

	/* skip past the magic number in the input file: arcfile */
	fseek(arcfile,(long)sizeof(int),0);
		
	/* set up the new archive header */
	strncpy(new_header.ar_magic,ARMAG5,SARMAG5);
	strncpy(new_header.ar_name,infilname,sizeof(new_header.ar_name));
	sputl(time(NULL),new_header.ar_date);
	sputl(0L,new_header.ar_syms); /* recreate archive without symbols */

		/* a word to the wise is sufficient */

	fprintf(stderr,"convert: warning, archive symbol table not created\n");
	fprintf(stderr,"         execute 'ar ts %s' to generate symbol table\n",
			outfilname);

	if ((arc_size = fwrite(&new_header,1,sizeof(new_header),newarc)) != 
		sizeof(new_header))
			writerr(nm_newarc);

	/* now process each archive member in turn */

	while (fread(&oldf_header,1,sizeof(oldf_header),arcfile) ==
			sizeof(oldf_header)) {

#if vax
		/* if the member has a name of "__.SYMDEF" then we can
		   delete it from the new archive.  this special member
		   was used with earlier versions of random access
		   libraries.  it has no application or use in the new
		   archive format. */

		if (strcmp(oldf_header.ar_name,"__.SYMDEF") == 0) {
			fseek(arcfile,EVEN(oldf_header.ar_size),1); /* skip */
			continue; /* and go on to the next archive member */
		};
#endif

		/* translate header data for each member */
		strncpy(newf_header.arf_name,oldf_header.ar_name,
				sizeof(oldf_header.ar_name));
		sputl(oldf_header.ar_date,newf_header.arf_date);
		sputl((long)oldf_header.ar_uid,newf_header.arf_uid);
		sputl((long)oldf_header.ar_gid,newf_header.arf_gid);
		sputl(oldf_header.ar_mode,newf_header.arf_mode);

		/* prepare the member for conversion */
		
		if ((member = fopen(nm_member,"w")) == NULL)
			writerr(nm_member);
		
		mem_size = oldf_header.ar_size;

		if (mem_size & 1) { /* ar expects members to be evenly sized */
			mem_size++;
			incr = 1;
		} else
			incr = 0;

		while (mem_size >= BUFSIZ) {
			if (fread(buffer,1,BUFSIZ,arcfile) != BUFSIZ)
				readerr(infilname);
			if (fwrite(buffer,1,BUFSIZ,member) != BUFSIZ)
				writerr(nm_member);
			mem_size -= BUFSIZ;
		};
		if (mem_size) {
			if (fread(buffer,1,mem_size,arcfile) != mem_size)
				readerr(infilname);
			if (fwrite(buffer,1,mem_size,member) != mem_size)
				writerr(nm_member);
		};
		
		/* now perform the actual conversion */

		fclose(member);
		if ((member = fopen(nm_member,"r")) == NULL)
			readerr(nm_member);

		if (oldf_header.ar_size < sizeof(magic_nbr))
			goto just_copy;			/* don't bother */
		if (fread(&magic_nbr,1,sizeof(magic_nbr),member) !=
			sizeof(magic_nbr))
			readerr(nm_member);
		
		fseek(member,0L,0);
	
		switch (magic_nbr) {
#if vax
					/* convert vax object files */
			case A_MAGIC1:
			case A_MAGIC2:
				mem_size = genobj(member);
				break;
#endif
					/* other members dont need conversion */
			default:
			just_copy:
				mem_size = gencpy(member,newf_header.arf_name);
				break;
		};


		fclose(member);

		/* now let's put the sucker back into the new archive */

		sputl((mem_size-incr),newf_header.arf_size); /* finish up */
		
		if (fwrite(&newf_header,1,sizeof(newf_header),newarc) !=
			sizeof(newf_header)) writerr(nm_newarc);

		arc_size += mem_size + sizeof(newf_header);

		/* put the new converted member into the new archive */

		fclose(tmpfil);
		if ((tmpfil = fopen(tmpfilname,"r")) == NULL)
			readerr(tmpfilname);
		while (mem_size = fread(buffer,1,BUFSIZ,tmpfil))
			fwrite(buffer,1,mem_size,newarc);
	};

	/* copy new archive file to tmpfil */

	fclose(newarc);
	fclose(tmpfil);
	if ((newarc = fopen(nm_newarc,"r")) == NULL)
		readerr(nm_newarc);
	if ((tmpfil = fopen(tmpfilname,"w")) == NULL)
		writerr(tmpfilname);

	while (mem_size = fread(buffer,1,BUFSIZ,newarc))
		fwrite(buffer,1,mem_size,tmpfil);

	/* time to clean up */

	fclose(newarc);
	unlink(nm_newarc);
	fclose(member);
	unlink(nm_member);

	return(arc_size);
}

long
xgenarc (arcfile) /* procedure to convert a pre 5.0 archive into
		     a 5.0 archive format.  the converted file
		     is written onto the temp file xfil.  the
		     xgenarc function returns the number of bytes
		     in the converted file xfil. */

FILE *arcfile;

{
	long arc_size; /* size of converted archive file on output */
	long mem_size; /* size of an archive member file */
	unsigned short magic_nbr;
	short incr;

	struct ar_hdr5  new_header;
	struct xar_hdr oldf_header;
	struct arf_hdr5 newf_header;

	/* set up temporary files */

	nm_newarc = tempnam(TMPDIR,"conv");
	nm_member = tempnam(TMPDIR,"conv");

	if ((newarc = fopen(nm_newarc,"w")) == NULL)
		writerr(nm_newarc);

	/* skip past the magic number in the input file: arcfile */
	fseek(arcfile,(long)sizeof(short),0);
		
	/* set up the new archive header */
	strncpy(new_header.ar_magic,ARMAG5,SARMAG5);
	strncpy(new_header.ar_name,infilname,sizeof(new_header.ar_name));
	sputl(time(NULL),new_header.ar_date);
	sputl(0L,new_header.ar_syms); /* recreate archive without symbols */

		/* a word to the wise is sufficient */

	fprintf(stderr,"convert: warning, archive symbol table not created\n");
	fprintf(stderr,"         execute 'ar ts %s' to generate symbol table\n",
			outfilname);

	if ((arc_size = fwrite(&new_header,1,sizeof(new_header),newarc)) != 
		sizeof(new_header))
			writerr(nm_newarc);

	/* now process each archive member in turn */

	while (fread(&oldf_header,1,sizeof(oldf_header),arcfile) ==
			sizeof(oldf_header)) {

		/* if the member has a name of "__.SYMDEF" then we can
		   delete it from the new archive.  this special member
		   was used with earlier versions of random access
		   libraries.  it has no application or use in the new
		   archive format. */


		if (strcmp(oldf_header.ar_name,"__.SYMDEF") == 0) {
			fseek(arcfile,EVEN(oldf_header.ar_size),1); /* skip */
			continue; /* and go on to the next archive member */
		};

		/* translate header data for each member */
		strncpy(newf_header.arf_name,oldf_header.ar_name,
				sizeof(oldf_header.ar_name));
		sputl(oldf_header.ar_date,newf_header.arf_date);
		sputl((long)oldf_header.ar_uid,newf_header.arf_uid);
		sputl((long)oldf_header.ar_gid,newf_header.arf_gid);
		sputl((long)oldf_header.ar_mode,newf_header.arf_mode);

		/* prepare the member for conversion */
		
		if ((member = fopen(nm_member,"w")) == NULL)
			writerr(nm_member);
		
		mem_size = oldf_header.ar_size;

		if (mem_size & 1) { /* ar expects members to be evenly sized */
			mem_size++;
			incr = 1;
		} else
			incr = 0;

		while (mem_size >= BUFSIZ) {
			if (fread(buffer,1,BUFSIZ,arcfile) != BUFSIZ)
				readerr(infilname);
			if (fwrite(buffer,1,BUFSIZ,member) != BUFSIZ)
				writerr(nm_member);
			mem_size -= BUFSIZ;
		};
		if (mem_size) {
			if (fread(buffer,1,mem_size,arcfile) != mem_size)
				readerr(infilname);
			if (fwrite(buffer,1,mem_size,member) != mem_size)
				writerr(nm_member);
		};
		
		/* now perform the actual conversion */

		fclose(member);
		if ((member = fopen(nm_member,"r")) == NULL)
			readerr(nm_member);

		/* just copy the member */
		mem_size = gencpy(member,newf_header.arf_name);

		fclose(member);

		/* now let's put the sucker back into the new archive */

		sputl((mem_size-incr),newf_header.arf_size); /* finish up */
		
		if (fwrite(&newf_header,1,sizeof(newf_header),newarc) !=
			sizeof(newf_header)) writerr(nm_newarc);

		arc_size += mem_size + sizeof(newf_header);

		/* put the new converted member into the new archive */

		fclose(tmpfil);
		if ((tmpfil = fopen(tmpfilname,"r")) == NULL)
			readerr(tmpfilname);
		while (mem_size = fread(buffer,1,BUFSIZ,tmpfil))
			fwrite(buffer,1,mem_size,newarc);
	};

	/* copy new archive file to xfil */

	fclose(newarc);
	fclose(xfil);
	if ((newarc = fopen(nm_newarc,"r")) == NULL)
		readerr(nm_newarc);
	if ((xfil = fopen(xfilname,"w")) == NULL)
		writerr(xfilname);

	while (mem_size = fread(buffer,1,BUFSIZ,newarc))
		fwrite(buffer,1,mem_size,xfil);

	/* time to clean up */

	fclose(newarc);
	unlink(nm_newarc);
	fclose(member);
	unlink(nm_member);
	fclose(xfil);

	return(arc_size);
}


#define NSYMS	2000	/* max number of symbols in random access directory */

/*
* Convert a 5.0 archive to a 6.0 archive.  Write output to ``tmpfil''.
*/
long
genparc(arcfile)
	FILE *arcfile;
{
	char sym_buf[NSYMS * (SYMNMLEN + 1)];	/* room for string table */
	char sym_off[NSYMS][sizeof(long) / sizeof(char)];	/* offsets */
	char num_syms[sizeof(long) / sizeof(char)];	/* num sym in dir. */
	char hdr_buf[(sizeof(struct ar_hdr) + 1) * sizeof(char)];
	struct ar_hdr5 old_hdr;		/* 5.0 entire archive header */
	struct arf_hdr5 old_fhdr;	/* 5.0 archive member header */
	struct ar_sym5 old_syms[NSYMS];	/* 5.0 random access directory */
	long off_pair[NSYMS][2];	/* map from 5.0 to 6.0 offsets */
	struct ar_hdr new_fhdr;		/* 6.0 archive member header */
	long nsyms, diff, size;
	int lastoff;
	register char *sp;
	register int i, j;

	if (fread((char *)&old_hdr, sizeof(old_hdr), 1, arcfile) != 1)
		readerr(infilname);
	/*
	* write out archive magic string
	*/
	if (fwrite(ARMAG, sizeof(char), SARMAG, tmpfil) != SARMAG)
		writerr(tmpfilname);
	for (i = 0; i < sizeof(long) / sizeof(char); i++)
		num_syms[i] = old_hdr.ar_syms[i];
	nsyms = sgetl(num_syms);
	if (nsyms > NSYMS)
	{
		fprintf(stderr,
			"convert: \"%s\" too many symbols in directory\n",
			infilname);
		abort();
	}
	if (fread(old_syms, sizeof(old_syms[0]), nsyms, arcfile) != nsyms)
		readerr(infilname);
	if (nsyms > 0)		/* Create a new symbol directory */
	{
#ifdef FLEXNAMES
		int trunc_cnt = 0;	/* num. of poss. truncated symbols */
#endif

		/*
		* length of the offset table
		*/
		size = (nsyms + 1) * (sizeof(long) / sizeof(char));
		/*
		* generate string table
		*/
		sp = sym_buf;
		for (j = 0; j < nsyms; j++)
		{
			i = 0;
			while (i < SYMNMLEN && old_syms[j].sym_name[i] != ' ')
				*sp++ = old_syms[j].sym_name[i++];
#ifdef FLEXNAMES
			if (i >= SYMNMLEN)
				trunc_cnt++;
#endif
			*sp++ = '\0';
		}
#ifdef FLEXNAMES
		/*
		* note the number of possibly truncated
		* symbols in the directory
		*/
		if (trunc_cnt > 0)
			fprintf(stderr, "Note: %d of %ld symbols possibly truncated in archive symbol table\n",
				trunc_cnt, nsyms);
#endif
		/*
		* set size to the actual length of the directory file
		*/
		size += (sp - sym_buf);
		if (size & 01)		/* archive assumes even length */
		{
			size++;
			*sp++ = '\0';
		}
		/*
		* write out header for symbol directory
		*/
		if (sprintf(hdr_buf, "%-16s%-12ld%-6u%-6u%-8o%-10ld%-2s",
			"/               ", time((long *)0), 0, 0, 0,
			size, ARFMAG) != sizeof(new_fhdr))
		{
			fprintf(stderr,
				"convert: internal header creation error\n");
			abort();
		}
		(void)strncpy((char *)&new_fhdr, hdr_buf, sizeof(new_fhdr));
		if (fwrite((char *)&new_fhdr, sizeof(new_fhdr), 1, tmpfil) != 1)
			writerr(tmpfilname);
		/*
		* set size to the distance to the beginning of
		* the first actual archive member's header.
		*/
		size += (SARMAG * sizeof(char)) + sizeof(new_fhdr);
		/*
		* write out the symbol directory, skipping the offsets table
		*/
		if (fwrite(num_syms, sizeof(num_syms), 1, tmpfil) != 1 ||
			fseek(tmpfil, sizeof(sym_off[0]) * nsyms, 1) != 0 ||
			fwrite(sym_buf, sizeof(char), sp - sym_buf, tmpfil)
			!= sp - sym_buf)
		{
			writerr(tmpfilname);
		}
	}
	else
		size = sizeof(char) * SARMAG;
	/*
	* for each archive file member, create a new header, copy the
	* contents, and change the optional pad to a \n, remember mapping
	* from old header offsets to new header offsets.
	*/
	lastoff = 0;
	while (fread((char *)&old_fhdr, sizeof(old_fhdr), 1, arcfile) == 1)
	{
		char	ustr[12];  /* string used to hold uid in decimal */
		char	gstr[12];  /* string used to hold gid in decimal */
		off_pair[lastoff][0] = ftell(arcfile) - sizeof(old_fhdr);
		off_pair[lastoff][1] = ftell(tmpfil);
		lastoff++;
		diff = sgetl(old_fhdr.arf_size);
		sprintf(ustr, "%u", sgetl(old_fhdr.arf_uid));
		sprintf(gstr, "%u", sgetl(old_fhdr.arf_gid));
		if (sprintf(hdr_buf, "%-16s%-12ld%-6.6s%-6.6s%-8o%-10ld%-2s",
			strcat(old_fhdr.arf_name, "/"),
			sgetl(old_fhdr.arf_date), ustr,
			gstr,sgetl(old_fhdr.arf_mode),
			diff, ARFMAG) != sizeof(new_fhdr))
		{
			fprintf(stderr,
				"convert: internal header creation error\n");
			abort();
		}
		(void)strncpy((char *)&new_fhdr, hdr_buf, sizeof(new_fhdr));
		if (fwrite((char *)&new_fhdr, sizeof(new_fhdr), 1, tmpfil) != 1)
			writerr(tmpfilname);
		size += sizeof(new_fhdr);
		j = diff;
		while ((i = fread(buffer, sizeof(char),
			j > BUFSIZ ? BUFSIZ : j, arcfile)) == BUFSIZ)
		{
			j -= fwrite(buffer, sizeof(char), i, tmpfil);
		}
		if (j >= BUFSIZ)
			readerr(infilname);
		if (fwrite(buffer, sizeof(char), i, tmpfil) != i)
			writerr(tmpfilname);
		size += diff;
		if (diff & 01)		/* eat pad also */
		{
			if (fread(buffer, sizeof(char), 1, arcfile) != 1)
				readerr(infilname);
			buffer[0] = '\n';
			if (fwrite(buffer, sizeof(char), 1, tmpfil) != 1)
				writerr(tmpfilname);
			size++;
		}
	}
	/*
	* Rewrite over the table of offsets in the archive symbol
	* directory.  Assume that off_pair is in sorted order.
	*/
	if (fseek(tmpfil,
		SARMAG * sizeof(char) + sizeof(new_fhdr) + sizeof(long), 0)
		!= 0)
	{
		fprintf(stderr, "convert: cannot seek back to offsets table\n");
		abort();
	}
	j = 0;
	for (i = 0; i < nsyms; i++)
	{
		long off = sgetl(old_syms[i].sym_ptr);

		for (off_pair[lastoff][0] = off; off_pair[j][0] != off; j++)
			;
		if (j >= lastoff)
		{
			fprintf(stderr, "convert: cannot find old offset!\n");
			abort();
		}
		sputl(off_pair[j][1], sym_off[i]);
	}
	/*
	* Now, insert the offsets table.
	*/
	if (fwrite(sym_off, sizeof(sym_off[0]), nsyms, tmpfil) != nsyms)
		writerr(tmpfilname);
	/*
	* Check for extra stuff - just to be sure
	*/
	if (fread(buffer, sizeof(char), BUFSIZ, arcfile) != 0)
	{
		fprintf(stderr,
			"convert: extra chars at end of 5.0 archive file %s\n",
			infilname);
		abort();
	}
	return (size);
}




long
gencpy(cpyfile, cpyname) /* routine for copying a file which does not 
		   	    need to be converted.  the tmpfil file is made
		    	    a copy of the infil input file.  */

FILE *cpyfile;
char *cpyname;

{
	long cpy_size;
	int bytes_in;

	cpy_size = 0;
	
	fclose(tmpfil); /* start with a clean file */
	if ((tmpfil = fopen(tmpfilname,"w")) == NULL)
		writerr(tmpfilname);

	while (bytes_in = fread(buffer,1,BUFSIZ,cpyfile)) {
		fwrite(buffer,1,bytes_in,tmpfil);
		cpy_size = cpy_size + bytes_in;
		}

	/* warn the user */
#if u3b || M32
	if(strcmp(infilname, cpyname) == 0)
#endif
	fprintf(stderr,"convert: warning, contents of file %s not modified\n",
			cpyname);

	return(cpy_size);
}





long
genobj (objfile) /* procedure to convert a pre 5.0 vax object file into
		    a 5.0 object file format.  the converted file
		    is written onto the temp file tmpfil.  the
		    genobj function returns the number of bytes
		    in the converted file tmpfil. */

FILE *objfile;

{
	long syms_output; /* nbr of symbols output in the new symbol table */
	long obj_size; /* size of converted object file on output */
	struct exec old_header; /* pre 5.0 a.out header */

	obj_size = 0;

	fclose(tmpfil);
	if ((tmpfil = fopen(tmpfilname,"w")) == NULL)
		writerr(tmpfilname);

	/* component inputs are read from the file objfile as needed in
	   sequence.  outputs are generated in sequence to the tmpfil file.
	   note that line numbers are not supplied in the converted output
	   file.  all sdb (pre 5.0 .stabtypes) data is deleted from the
	   input file if it exists. */

	obj_size += gen_headers(objfile, &old_header);
	obj_size += gen_scns(objfile, &old_header);
	obj_size += gen_text(objfile, &old_header);
	obj_size += gen_data(objfile, &old_header);
	obj_size += gen_reloc(objfile, &old_header);

	syms_output = gen_syms(objfile, &old_header);
	obj_size += syms_output;
	syms_output /= SYMESZ;


	/* patch up the relocation entries to match the new symbol table */

	patch_reloc(tmpfil, syms_output, &old_header);

	/* converted output is left in the file tmpfil, the size of tmpfil
	   is returned from the variable obj_size */

	return(obj_size);
}






long
gen_headers (file, old_header) /* procedure reads in the old a.out header
				  and generates a new file header as well
				  as a new auxiliary header.  the old a.out
				  header is returned so that all the other
				  component processors can use the info. 
				  all output is assumed to be written to the
				  tmpfil file.  the number of bytes output
				  is returned. */

FILE *file;
struct exec *old_header;

{
	long header_size;
	struct filehdr new_header;

	header_size = sizeof(struct filehdr);

	/* get the old header data */

	if (fread(old_header,1,sizeof(struct exec),file)!=sizeof(struct exec))
		readerr(infilname);

	/* compute the new header data */

	/* magic number */

	switch (old_header->a_magic) {

		case A_MAGIC1: new_header.f_magic = VAXWRMAGIC;
			       break;

		case A_MAGIC2: new_header.f_magic = VAXROMAGIC;
			       break;

		default: fprintf(stderr,"illegal magic number = %o\n",
					old_header->a_magic);
			 abort();
		}

	/* number of sections */

	new_header.f_nscns = 3; /* text, data and bss makes three */

	/* time & date stamp */
	
	new_header.f_timdat = time(NULL); /* hickory dickory dock, the mouse
					     ran up the clock. ... */

	/* file ptr to symtab */

	new_header.f_symptr = /* the sum of a whole bunch of things */
			    sizeof(struct filehdr)     /* file header */
				/* see below */	       /* optional header */
			+  (3 * sizeof(struct scnhdr)) /* 3 section headers */
			+   old_header->a_text	       /* raw text area */
			+   old_header->a_data	       /* raw data area */
			+  ((old_header->a_trsize/sizeof(struct relocation_info))
			  * RELSZ)       /* text relocation */
			+   ((old_header->a_drsize/sizeof(struct relocation_info))
			  * RELSZ)      /* data relocation */
			+   0; 	/* none */	       /* text, data line nbr */

	/* number of symtab entries */

	new_header.f_nsyms = old_header->a_syms / sizeof(struct onlist);

	/* sizeof optional header */

	if ((!(old_header->a_trsize || old_header->a_drsize)) && !arcflag) {
		/* we shall have an optional header */
		new_header.f_symptr += sizeof(struct aouthdr);
		new_header.f_opthdr =  sizeof(struct aouthdr);
		}
	else
		new_header.f_opthdr = 0;

	/* flags */

	new_header.f_flags = F_AR32WR; /* file created on a vax */
	new_header.f_flags |= F_LNNO;  /* line nbrs stripped */
	if ((!(old_header->a_trsize || old_header->a_drsize)) && !arcflag) {
		new_header.f_flags |= F_RELFLG; /* no reloc entries */
		new_header.f_flags |= F_EXEC;   /* file is executable */
		};
	
	/* put out the new header to the tmpfil file */

	if (fwrite(&new_header,1,sizeof(struct filehdr),tmpfil) !=
		sizeof(struct filehdr)) writerr(tmpfilname);

	/* generate an optional header if the input is a fully bound
	   executable module (i.e., previously link edited a.out file) */

	if ((!(old_header->a_trsize || old_header->a_drsize)) && !arcflag) {
		struct aouthdr new_opt_hdr;
		
		header_size += sizeof(struct aouthdr);

		new_opt_hdr.magic = old_header->a_magic;
		new_opt_hdr.vstamp = old_header->a_stamp;
		new_opt_hdr.tsize = old_header->a_text;
		new_opt_hdr.dsize = old_header->a_data;
		new_opt_hdr.bsize = old_header->a_bss;
		new_opt_hdr.entry = old_header->a_entry;
		new_opt_hdr.text_start = 0L; /* always */
		new_opt_hdr.data_start = old_header->a_text;
		if (old_header->a_magic == A_MAGIC2) { /* read only text */
			if (new_opt_hdr.data_start & 0777) {
				new_opt_hdr.data_start &= (~0777L);
				new_opt_hdr.data_start += 512;
				}
		}

		/* write out the optional header */

		if (fwrite(&new_opt_hdr,1,sizeof(struct aouthdr),tmpfil) !=
			sizeof(struct aouthdr)) writerr(tmpfilname);
	}

	return(header_size);
}






long
gen_scns (file, old_header) /* procedure generates text, data and bss section
			       headers based upon the information found in the
			       old a.out header.  all info is assumed to be
			       written to the tmpfil file.  the number of
			       bytes output to the tmpfil file is returned. */

FILE *file;
struct exec *old_header;

{
	struct scnhdr new_header;
	long scns_size, datastart;

	/* generate section headers for .text, .data and .bss */

	scns_size = 3 * sizeof(struct scnhdr);


	/* generate .text section header */
	
	/* section name of ".text" */

	strncpy(new_header.s_name,_TEXT,sizeof(new_header.s_name));

	/* .text physical and virtual address is always zero in vaxland */

	new_header.s_paddr = 0L;
	new_header.s_vaddr = 0L;

	/* .text section size */

	new_header.s_size = old_header->a_text;
	
	/* .text area file pointer */

	new_header.s_scnptr = sizeof(struct filehdr) + scns_size;
	
	/* was there an optional header generated? */

	if ((!(old_header->a_trsize || old_header->a_drsize)) && !arcflag)
		new_header.s_scnptr += sizeof(struct aouthdr);

	/* relocation entry file pointer */

	if (new_header.s_nreloc = (old_header->a_trsize /
				sizeof(struct relocation_info)))
		new_header.s_relptr = new_header.s_scnptr +
				      old_header->a_text +
				      old_header->a_data;
	else
		new_header.s_relptr = 0L;

	/* line numbers do not exist in the old formats, so ignore them */

	new_header.s_lnnoptr = 0L;
	new_header.s_nlnno = 0;

	/* set the section flag */

	new_header.s_flags = STYP_REG; /* just your everyday regular section */

	/* output the .text section header */

	if (fwrite(&new_header,1,sizeof(struct scnhdr),tmpfil) !=
		sizeof(struct scnhdr)) writerr(tmpfilname);



	/* generate .data section header */
	
	/* section name of ".data" */

	strncpy(new_header.s_name,_DATA,sizeof(new_header.s_name));

	/* .data section  physical and virtual address */

	datastart = old_header->a_text;
	if ((old_header->a_magic == A_MAGIC2) && /* read only text */
	    ((!(old_header->a_trsize || old_header->a_drsize))) && !arcflag)
		if (datastart & 0777) {
			datastart = datastart &= (~0777L);
			datastart += 512;
			};

	new_header.s_paddr = datastart;
	new_header.s_vaddr = datastart;

	/* .data section size */

	new_header.s_size = old_header->a_data;
	
	/* .data area file pointer */

	new_header.s_scnptr = sizeof(struct filehdr) + scns_size
				+ old_header->a_text;
	
	/* was there an optional header generated? */

	if ((!(old_header->a_trsize || old_header->a_drsize)) && !arcflag)
		new_header.s_scnptr += sizeof(struct aouthdr);

	/* relocation entry file pointer */

	if (new_header.s_nreloc = (old_header->a_drsize /
				sizeof(struct relocation_info)))
		new_header.s_relptr = new_header.s_scnptr +
				      old_header->a_data +
				      (old_header->a_trsize /
				       sizeof(struct relocation_info)) *
				       RELSZ;
	else
		new_header.s_relptr = 0L;

	/* line numbers do not exist in the old formats, so ignore them */

	new_header.s_lnnoptr = 0L;
	new_header.s_nlnno = 0;

	/* set the section flag */

	new_header.s_flags = STYP_REG; /* just your everyday regular section */

	/* output the .data section header */

	if (fwrite(&new_header,1,sizeof(struct scnhdr),tmpfil) !=
		sizeof(struct scnhdr)) writerr(tmpfilname);



	/* generate .bss section header */
	
	/* section name of ".bss" */

	strncpy(new_header.s_name,_BSS,sizeof(new_header.s_name));

	/* .bss section  physical and virtual address */

	new_header.s_paddr = datastart + old_header->a_data;
	new_header.s_vaddr = datastart + old_header->a_data;

	/* .bss section size */

	new_header.s_size = old_header->a_bss;
	
	/* .bss area file pointer */

	new_header.s_scnptr =  0L; /* .bss section not stored in the file */
	
	/* relocation entry file pointer */

	new_header.s_nreloc = 0;
	new_header.s_relptr = 0L;

	/* line numbers do not exist in the old formats, so ignore them */

	new_header.s_lnnoptr = 0L;
	new_header.s_nlnno = 0;

	/* set the section flag */

	new_header.s_flags = STYP_REG; /* just your everyday regular section */

	/* output the .bss section header */

	if (fwrite(&new_header,1,sizeof(struct scnhdr),tmpfil) !=
		sizeof(struct scnhdr)) writerr(tmpfilname);



	return(scns_size);
}






long
gen_text (file, old_header) /* procedure copies the raw text area from the
			       input file parameter into the tmpfil file.
			       the size of the raw text area is gleaned
			       from the old a.out header.  the number of bytes
			       output to the tmpfil file is returned. */

FILE *file;
struct exec *old_header;

{
	long text_size, bytes;

	bytes = text_size = old_header->a_text;


	while (bytes >= BUFSIZ) {
		if (fread(buffer,1,BUFSIZ,file) != BUFSIZ)
			readerr(infilname);
		if (fwrite(buffer,1,BUFSIZ,tmpfil) != BUFSIZ)
			writerr(tmpfilname);
		bytes -= BUFSIZ;
		};

	if (bytes > 0) {
		if (fread(buffer,1,bytes,file) != bytes)
			readerr(infilname);
		if (fwrite(buffer,1,bytes,tmpfil) != bytes)
			writerr(tmpfilname);
		};


	return(text_size);
}






long
gen_data (file, old_header) /* procedure copies the raw data area from the
			       input file parameter into the tmpfil file.
			       the size of the raw data area is gleaned
			       from the old a.out header.  the number of bytes
			       output to the tmpfil file is returned. */

FILE *file;
struct exec *old_header;

{
	long data_size, bytes;

	bytes = data_size = old_header->a_data;


	while (bytes >= BUFSIZ) {
		if (fread(buffer,1,BUFSIZ,file) != BUFSIZ)
			readerr(infilname);
		if (fwrite(buffer,1,BUFSIZ,tmpfil) != BUFSIZ)
			writerr(tmpfilname);
		bytes -= BUFSIZ;
		};

	if (bytes > 0) {
		if (fread(buffer,1,bytes,file) != bytes)
			readerr(infilname);
		if (fwrite(buffer,1,bytes,tmpfil) != bytes)
			writerr(tmpfilname);
		};



	return(data_size);
}






long
gen_reloc (file, old_header) /* procedure reads in relocation info from
				input file and converts to new format
				before rewriting information onto the
				output tmpfil file.  the number of bytes
				generated in the new format relocation
				area is returned.  both the text and
				data section relocation entries are created
				(text followed by data). */

FILE *file;
struct exec *old_header;

{
	long reloc_size, entries;
	struct reloc new_entry;
	struct relocation_info old_entry;

	entries = (old_header->a_trsize + old_header->a_drsize) /
		  sizeof(struct relocation_info);
	reloc_size = entries * RELSZ;


	while (entries--) { /* process each relocation entry in order */

		/* get the old entry */

		if (fread(&old_entry,1,sizeof(struct relocation_info),file)
			!= sizeof(struct relocation_info)) readerr(infilname);

		/* convert the old entry to a temporary version of the new
		   entry, remember that symbol indeces have to be patched
		   up later. */

		/* an address is an address ... */

		new_entry.r_vaddr = old_entry.r_address;
		
		/* make .data relocation entry address relative to the
		   beginning of the file */

		if (entries < (old_header->a_drsize /
			sizeof(struct relocation_info)))
			new_entry.r_vaddr += old_header->a_text;

		/* if pc relative w/o symbol put in special -1 index, if
		   not pc relative and not extern put in a special ptr
		   to artificial .text, .data and .bss symbols else
		   store the unpatched symbol table ptr */

		if ((old_entry.r_pcrel & (!old_entry.r_extern)) && 
			(old_entry.r_symbolnum == 2)) 
				new_entry.r_symndx = -1L;
			
		else
		if (!old_entry.r_extern)
			switch (old_entry.r_symbolnum) {

			   case 4:  /* relocate off .text symbol */
			   case 5:
				new_entry.r_symndx = -4L;
				break;

			   case 6:  /* relocate off .data symbol */
			   case 7:
				new_entry.r_symndx = -6L;
				break;

			   case 8: /* relocate off .bss symbol */
			   case 9:
				new_entry.r_symndx = -8L;
				break;

			   default: /* result of JFR (asshole) kludge */
				if (entries < (old_header->a_drsize /
					sizeof(struct relocation_info)))
					new_entry.r_symndx = -6L;
				else
					new_entry.r_symndx = -4L;
				break;

			}
		else
			new_entry.r_symndx = old_entry.r_symbolnum;
		
		/* figure out the relocation type, size and offset */

		if (old_entry.r_pcrel)
					/* pc relative addressing */
			if (old_entry.r_length == 0)
				new_entry.r_type = R_PCRBYTE;
			else if (old_entry.r_length == 1)
				new_entry.r_type = R_PCRWORD;
			else if (old_entry.r_length == 2)
				new_entry.r_type = R_PCRLONG;
			else { /* uh oh! */
				fprintf(stderr,"convert: relocation error\n");
				abort();
			     }
		else
					/* symbol relative addressing */
			if (old_entry.r_length == 0)
				new_entry.r_type = R_RELBYTE;
			else if (old_entry.r_length == 1)
				new_entry.r_type = R_RELWORD;
			else if (old_entry.r_length == 2)
				new_entry.r_type = R_RELLONG;
			else { /* uh oh! */
				fprintf(stderr,"convert: relocation error\n");
				abort();
			     };

		/* write out the new entry */
		
		if (fwrite(&new_entry,1,RELSZ,tmpfil) != RELSZ)
			writerr(tmpfilname);

		};


	return(reloc_size);
}






long
gen_syms (file, old_header) /* procedure converts old symbol table from the
			       input file into the new symbol table format.
			       the new symbol table is written to the tmpfil
			       file.  the number of bytes in the new symbol
			       table is returned.  note that all debugging
			       (sdb) symbol information (i.e., .stabtype
			       symbols) is deleted from the symbol table
			       because of incompatabilities with the two
			       formats.  as the symbol table is manipulated,
			       symbol indexes may be modified as their 
			       positions are moved about.  this circumstance
			       can screw up the relocation entries.  a patch
			       list of new and old symbol indexes is created
			       so that the relocation entries can be modified
			       in the last stage of processing. */

FILE *file;
struct exec *old_header;

{
	long syms_size;
	int symsin, symsout, nsyms;
	struct onlist old_symbol;
	struct syment new_symbol;	
	long datastart;


	nsyms = old_header->a_syms / sizeof(struct onlist);

	if (nsyms >= MAXSYMS) { /* uh oh! */
		fprintf(stderr,"convert: input symbol table exceeds %d symbols\n",
			MAXSYMS);
		fprintf(stderr,"         raise MAXSYMS parameter\n");
		abort();
	};


	symsin = 0;


	/* generate the artificial .text, .data and .bss
	   section/segment pointer symbols */

	symsout = 3; /* .text, .data and .bss makes three */
	syms_size = 3 * SYMESZ;

	/* create .text symbol */

	strncpy(new_symbol.n_name,_TEXT,SYMNMLEN);
	new_symbol.n_value = 0L; /* .text always begins at zero */
	new_symbol.n_numaux = 0;
	new_symbol.n_type = T_NULL;
	new_symbol.n_scnum = 1;
	new_symbol.n_sclass = C_STAT;

	/* output the .text symbol */

	if (fwrite(&new_symbol,1,SYMESZ,tmpfil) != SYMESZ)
		writerr(tmpfilname);

	/* create .data symbol */

	strncpy(new_symbol.n_name,_DATA,SYMNMLEN);
	new_symbol.n_value = old_header->a_text;
	new_symbol.n_scnum = 2;

	/* output the .data symbol */

	if (fwrite(&new_symbol,1,SYMESZ,tmpfil) != SYMESZ)
		writerr(tmpfilname);

	/* create .bss symbol */

	strncpy(new_symbol.n_name,_BSS,SYMNMLEN);
	new_symbol.n_value += old_header->a_data; /* .bss adjoins */
	new_symbol.n_scnum = 3;

	/* output the .bss symbol */

	if (fwrite(&new_symbol,1,SYMESZ,tmpfil) != SYMESZ)
		writerr(tmpfilname);

	/* compute symbol value adjustment factor */

	datastart = old_header->a_text;
	if (old_header->a_magic == A_MAGIC2) /* read only text */
		if (datastart & 0777) {
			datastart &= (~0777L);
			datastart += 512;
			};

	while (symsin < nsyms) { /* process each input symbol */
		
		/* set patch table information */

		patch[symsin].sym = symsout;
		patch[symsin].adj = NOADJUST; /* assume no adjustment */

		/* read in the old symbol table entry */

		if (fread(&old_symbol,1,sizeof(struct onlist),file) !=
		     sizeof(struct onlist)) 
			readerr(infilname);
		else
			symsin++;

		/* check the symbol type, we shall skip over all file
		   name symbols and sdb .stabtype support symbols */

		if ((old_symbol.n_type < 0) || (old_symbol.n_type > 9))
			continue; /* don't mess with this one */

		/* okay, generate a new symbol equivalent to the old one */

		/* copy the symbol mnemonic */

		strncpy(new_symbol.n_name,old_symbol.n_oname,SYMNMLEN);

		/* copy the symbol value, adjust the .bss and .data
		   symbol values because they had the rounded up value
		   of the .data segment added in instead of the actual
		   value of .data or .bss offsets (only JFR knows
		   why!)   adjustments are performed below */

		new_symbol.n_value = old_symbol.n_value;

		/* we can't generate any aux entries or figure out the
		   type of the symbol */

		new_symbol.n_numaux = 0;
		new_symbol.n_type = T_NULL;

		/* figure out the section numbers and storage class data */

		switch (old_symbol.n_type) {

			case 0:			/* undefined */
				new_symbol.n_scnum = N_UNDEF;
				new_symbol.n_sclass = C_STAT;
				break;

			case 1:			/* undefined external */
				new_symbol.n_scnum = N_UNDEF;
				new_symbol.n_sclass = C_EXT;
				break;

			case 2:			/* absolute */
				new_symbol.n_scnum = N_ABS;
				new_symbol.n_sclass = C_STAT;
				break;

			case 3:			/* absolute external */
				new_symbol.n_scnum = N_ABS;
				new_symbol.n_sclass = C_EXT;
				break;

			case 4:			/* text */
				new_symbol.n_scnum = 1;
				new_symbol.n_sclass = C_STAT;
				break;

			case 5:			/* text external */
				new_symbol.n_scnum = 1;
				new_symbol.n_sclass = C_EXT;
				break;

			case 6:			/* data */
				new_symbol.n_scnum = 2;
				new_symbol.n_sclass = C_STAT;
				/* adjust data symbol value */
				new_symbol.n_value = new_symbol.n_value -
					datastart + old_header->a_text;
				patch[symsin-1].adj = ADJUST;
				break;

			case 7:			/* data external */
				new_symbol.n_scnum = 2;
				new_symbol.n_sclass = C_EXT;
				/* adjust data external symbol value */
				new_symbol.n_value = new_symbol.n_value -
					datastart + old_header->a_text;
				patch[symsin-1].adj = ADJUST;
				break;

			case 8:			/* bss */
				new_symbol.n_scnum = 3;
				new_symbol.n_sclass = C_STAT;
				/* adjust bss symbol value */
				new_symbol.n_value = new_symbol.n_value -
					datastart + old_header->a_text;
				patch[symsin-1].adj = ADJUST;
				break;

			case 9:			/* bss external */
				new_symbol.n_scnum = 3;
				new_symbol.n_sclass = C_EXT;
				/* adjust bss external symbol value */
				new_symbol.n_value = new_symbol.n_value -
					datastart + old_header->a_text;
				patch[symsin-1].adj = ADJUST;
				break;

			default: /* uh oh! */
				fprintf(stderr,
				   "convert: internal symbol table botch\n");
				abort();
		};

		/* output the new and equivalent symbol */

		if (fwrite(&new_symbol,1,SYMESZ,tmpfil) != SYMESZ)
			writerr(tmpfilname);
		    
		symsout++;
		syms_size += SYMESZ;
	};


	return(syms_size);
}






patch_reloc (pfile, nsyms, old_header)
		       /* procedure patches all relocation entry indexes
		          according to the global patch list array which
		          indicates old and new symbol table indexes.  the input
		          file pfile is modified and returned as output with
		          the relocation patches intact.  the size of pfile is
		          not altered by this procedure.  all .data section
			  relocatable locations are adjusted to compensate
			  for extra values added in by the pre-5.0 vax
			  assembler.  (see gen_syms above) */

FILE *pfile;
long nsyms;
struct exec *old_header;

{
	struct filehdr fhead;
	struct scnhdr  shead;
	struct reloc   rentry;
	long   nreloc, ndreloc, relptr, rewptr, dataptr, adjust;
	char doadjust;

	long ldata;
	short sdata;
	char bdata;

	fclose(pfile);
	if ((pfile = fopen(tmpfilname,"r+")) == NULL) /* open for update */
		readerr(tmpfilname);

	/* update the number of symbols data in the file header */

	if (fread(&fhead,1,sizeof(struct filehdr),pfile) != 
		sizeof(struct filehdr)) readerr(tmpfilname);

	fhead.f_nsyms = nsyms;
	
	/* update file header */
	
	fseek(pfile,0L,0);
	if (fwrite(&fhead,1,sizeof(struct filehdr),pfile) !=
		sizeof(struct filehdr)) writerr(tmpfilname);

	/* read text and data section headers so that we can get the
	   location and number of all relocation entries */

	fseek(pfile,(long)(fhead.f_opthdr+sizeof(struct filehdr)),0);

	/* read text section header to get the pointer to the beginning
	   of the relocation entry blocks and the quantity of text
	   relocation entries */
	
	if (fread(&shead,1,sizeof(struct scnhdr),pfile) !=
		sizeof(struct scnhdr)) readerr(tmpfilname);

	relptr = shead.s_relptr;
	nreloc = shead.s_nreloc;

	/* read data section header to get the quantity of data relocation
	   entries */

	if (fread(&shead,1,sizeof(struct scnhdr),pfile) !=
		sizeof(struct scnhdr)) readerr(tmpfilname);

	if (relptr == 0L) /* there were no text relocations */
		relptr = shead.s_relptr;
	nreloc += shead.s_nreloc;

	/* get data section pointer and compute data adjustment factor */

	ndreloc = shead.s_nreloc;
	dataptr = shead.s_scnptr - old_header->a_text;
	adjust = old_header->a_text;
	if (old_header->a_magic == A_MAGIC2) /* read only text */
		if (adjust & 0777) {
			adjust &= (~0777L);
			adjust += 512;
			};
	adjust -= old_header->a_text;

	/* seek to the beginning of the relocation entry block and update
	   all relocation entry symbol table pointers according to the
	   patch table */

	fseek(pfile,relptr,0);

	while (nreloc--) { /* update each relocation entry in turn */

		/* read in a relocation table entry */

		if (fread(&rentry,1,RELSZ,pfile) != RELSZ)
			readerr(tmpfilname);

		/* determine if relocatable data adjustment is necessary */

		if (rentry.r_symndx >= 0L)
			doadjust = patch[rentry.r_symndx].adj;
		else if (rentry.r_symndx == -6L || rentry.r_symndx == -8L)
			doadjust = ADJUST;
		else
			doadjust = NOADJUST;

		/* update the ptr value */

		if (rentry.r_symndx >= 0L)
			rentry.r_symndx = patch[rentry.r_symndx].sym;
		else if (rentry.r_symndx == -4L) /* relocate off .text symbol */
			rentry.r_symndx = 0L;
		else if (rentry.r_symndx == -6L) /* relocate off .data symbol */
			rentry.r_symndx = 1L;
		else if (rentry.r_symndx == -8L) /* relocate off .bss symbol */
			rentry.r_symndx = 2L;
	
		/* remember where to jump back to update the relocation entry */

		rewptr = ftell(pfile) - RELSZ;

		/* is this a relocation entry that needs adjustment */

		if (doadjust == ADJUST) { /* yes it is, update the raw data */
			long dataloc;

			/* get to the address of the data entity */
			dataloc = dataptr + rentry.r_vaddr;
			fseek(pfile,dataloc,0);

			/* read in the appropriate nbr of bytes, adjust the
			   value and then write out the appropriate nbr of
			   bytes */

			switch (rentry.r_type) {
			
				case R_PCRBYTE:
				case R_RELBYTE:
					fread(&bdata,1,1,pfile);
					bdata -= adjust;
					fseek(pfile,dataloc,0);
					fwrite(&bdata,1,1,pfile);
					break;

				case R_PCRWORD:
				case R_RELWORD:
					fread(&sdata,1,2,pfile);
					sdata -= adjust;
					fseek(pfile,dataloc,0);
					fwrite(&sdata,1,2,pfile);
					break;

				case R_PCRLONG:
				case R_RELLONG:
					fread(&ldata,1,4,pfile);
					ldata -= adjust;
					fseek(pfile,dataloc,0);
					fwrite(&ldata,1,4,pfile);
					break;
			};
		};

		/* put the relocation entry back into the file */

		fseek(pfile,rewptr,0); /* jump back jack */

		if (fwrite(&rentry,1,RELSZ,pfile) != RELSZ)
			writerr(tmpfilname);

		/* reset the file pointer */

		fseek(pfile,0L,1);
	};

}
