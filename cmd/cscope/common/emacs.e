/ emacs menu for cscope /
((Ø) cscope (find current word [MACRO])
	(extern symbol-character)

	/ if this character is not part of a symbol /
	(cond ((not symbol-character)
		
		/ if the previous character is not part of a symbol, go to
		  the next word /
		back
		(cond ((not symbol-character) forward-word back-word))
	))
	/ get the current symbol (leave cursor at beginning of symbol) /
	(while symbol-character forward)	/ go past last symbol character /
	mark					/ mark this point /
	back					/ back to last symbol character /
	(while (cond (symbol-character (return back))))	/ back fails at BOF /
	(cond ((not symbol-character) forward))		/ forward if not at BOF /
	pickup-region				/ get the symbol /
	(local-string symbol)
	symbol=

	/ if arg > 0 then display the menu /
	(cond ((> arg 0) (display-menu
		(format symbol "5  Find functions calling %l()")
		(format symbol "4  Find functions called by %l()")
		(format symbol "3  Find global definition of %l")
		(format symbol "2  Find symbol %l")
		"1  Call cscope"
		5)
	))
	/ get the selection /
	(local selection)
	(selection= (read-character "Selection?"))

	/ if the selection is in range /
	(cond ((&& (>= selection '1') (<= selection '5'))
	
		/ if the selection requests finding the symbol /
		(local-string findsymbol)
		(findsymbol= "")
		(cond ((>= selection '2')
			(findsymbol= (format (char-to-string (- selection 2)) symbol "-%l '%l'"))))
		
		/ if arg > 1 or < 0 then don't update the cross-reference database /
		(local-string doption)
		(doption= "")
		(cond ((|| (> arg 1) (< arg 0)) (doption= "-d")))
		
		/ call cscope with usilent mode off /
		(local oldmode)				/ save old usilent mode /
		(oldmode= (set-mode "usilent" 0))	/ turn off usilent mode /
		(run-command (format doption findsymbol "cscope %l %l"))
		(set-mode "usilent" oldmode)		/ restore usilent mode /
	))
)
/ see if the current character is part of a symbol /
(symbol-character ()
	(local c)
	(c= current-character)
	(return (cond	((&& (>= c 'a') (<= c 'z')))
			((&& (>= c 'A') (<= c 'Z')))
			((&& (>= c '0') (<= c '9')))
			((== c '_'))
		)
	)
)
