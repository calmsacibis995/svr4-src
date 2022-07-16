#ident	"@(#)libc-port:gen/errlist.awk	1.1"

# create two files from a list of input strings,
# new_list.c contains an array of characters indexed into by perror and strerror,
# errlst.c contains an array of pointers to strings, for compatibility
# with existing user programs that reference it directly

BEGIN	{
		FS = "\t"
		hi = 0

		newfile = "new_list.c"
		oldfile = "errlst.c"

		print "#ident\t\"@(#)libc-port:gen/errlist.awk	1.1\"\n" >oldfile
		print "/*LINTLIBRARY*/" >oldfile
		print "#ifdef __STDC__" >oldfile
		print "\t#pragma weak sys_errlist = _sys_errlist" >oldfile
		print "\t#pragma weak sys_nerr = _sys_nerr" >oldfile
		print "#endif" >oldfile
		print "#include \"synonyms.h\"\n" >oldfile
		print "const char * const sys_errlist[] = {" >oldfile

		print "#ident\t\"@(#)libc-port:gen/errlist.awk	1.1\"\n" >newfile
		print "/*LINTLIBRARY*/" >newfile
		print "#include \"synonyms.h\"\n" >newfile
	}

/^[0-9]+/ {
		if ($1 > hi)
			hi = $1
		astr[$1] = $2
	}

END	{
		print "const int _sys_index[] =\n{" >newfile
		k = 0
		for (j = 0; j <= hi; ++j)
		{
			if (astr[j] == "")
				astr[j] = sprintf("Error %d", j)
			printf "\t%d,\n", k >newfile
			k += length(astr[j]) + 1
		}
		print "};\n" >newfile

		print "const char _sys_errs[] =\n{" >newfile
		for (j = 0; j <= hi; ++j)
		{
			print "\t\"" astr[j] "\"," >oldfile
			printf "\t" >newfile
			n = length(astr[j])
			for (k = 1; k <= n; ++k)
				printf "'%s',", substr(astr[j],k,1) >newfile
			print "'\\0'," >newfile
		}
		print "};\n" >newfile
		print "};\n" >oldfile

		print "const int _sys_num_err = " hi + 1 ";" >newfile
		print "const int sys_nerr = " hi + 1 ";" >oldfile
	}
