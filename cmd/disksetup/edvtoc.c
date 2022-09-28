/*	Copyright (c) 1990 UNIX System Laboratories, Inc.	*/
/*	Copyright (c) 1984, 1986, 1987, 1988, 1989, 1990 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF     	*/
/*	UNIX System Laboratories, Inc.                     	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)disksetup:edvtoc.c	1.3.1.1"

#include <stdio.h>
#include <fcntl.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/vtoc.h>
#include <sys/stat.h>
#include <sys/alttbl.h>

char    *devname;		/* name of device */
int	devfd = 0;		/* device file descriptor */
FILE	*vtocfile;		/* file containing new vtoc info */
struct  disk_parms   dp;        /* device parameters */
struct  pdinfo	pdinfo;		/* physical device info area */
struct  vtoc	ovtoc;		/* current virt. table of contents */
struct  vtoc	nvtoc;		/* new virt. table of contents */
char    *buf;			/* buffer used to read in disk structs. */
struct  absio absio;
char    errstring[15] = "EDVTOC error: ";

void
main(argc, argv)
int	argc;
char	*argv[];
{
	static char     options[] = "f:";
	extern int	optind;
	extern char	*optarg;
	struct stat statbuf;
	int c, i;

	if (argc < 3) {
		giveusage();
		exit(1);
	}
	while ( (c=getopt(argc, argv, options)) != EOF ) {
		switch (c) {
		case 'f':
			if ((vtocfile = fopen(optarg, "r")) == NULL) {
				fprintf(stderr, "Edvtoc: unable to open %s file.\n",optarg);
				exit(40);
			}
			break;
		default:
			fprintf(stderr,"Invalid option '%s'\n",argv[optind]);
			giveusage();
			exit(1);
		}
	}

		/* get the last argument -- device stanza */
	if (argc != optind+1) {
		fclose(vtocfile);
		fprintf(stderr,"Missing disk device name\n");
		giveusage();
		exit(1);
	}
	devname = argv[optind];
	if (stat(devname, &statbuf)) {
		fclose(vtocfile);
		fprintf(stderr,"edvtoc stat of %s failed",devname);
		perror(errstring);
		exit(1);
	}
	if ((statbuf.st_mode & S_IFMT) != S_IFCHR) {
		fclose(vtocfile);
		fprintf(stderr,"edvtoc: device %s is not character special",devname);
		giveusage();
		exit(1);
	}
	if ((devfd=open(devname, O_RDWR)) == -1) {
		fprintf(stderr,"edvtoc: open of %s failed\n", devname);
		syserror(2);
	}

	if (ioctl(devfd, V_GETPARMS, &dp) == -1) {
		fprintf(stderr,"edvtoc: GETPARMS on %s failed\n", devname);
		syserror(2);
	}

	if ((buf=(char *)malloc(dp.dp_secsiz)) == NULL) {
		fprintf(stderr,"edvtoc: cannot malloc space.\n");
		syserror(3);
	}

	absio.abs_sec = dp.dp_pstartsec + VTOC_SEC;
	absio.abs_buf = buf;
	if (ioctl(devfd, V_RDABS, &absio) == -1) {
		fprintf(stderr,"edvtoc: reading pdinfo failed\n");
		syserror(4);
	}
	memcpy((char *)&pdinfo, absio.abs_buf, sizeof(pdinfo));
	if ((pdinfo.sanity != VALID_PD) || (pdinfo.version != V_VERSION)) {
		fprintf(stderr,"edvtoc: invalid pdinfo block found.\n");
		fclose(vtocfile);
		close(devfd);
		exit(5);
	}
	memcpy((char *)&ovtoc,&buf[pdinfo.vtoc_ptr%dp.dp_secsiz],sizeof(ovtoc));
	if ((ovtoc.v_sanity != VTOC_SANE) || (ovtoc.v_version != V_VERSION)) {
		fprintf(stderr,"edvtoc: invalid VTOC found.\n");
		fclose(vtocfile);
		close(devfd);
		exit(6);
	}
	memcpy((char *)&nvtoc, (char *)&ovtoc, sizeof(ovtoc));
	readvtoc();
	if (chose_new_vtoc()) {
		set_timestamps();
 		*((struct pdinfo *)buf) = pdinfo;
 		*((struct vtoc *)&buf[pdinfo.vtoc_ptr%dp.dp_secsiz]) = nvtoc;
		absio.abs_sec = dp.dp_pstartsec + VTOC_SEC;
		absio.abs_buf = buf;
		if (ioctl(devfd, V_WRABS, &absio) == -1) {
			fprintf(stderr,"edvtoc: writing vtoc failed\n");
			perror(errstring);
		}
	}
	else
		printf("New VTOC will not be written to the disk.\n");

	close(devfd);
	exit(0);
}

