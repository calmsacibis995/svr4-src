#ident	"@(#)lp.admin:printers/filters/filter.ch	1.1"
menu=Filters

lifetime=shortterm

multiselect

close=`
	getitems "," | set -l Form_Choice;
	regex -v "$Form_Choice" '^all,.*' 'all' '(.*)$0' '$m0' '^$' | set -l Form_Choice;
	return 0
`

name=all

`cut -d: -f5 $SPOOLDIR/admins/lp/filter.table | regex '^(.*)$0' '
name="$m0"
'`
