	; cscope.ml (s.cscope.ml) - 1.4 (2/21/84 14:53:58)
	;
	; Macro to handle invocation of gmacs by cscope from the
	; experimental tools.  Cscope invokes gmacs with two arguments:
	;
	;	gmacs +line file
	;
	; This macro gobbles the line number, visits the specified file,
	; and moves to the specified line number.
	;
	; Author:	Tom Whitten

(progn
	args
	pluses
	(setq pluses 0)
	(setq args (argc))
	(if (> args 1)
		(progn
			(if (= (string-to-char "+") (string-to-char (argv 1)))
				(setq pluses 1)
			)
			(setq args (- args 1))
			(while (> args pluses)
				(visit-file (argv args))
				(setq args (- args 1))
			)
			(if (= (> (argc) 2) (> pluses 0))
				(goto-line (argv 1))
			)
		)
	)
)
