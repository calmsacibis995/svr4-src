#ident	"@(#)bkrs:bkexcept.d/exconv.com	1.1"
# 1 convert \.* to a special string to avoid collapsing to .* and then to *
# 2 convert \\ to a special string
# 3 any expression with a character that is not the . followed by * is suspect
# 4 any expression with parentheses or braces is questionable
# 5 any \n expression is questionable
# 6 collapse .* to *
# 7 add a trailing * to expressions that do not end in $ 
# 8 prepend * to expressions not beginning with ^
# 9 a beginning ^ goes to nothing
# 10 a terminating escaped $ is converted to special expression
# 11 a terminating dollar which is not escaped goes to nothing
# 12 [^ is replaced by [!
# 13 replace questionable expressions by QUESTIONABLE: followed by expression
# 14 put in escaped .*
# 15 put back double backslash
# 16 replace escaped single characters by the character itself
# 17 put back escaped $
# 18 a ? is escaped
