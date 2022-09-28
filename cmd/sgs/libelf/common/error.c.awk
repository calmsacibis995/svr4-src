#ident	"@(#)libelf:common/error.c.awk	1.2"

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
		print "#ifdef __STDC__"
		print "	#pragma weak	elf_errmsg = _elf_errmsg"
		print "	#pragma weak	elf_errno = _elf_errno"
		print "#endif\n\n"
		print "#include \"syn.h\""
		print "#include \"error.h\"\n\n"
		print "#ifndef __STDC__"
		print "#	define const"
		print "#endif\n\n"
		print "extern int	_elf_err;"
	
		for (j = 1; j <= major; ++j)
		{
			print
			print
			printf "static const char *const\t%s[] =\t/* E%s_... */\n",\
				info[j MAJOR_DATA],\
				info[j MAJOR_ENUM]
			print "{"
x = j MAJOR_PREF
			printf "\t/* 0 */\t\t\"%s: reason unknown\",\n", info[x]
			for (k = 1; k <= info[j MAJOR_NUMB]; ++k)
			{
				printf "\t/* %s */\t\"%s: %s\",\n",\
					info[j MAJOR_MINOR k MINOR_ENUM],\
					info[j MAJOR_PREF],\
					info[j MAJOR_MINOR k MINOR_TEXT] 
			}
			print "};"
		}

		print
		print
		print "static const struct\tMsg"
		print "{"
		print "	const char *const	*m_list;\t/* message list */"
		print "	int			m_num;\t\t/* # entries */"
		print "} msg[] =\n{"
		for (j = 1; j <= major; ++j)
		{
			printf "\t/* E%s */\t{ %s, sizeof(%s) / sizeof(%s[0]) },\n",\
				info[j MAJOR_ENUM],\
				info[j MAJOR_DATA],\
				info[j MAJOR_DATA],\
				info[j MAJOR_DATA]
		}
		print "};"
		print
		print
		print "const char *"
		print "elf_errmsg(err)"
		print "	int err;"
		print "{"
		print "	int	major, minor;"
		print
		print "	if (err == 0)"
		print "	{"
		print "		if ((err = _elf_err) == 0)"
		print "			return 0;"
		print "	}"
		print "	else if (err == -1)"
		print "	{"
		print "		if ((err = _elf_err) == 0)"
		print "			return \"ELF error 0\";"
		print "	}"
		print "	major = errmaj(err);"
		print "	minor = errmin(err);"
		print "	if (major < sizeof(msg) / sizeof(msg[0]))"
		print "	{"
		print "		if (minor >= msg[major].m_num)"
		print "			minor = 0;"
		print "		return msg[major].m_list[minor];"
		print "	}"
		print "	return \"Unknown ELF error\";"
		print "}"
		print
		print
		print "int"
		print "elf_errno()"
		print "{"
		print "	int temp;"
		print
		print "	temp =_elf_err;"
		print "	_elf_err = 0;"
		print "	return temp;"
		print "}"
	}
