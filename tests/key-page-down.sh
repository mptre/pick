if testcase "page down scrolls a page worth of choices"; then
	{ echo 1; echo 2; echo 3; echo 4; echo 5; } >"$STDIN"
	pick -k "\\033[6~ \\n" -l 5 <<-EOF
	5
	EOF
fi

if testcase "page down first choice not selected"; then
	{ echo 1; echo 2; echo 3; echo 4; echo 5; echo 6; } >"$STDIN"
	pick -k "^N \\033[6~ \\n" -l 5 <<-EOF
	6
	EOF
fi

if testcase "page down all choices fit on one page"; then
	{ echo 1; echo 2; echo 3; echo 4; } >"$STDIN"
	pick -k "\\033[6~ \\n" -l 5 <<-EOF
	4
	EOF
fi

if testcase "ctrl-v is an alias for page down"; then
	{ echo 1; echo 2; echo 3; echo 4; echo 5; } >"$STDIN"
	pick -k "\\026 \\n" -l 5 <<-EOF
	5
	EOF
fi

if testcase "alt-space is an alias for page down"; then
	{ echo 1; echo 2; echo 3; echo 4; echo 5; } >"$STDIN"
	pick -k "\\033\\  \\n" -l 5 <<-EOF
	5
	EOF
fi
