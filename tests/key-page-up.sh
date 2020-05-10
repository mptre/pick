if testcase "page up scrolls a page worth of choices"; then
	{ echo 1; echo 2; echo 3; echo 4; echo 5; echo 6; echo 7; echo 8; echo 9; } >"$STDIN"
	pick -k "\\033[6~ \\033[6~ \\033[5~ \\n" -l 5 <<-EOF
	5
	EOF
fi

if testcase "page up selected choice below the first page"; then
	{ echo 1; echo 2; echo 3; echo 4; echo 5; echo 6; } >"$STDIN"
	pick -k "\\033[6~ \\016 \\033[5~ \\n" -l 5 <<-EOF
	2
	EOF
fi

if testcase "page up all choices fit on one page"; then
	{ echo 1; echo 2; echo 3; echo 4; } >"$STDIN"
	pick -k "\\033[6~ \\033[5~ \\n" -l 5 <<-EOF
	1
	EOF
fi

if testcase "alt-v is an alias for page up"; then
	{ echo 1; echo 2; echo 3; echo 4; echo 5; echo 6; echo 7; echo 8; echo 9; } >"$STDIN"
	pick -k "^V ^V \\033v \\n" -l 5 <<-EOF
	5
	EOF
fi