syserror(exitval)
int exitval;
{
	fclose(vtocfile);
	if (devfd > 0)
		close(devfd);
	perror(errstring);
	exit();
}

/* set timestamps of slices which have changed to 0, so if mirror driver present */
/* the mirror slice will be updated.						 */
set_timestamps()
{
	int i;

	for (i = 0; i < V_NUMPAR; i++) 
		if ((nvtoc.v_part[i].p_size != ovtoc.v_part[i].p_size) ||
		   (nvtoc.v_part[i].p_start != ovtoc.v_part[i].p_start))
			nvtoc.timestamp[i] = 0;
}

/*  Printslice prints a single slice entry.  */
printslice(v_p)
struct partition *v_p;
{
	printf("tag: ");
	switch(v_p->p_tag) {
	case V_BOOT:	printf("BOOT ");			break;
	case V_ROOT:	printf("ROOT  ");			break;
	case V_SWAP:	printf("SWAP  ");			break;
	case V_USR:	printf("USER  ");			break;
	case V_BACKUP:	printf("DISK  ");			break;
	case V_STAND:	printf("STAND  ");			break;
	case V_HOME:	printf("HOME  ");			break;
	case V_DUMP:	printf("DUMP  ");			break;
	case V_VAR:	printf("VAR  ");			break;
	case V_ALTS:	printf("ALTSECTS  ");			break;
	case V_ALTTRK:	printf("ALTTRKS  ");			break;
	case V_OTHER:	printf("DOS  ");			break;
	default:	if (v_p->p_tag == 0)
				printf("EMPTY ");
			else
				printf("other 0x%x  ",v_p->p_tag);	
			break;
	}

	printf("perms: ");
	if (v_p->p_flag & V_VALID)	printf("VALID ");
	if (v_p->p_flag & V_UNMNT)	printf("UNMOUNTABLE ");
	if (v_p->p_flag & V_RONLY)	printf("READ ONLY ");
	if (v_p->p_flag & V_OPEN)	printf("(driver open) ");
	if (v_p->p_flag & ~(V_VALID|V_OPEN|V_RONLY|V_UNMNT))
					printf("flag: 0x%x",v_p->p_flag);
	printf("  start: %ld  length: %ld\n",
		v_p->p_start, v_p->p_size);
}

giveusage()
{
	fprintf(stderr,"edvtoc -f vtoc-file raw-device\n");
}

int 
yes_response()
{
	for (;;) {
		gets(buf);
		if (buf[0] == 'y' || buf[0] == 'Y') 
			return 1;
		if (buf[0] == 'n' || buf[0] == 'N') 
			return 0;
		printf("\nInvalid response - please answer with y or n.");
	}
}

			
readvtoc()
{
	int i, n, slice, tag, flag;
	daddr_t size, start;
	daddr_t numsects, availsects, availcyls;

	for (i=1; (fgets(&buf[0],512,vtocfile) != NULL); i++) {
		if (buf[0] == '#')
			continue;
		n = sscanf(&buf[0],"%d 0x%x 0x%x %ld %ld",&slice,&tag,&flag,
			&start, &size); 
		if ((n < 5) || (slice < 0) || (slice >= V_NUMPAR)) {
			fprintf(stderr,"edvtoc: vtocfile line %d is invalid.\n",i);
			fclose(vtocfile);
			close(devfd);
			exit(1);
		}
		if (size > 0)
			nvtoc.v_nparts = i+1;
		nvtoc.v_part[slice].p_start = start;
		nvtoc.v_part[slice].p_size = size;
		nvtoc.v_part[slice].p_flag = flag;
		nvtoc.v_part[slice].p_tag = tag;
	}
	fclose(vtocfile);
}

int 
chose_new_vtoc()
{
	int i;

	printf("The following slices are the new disk configuration you have\n");
	printf("created. NO ERROR or VALIDITY checking has been done on it.\n");
	for(i=0; i < V_NUMPAR; i++) {
		if (nvtoc.v_part[i].p_size > 0) {
			printf("slice %d: ",i);
			printslice(&nvtoc.v_part[i]);
		}
	}
	printf("Is this configuration the VTOC you want written to %s? (y/n) ",devname);
	if (yes_response())
		return(1);
	else
		return(0);
}
