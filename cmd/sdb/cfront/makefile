#	Copyright (c) 1990 UNIX System Laboratories, Inc.
#	Copyright (c) 1988 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF
#	UNIX System Laboratories, Inc.
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)sdb:cfront/makefile	1.5"
#Makefile for the CC translator
#	BSD 4.2 or later should first run bsd.fix; this will set the 
#	shell variables:
#	CCFLAGS=-O -DBSD
#	BSD=1

CCFLAGS=
BSD=

#For first make (bootstrap):
#	make scratch
#Otherwise:
#	make
#Additional: when make is complete:
#       make clean - removes all .o files generated in these procedures
#####################

CC	=	xCC

all:	libC.a munch cfront
	:

libC.a:	
	cd lib/mk; $(MAKE) CC=../xCC CCFLAGS="$(CCFLAGS)" BSD=$(BSD)
	mv lib/mk/libC.a .

munch:	always
	-@chmod +x xCC
	./xCC $(CCFLAGS) -o munch lib/static/munch.c
	rm munch.o

cfront:	always
	cd src; $(MAKE) CCC=../xCC CCFLAGS="$(CCFLAGS)"
	mv src/cfront cfront
	
scratch: always
	cd scratch; $(MAKE) CC=cc AR=ar NM=nm BSD=$(BSD) CCFLAGS="$(CCFLAGS)"

clean:	
	rm -f scratch/*..o
	rm -f lib/mk/*.o
	rm -f src/*.o
	rm -f src/y.tab.c

# create a simple test case that will require invocation of static constructors
test:
	echo "#include <stream.h>" > ctor_test.c
	echo "main() {" >> ctor_test.c
	echo "cout << \"hello, world\\\n\"; " >> ctor_test.c
	echo "exit( 0 ); }" >> ctor_test.c

	echo "if test \`type CC | grep -c \"not found\"\` -ne 0" > test.sh 
	echo "then" >> test.sh 
	echo "echo \"?? make test: cannot find CC: check PATH ::\"" >> test.sh 
	echo "exit 1; fi" >> test.sh

	echo "if test -f testcase; then rm testcase; fi" >> test.sh
	echo "CC -o testcase ctor_test.c > test.diag 2>&1" >> test.sh
	echo "if test ! -f testcase" >> test.sh
	echo "then" >> test.sh
	echo "echo \"?? make test: compile failed: see test.diag ::\"" >> test.sh
	echo "echo \":: see Common Problems in Release Notes ::\"" >> test.sh
	echo "exit 1; fi" >> test.sh

	echo "rm test.diag ctor_test.*" >> test.sh
	echo "testcase > test.run 2>&1" >> test.sh
	echo "if test \`fgrep -c \"hello, world\" test.run\` -eq 1" >> test.sh 
	echo "then" >> test.sh 
	echo "echo \"!! make test: pass: static constructors ok ::\"" >> test.sh 
	echo "rm testcase test.run; exit 0" >> test.sh

	echo "else" >> test.sh
	echo "echo \"?? make test: execution failed: see test.run ::\"" >> test.sh
	echo "if test \`nm testcase | grep -c STI\` -eq 0" >> test.sh 
	echo "then" >> test.sh
	echo "echo \":: make test: munch or patch not working ::\"" >> test.sh

	echo "else" >> test.sh
	echo "if test \`nm testcase | grep -c LINK\` -eq 0" >> test.sh
	echo "then" >> test.sh
	echo "echo \":: make test: patch not working ::\"" >> test.sh
	echo "fi" >> test.sh
	echo "fi" >> test.sh

	echo "echo \":: see Common Problems in Release Notes ::\"" >> test.sh
	echo "fi" >> test.sh
	echo "exit 1" >> test.sh

	chmod +x test.sh 
	test.sh
	rm test.sh 

#This target will populate the scratch directories with good-old-c
#files.  This is used to port to another machine.
#Be sure to set CC on the command line!

fillscratch:
#	make sure the directories exist:
	if test ! -d scratch/src; then mkdir scratch/src; fi
	if test ! -d scratch/mnch; then mkdir scratch/mnch; fi
	if test ! -d scratch/lib; then mkdir scratch/lib; fi
	cd src; yacc gram.y
	cd scratch/src; $(CC) -I../../src -I../../incl -Fc -..c ../../src/*.c;
	cd scratch/lib; $(CC) -I../../lib/complex \
		 -I../../incl -Fc -..c ../../lib/new/*.c
	cd scratch/lib; $(CC) -I../../lib/complex \
		 -I../../incl -Fc -..c ../../lib/static/*.c
	rm scratch/lib/munch..c
#Dont need a real munch here:
	echo "main(){ exit(0); }" >scratch/mnch/munch..c
	-chmod +x xCC scratch/bsd.sed bsd.fix

always:	
