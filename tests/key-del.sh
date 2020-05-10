if testcase "delete removes the character under cursor"; then
	{ echo ab; echo ac; } >"$STDIN"
	pick -k "ab ^B ^D c \\n" <<-EOF
	ac
	EOF
fi

if testcase "utf-8 delete"; then
	{ echo aa; echo aå; } >"$STDIN"
	pick -k "aå ^B ^D a \\n" <<-EOF
	aa
	EOF
fi

if testcase "delete key is an alias for delete"; then
	{ echo ab; echo ac; } >"$STDIN"
	pick -k "ab ^B \\033[3~ c \\n" <<-EOF
	ac
	EOF
fi
