if testcase "backspace deletes the ascii character behind the cursor"; then
	{ echo ab; echo ac; } >"$STDIN"
	pick -k "abb \\b \\b c \\n" <<-EOF
	ac
	EOF
fi

if testcase "backspace deletes the utf-8 character behind the cursor"; then
	{ echo å; echo ä; echo ö; } >"$STDIN"
	pick -k "ö \\b \\n" <<-EOF
	å
	EOF
fi

if testcase "del is an alias for backspace"; then
	{ echo a; echo c; } >"$STDIN"
	pick -k "a \\177 c \\n" <<-EOF
	c
	EOF
fi
