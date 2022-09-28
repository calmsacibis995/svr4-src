#ident	"@(#)bkrs:bkexcept.d/exconv.sed	1.3"
s;\\\.\*;BACKDOTSTARBACKDOTSTAR;g
s;\\\\;BACKSLASHBACKSLASH;g
s;^.*[^.]\*.*$;QUESTIONABLEQUESTIONABLE&;
s;^.*[{}()].*$;QUESTIONABLEQUESTIONABLE&;
s;^.*\\[0-9].*$;QUESTIONABLEQUESTIONABLE&;
s;\.\*;*;g
s;\([^$]\)$;\1*;
s;^\([^^]\);*\1;
s;^^;;
s;\\$$;BACKSLASHDOLLARBACKSLASHDOLLAR;
s;\([^\]\)$$;\1;
s;\[\^;\[!;g
s;\(.*\)QUESTIONABLEQUESTIONABLE\(.*\)$;QUESTIONABLE:	\1\2;
s;BACKDOTSTARBACKDOTSTAR;.*;g
s;BACKSLASHBACKSLASH;\\;
s;\\\(.\);\1;g
s;BACKSLASHDOLLARBACKSLASHDOLLAR;$;
s;\?;\\\?;g
