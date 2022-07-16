#ident	"@(#)qt:uface/Menu.tape	1.1"
menu="Tape Operations"

`message "Move to an item with the arrow keys and strike the ENTER key to select"`

name=Erase
action=open text $VMSYS/OBJECTS/tape/Text.tape '-e' 'erased' 'Erase' \
' Warning: This will erase whatever is currently
 on the cartridge tape.
'
itemmsg="Move to an item with the arrow keys and strike the ENTER key to select"

name=Retension
action=open text $VMSYS/OBJECTS/tape/Text.tape '-t' 'retensioned' 'Retension' \
' Notice: This operation will take a few minutes
 to complete.
'
itemmsg="Move to an item with the arrow keys and strike the ENTER key to select"

name=Rewind
action=open text $VMSYS/OBJECTS/tape/Text.tape '-w' 'rewound' 'Rewind'
itemmsg="Move to an item with the arrow keys and strike the ENTER key to select"

name=Reset
action=open text $VMSYS/OBJECTS/tape/Text.tape '-r' 'reset' 'Reset'
itemmsg="Move to an item with the arrow keys and strike the ENTER key to select"
