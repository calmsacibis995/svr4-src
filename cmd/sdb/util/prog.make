#ident	"@(#)sdb:util/prog.make	1.8"

all:	$(TARGET)

$(TARGET):	$(OBJECTS)
	rm -f $(TARGET)
	$(CPLUS) -o $(TARGET) $(LINK_MODE) $(OBJECTS) $(LIBRARIES)

install:	$(CCSBIN)/$(BASENAME)

$(CCSBIN)/$(BASENAME):	$(TARGET)
	$(STRIP) $(TARGET)
	cp $(TARGET) $(CCSBIN)/$(BASENAME)
