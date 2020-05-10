if testcase "end selects the last choice"; then
	{ echo 1; echo 2; echo 3; echo 4; } >"$STDIN"
	pick -k "\\033OF \\n" <<-EOF
	4
	EOF
fi

if testcase "alt-> is an alias for end"; then
	{ echo 1; echo 2; echo 3; echo 4; } >"$STDIN"
	pick -k "\\033> \\n" <<-EOF
	4
	EOF
fi
