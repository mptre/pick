if testcase "ctrl-k deletes til the end of line"; then
	{ echo a; echo ab; } >"$STDIN"
	pick -k "ab ^B ^K \\n" <<-EOF
	a
	EOF
fi
