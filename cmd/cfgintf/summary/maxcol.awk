#ident	"@(#)cfgintf:summary/maxcol.awk	1.1.1.1"

BEGIN {
	maxcol = 0
}

maxcol < length { maxcol = length }

END {print maxcol}
