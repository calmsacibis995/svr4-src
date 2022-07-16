#ident	"@(#)rtld:i386/genset.awk	1.5"

# Create rtsetaddr.s or rtabiaddr.s.
# _rt_setaddr() looks up the values of several special symbols that may be
# found in either the user's a.out or ld.so.  If the special symbol is in the
# a.out, set the address in the GOT, else do nothing.  This is in assembler
# because we have to access the GOT directly.
# The list of symbols is in genset.in.  All symbols that must appear in
# rtabiaddr.s have "MIN" as $2.

BEGIN	{
	error = 0
	mode = 0	# 1: full libc, 2: min abi
	count = 0
}

$1 ~ /^@.*/	{
	# process an @ directive
	# @FULL: full libc
	# @MIN: minimal abi libc
	if (mode != 0) {
		printf "genset.awk: too many @ directives\n" | "cat -u >&2"
		error = 1
		exit
	}

	if ($1 == "@FULL") {
		printf "\t.file\t\"rtsetaddr.s\"\n"
		mode = 1
	} else if ($1 == "@MIN") {
		printf "\t.file\t\"rtabiaddr.s\"\n"
		mode = 2
	} else {
		printf "genset.awk: illegal mode " \
			"(@MIN: min abi, @FULL: full libc)\n" | \
			"cat -u >&2"
		error = 1
		exit
	}

	printf "\n\t.globl\t_rt_setaddr\n"
	printf "\t.text\n\t.type\t_rt_setaddr,@function\n\t.align\t4\n"
	printf "_rt_setaddr:\n"
	printf "\tpushl\t%%ebp\n"
	printf "\tmovl\t%%esp,%%ebp\n"
	printf "\tpushl\t%%eax\n"
	printf "\tpushl\t%%ebx\n"
	printf "\tcall\t.L1\n"
	printf ".L1:\n"
	printf "\tpopl\t%%ebx\n"
	printf "\taddl\t$_GLOBAL_OFFSET_TABLE_+[.-.L1],%%ebx\n"
	printf "\n"
	next
}

$0 ~ /^#.*/	{
	# skip comments
	next
}

NF == 0	{
	# skip blank lines
	next
}

{
	if (mode == 0) {
		printf "genset.awk: @ directive not set\n" | "cat -u >&2"
		error = 1
		exit
	}
	if (mode == 2 && ($2 != "MIN" && $2 != "PRE"))
		next
	# put out a .globl and a .string for the symbol
	if (mode == 2 && $2 == "MIN" && $3 != "") {
		printf "\t.globl\t%s\n", $3 $1
	} else if ($2 == "PRE") {
		printf "\t.globl\t%s\n", $3 $1
		printf "\t.globl\t%s\n", $1
	} else {
		printf "\t.globl\t%s\n", $1
	}
	printf "\t.section\t.rodata\n\t.align\t4\n"
	printf ".X%.3d:\t/ %s\n\t.string\t\"%s\"\n", count, $1, $1

	# put out the code to look up the symbol and fix up the GOT.
	printf "\t.text\n"
	printf "/ sym = _lookup(\"%s\", LO_ALL, _ld_loaded, &lm, LOOKUP_NORM);\n", $1
	printf ".SYM%.3d:\n", count
	printf "\tpushl\t$0\n"
	printf "\tleal\t-4(%%ebp),%%eax\n"
	printf "\tpushl\t%%eax\n"
	printf "\tmovl\t_ld_loaded@GOT(%%ebx),%%eax\n"
	printf "\tpushl\t(%%eax)\n"
	printf "\tpushl\t$0\n"
	printf "\tleal\t.X%.3d@GOTOFF(%%ebx),%%eax\n", count
	printf "\tpushl\t%%eax\n"
	printf "\tcall\t_lookup@PLT\n"
	printf "\taddl\t$20,%%esp\n"
	printf "/ if (sym)\n"
	if (mode == 2 && $2 == "MIN" && $3 != "") {
		printf "/    %s@GOT = sym->st_value + (NAME(lm) ? " \
			"ADDR(lm) : 0 );\n", $3 $1
	} else if ($2 == "PRE") {
		printf "/    %s@GOT = sym->st_value + (NAME(lm) ? " \
			"ADDR(lm) : 0 );\n", $3 $1
	} else {
		printf "/    %s@GOT = sym->st_value + (NAME(lm) ? " \
			"ADDR(lm) : 0 );\n", $1
	}
	printf "\tcmpl\t$0,%%eax\n"
	printf "\tjz\t.SYM%.3d\n", count + 1
	printf "\tmovl\t-4(%%ebp),%%ecx\n"
	printf "\tcmpl\t$0,4(%%ecx)\n"
	printf "\tjz\t.J1%.3d\n", count
	printf "\tmovl\t0(%%ecx),%%edx\n"
	printf "\tjmp\t.J2%.3d\n", count
	printf ".J1%.3d:\n", count
	printf "\tmovl\t$0,%%edx\n"
	printf ".J2%.3d:\n", count
	printf "\taddl\t4(%%eax),%%edx\n"
	if (mode == 2 && $2 == "MIN" && $3 != "") {
		printf "\tmovl\t%%edx,%s@GOT(%%ebx)\n", $3 $1
	} else if ($2 == "PRE") {
		printf "\tmovl\t%s@GOT(%%ebx),%%eax\n", $3 $1
		printf "\tmovl\t%%edx,(%%eax)\n"
	} else {
		printf "\tmovl\t%%edx,%s@GOT(%%ebx)\n", $1
	}
	count++
}

END	{
	if (error == 1)
		exit

	printf ".SYM%.3d:\n", count

	if (mode == 1) {	# only for full libc
		# initialize the __first_list in stdio, this has to be doen
		# after fixing up the GOT entries, so that it has the address
		# of __iob from either the user's code, or the library,
		# as appropriate

		printf "/ Initialize some values ....\n"
		printf "/ Set up __first_link used in port/stdio/flush.c\n"
		printf "/ struct Link __first_link = { &_iob[0], ... }\n"
		printf "\tmovl\t__first_link@GOT(%%ebx),%%eax\n"
		printf "\tmovl\t__iob@GOT(%%ebx),%%edx\n"
		printf "\tmovl\t%%edx,0(%%eax)\n"
	}

	printf "\tpopl\t%%ebx\n"
	printf "\tleave\n"
	printf "\tret\n"
	printf "\t.size\t_rt_setaddr,.-_rt_setaddr\n"
	if (mode == 2) printf "\t.weak\t_cleanup\n"
}
