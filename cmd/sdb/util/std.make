#ident	"@(#)sdb:util/std.make	1.5"

include ../../util/defs.make
include	../../util/CC.rules

SOURCES = $(CSOURCES) $(CCSOURCES)

clean:
	-rm -f *.o y.* lex.yy.c

clobber:	clean
	rm -f $(TARGET)

basedepend:
	rm -f BASEDEPEND OBJECT.list
	@if [ "$(CCSOURCES)" ] ;\
		then echo "	../../util/depend $(CPLUS_CMD_FLAGS) $(CCSOURCES) >> BASEDEPEND" ; \
		CC=$(CC) ../../util/depend $(CPLUS_CMD_FLAGS) $(CCSOURCES) >> BASEDEPEND ; \
	fi
	@if [ "$(CSOURCES)" ] ;\
		then echo "	../../util/depend $(CC_CMD_FLAGS) $(CSOURCES) >> BASEDEPEND" ; \
		CC=$(CC) ../../util/depend $(CC_CMD_FLAGS) $(CSOURCES) >> BASEDEPEND ; \
	fi
	chmod 666 BASEDEPEND

depend:	basedepend
	rm -f DEPEND
	cat BASEDEPEND | \
		../../util/substdir $(PRODINC) '$$(PRODINC)' | \
		../../util/substdir $(SGSBASE) '$$(SGSBASE)' | \
		../../util/substdir $(INCC) '$$(INCC)' | \
		../../util/substdir $(INC) '$$(INC)' > DEPEND
	../../util/mkdefine OBJECTS < OBJECT.list >> DEPEND
	chmod 444 DEPEND
	rm -f BASEDEPEND

rebuild:	clobber depend all
