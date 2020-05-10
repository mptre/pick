if testcase "up with vertical scroll"; then
	{ echo 1; echo 2; echo 3; echo 4; } >"$STDIN"
	pick -k "^N ^N ^N ^N ^P ^P ^P ^P \\n" <<-EOF
	1
	EOF
fi

if testcase "up out of bounds with vertical scroll"; then
	{ echo 1; echo 2; echo 3; echo 4; echo 5; } >"$STDIN"
	pick -k "^N ^N ^N ^N ^P ^P ^P ^P ^P \\n" -l 5 <<-EOF
	1
	EOF
fi

if testcase "up arrow is an alias for up"; then
	{ echo 1; echo 2; echo 3; echo 4; } >"$STDIN"
	pick -k "^N ^N ^N ^N \\033OA \\033OA \\033OA \\033OA \\n" <<-EOF
	1
	EOF
fi
