#ident	"@(#)face:src/oam/programs/Menu.mod	1.3"

menu="Modify Global Programs"

framemsg="Move the cursor to the item you want and press ENTER to select it."

help=open TEXT $INTFBASE/Text.itemhelp 'menu:F1'

`fmlgrep '^vmsys:' /etc/passwd | fmlcut -f6 -d: | set -e VMSYS;
$VMSYS/bin/listserv -m VMSYS`
