if testcase "down moves the selection downwards"; then
	{ echo 1; echo 2; echo 3; echo 4; } >"$STDIN"
	pick -k "^N ^N ^N ^N \\n" <<-EOF
	4
	EOF
fi

if testcase "down out of bounds"; then
	{ echo 1; echo 2; } >"$STDIN"
	pick -k "^N ^N ^N \\n" <<-EOF
	2
	EOF
fi

if testcase "down with vertical scroll"; then
	{ echo 1; echo 2; echo 3; echo 4; echo 5; } >"$STDIN"
	pick -k "^N ^N ^N ^N \\n" -l 5 <<-EOF
	5
	EOF
fi

if testcase "down out of bounds with vertical scroll"; then
	{ echo 1; echo 2; echo 3; echo 4; echo 5; } >"$STDIN"
	pick -k "^N ^N ^N ^N ^N \\n" -l 5 <<-EOF
	5
	EOF
fi

if testcase "down arrow is an alias for down"; then
	{ echo 1; echo 2; echo 3; echo 4; } >"$STDIN"
	pick -k "\\033OB \\n" -l 5 <<-EOF
	2
	EOF
fi
