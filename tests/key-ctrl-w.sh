if testcase "ctrl-w deletes the ascii word behind the cursor"; then
	{ echo ab; echo ab cd; } >"$STDIN"
	pick -k "^W ab\\ cd ^W ^W \\n" -- <<-EOF
	ab
	EOF
fi

if testcase "ctrl-w deletes the utf-8 word behind the cursor"; then
	{ echo aa Åå aa; echo aa Åå aa aa; } >"$STDIN"
	pick -k "aa\\ Åå ^W aa\\ aa \\n" -- <<-EOF
	aa Åå aa aa
	EOF
fi

if testcase "non alnum characters are not recognized as part of a word"; then
	{ echo a/a; echo a/a/a; } >"$STDIN"
	pick -k "a/a ^W a/a \\n" -- <<-EOF
	a/a/a
	EOF
fi

if testcase "underscore is recognized as part of a word"; then
	{ echo aa_aa_bb; echo bb; } >"$STDIN"
	pick -k "aa_aa ^W bb \\n" -- <<-EOF
	bb
	EOF
fi

if testcase "alt-backspace is an alias for ctrl-w"; then
	{ echo aa Åå aa; echo aa Åå aa aa; } >"$STDIN"
	pick -k "aa\\ Åå \\033\\b aa\\ aa \\n" -- <<-EOF
	aa Åå aa aa
	EOF
fi
