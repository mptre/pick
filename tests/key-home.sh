if testcase "home selects the first choice"; then
	{ echo 1; echo 2; echo 3; echo 4; } >"$STDIN"
	pick -k "^N \\033OH \\n" <<-EOF
	1
	EOF
fi

if testcase "alt-< is an alias for home"; then
	{ echo 1; echo 2; echo 3; echo 4; } >"$STDIN"
	pick -k "^N \\033< \\n" <<-EOF
	1
	EOF
fi
