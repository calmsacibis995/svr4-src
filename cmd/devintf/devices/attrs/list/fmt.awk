#ident	"@(#)devintf:devices/attrs/list/fmt.awk	1.1"
/^[^']+=/	{
	FS = "=" ;
	attr = $1 ;
	fill = "" ;
	for (i = length($1) ; i < 15 ; i++) fill = fill" " ;
}
/^[^']+=/  	{ print $1fill$2 } ;
/^[^=]*'/	{ print "               "$0 } ;
