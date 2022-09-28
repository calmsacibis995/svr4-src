#ident	"@(#)sdb:util/lib.make	1.3"

$(TARGET):	$(OBJECTS)
	rm -f $(TARGET)
	$(AR) -qc $(TARGET) $(OBJECTS)
	@if [ $(MACH) = sparc ] ; then echo "\tranlib $(TARGET)" ;\
		ranlib $(TARGET) ; fi
	chmod 664 $(TARGET)

all:	$(TARGET)

install:	all
