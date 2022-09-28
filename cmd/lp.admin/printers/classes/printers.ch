#ident	"@(#)lp.admin:printers/classes/printers.ch	1.1"
menu=Printers

lifetime=shortterm

multiselect
init=`cocreate -i lpdata -R printers.ch -e "-EOT-" $SPOOLDIR/bin/lpdata -f $datafile`
done=CLOSE
close=`
	getitems "," | set -l x;
	echo $x | regex
		'^all,'
			'all'
		'(.*)$0'
			'$m0' |
	set -l Form_Choice;
	unset -l x;
	codestroy -R printers.ch lpdata;
`

`	cosend lpdata list_printers |
	regex
		'^(..*)$0'
		'name="$m0"
		'
`
