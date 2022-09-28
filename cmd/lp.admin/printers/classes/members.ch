#ident	"@(#)lp.admin:printers/classes/members.ch	1.1"
menu=Printers

lifetime=shortterm

multiselect
init=`cocreate -i lpdata -R printers.ch -e "-EOT-" $SPOOLDIR/bin/lpdata -f $datafile`

done=CLOSE

close=`
    getitems "," | regex
	',all$'		'all'
	'(.*)$0'	'$m0'
    | set -l Form_Choice;
    unset -l all -l x;
    codestroy -R printers.ch lpdata;
`

`
    cosend lpdata "get_class/n$ARG1" > /dev/null;
    shell '[ ! -s $datafile ] && exit 1;exit 0';
    regex -v "$RET" '0' && set -l all=1 &&
    readfile "$datafile" |
    regex
    '^(..*)$0'
    'name="$m0"
    ';
    regex -v "$all"
    '1'
    'name=all'
`
