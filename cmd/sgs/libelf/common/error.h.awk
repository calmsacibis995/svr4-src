#ident	"@(#)libelf:common/error.h.awk	1.2"

# Convert error description file to C.
# Input has the following form (fields separated by tabs):
#
#	12345678901234567890...
#	# comment
#	#ident	"string"
#
#	MAJOR_ENUM	MAJOR_DATA	Error prefix
#		MINOR_ENUM	message text
#
# Example
#
#	
#	BUG	bug	Internal error
#		BUG1	Goofed up data structure
#		BUG2	Messed up pointer
#
#	FMT	fmt	Format error
#		...

BEGIN	{
		ident = ""
		major = "_0"
		FS = "\t"
		MAJOR_ENUM = "_0"
		MAJOR_DATA = "_1"
		MAJOR_NUMB = "_2"
		MAJOR_PREF = "_3"
		MAJOR_MINOR = "_4"
		MINOR_ENUM = "_0"
		MINOR_TEXT = "_1"
	}

/^#/	{}	# do nothing
/^$/	{}	# do nothing

/^#ident/	{
		ident = $0
	}

/^[A-Za-z]/	{
		++major
		info[major MAJOR_ENUM] = $1
		info[major MAJOR_DATA] = $2
		info[major MAJOR_PREF] = $3
		info[major MAJOR_NUMB] = 0
	}

/^	[A-Za-z]/ {
		j = ++info[major MAJOR_NUMB];
		info[major MAJOR_MINOR j MINOR_ENUM] = $2
		info[major MAJOR_MINOR j MINOR_TEXT] = $3
	}

END	{
		if (ident != "")
			print ident, "\n\n"
		print "#define errbld(M,m)	(((int)(M)<<8)+(int)(m))"
		print "#define errmaj(e)	((unsigned)(e)>>8)"
		print "#define errmin(e)	((unsigned)(e)&0xff)"

		print "\n\ntypedef enum\n{"
		for (j = 1; j <= major; ++j)
		{
			printf "\tE%s,\n", info[j MAJOR_ENUM]
		}
		printf "\tELast\n} Major;\n"

		for (j = 1; j <= major; ++j)
		{
			print
			print
			print "typedef enum\n{"
			printf "\tE%s_0 = errbld(E%s, 0),\n",\
				info[j MAJOR_ENUM], info[j MAJOR_ENUM]
			for (k = 1; k <= info[j MAJOR_NUMB]; ++k)
			{
				printf "\tE%s_%s,\t\t/* %s */\n",\
					info[j MAJOR_ENUM],\
					info[j MAJOR_MINOR k MINOR_ENUM],\
					info[j MAJOR_MINOR k MINOR_TEXT] 
			}
			printf "\tE%s_Last\n} E%s;\n",\
				info[j MAJOR_ENUM],\
				info[j MAJOR_DATA]
		}
	}
