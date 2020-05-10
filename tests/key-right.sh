if testcase "right moves the cursor to the right"; then
	echo bab >"$STDIN"
	pick -k "a ^B b ^F b \\n" <<-EOF
	bab
	EOF
fi

if testcase "right moves the cursor to the right and handles utf-8"; then
	echo åaåö >"$STDIN"
	pick -k "åaå ^A ^F ^F ^F ö \\n" <<-EOF
	åaåö
	EOF
fi

if testcase "right arrow is an alias for right"; then
	echo bab >"$STDIN"
	pick -k "a ^B b \\033OC b \\n" <<-EOF
	bab
	EOF
fi

if testcase "right end of line"; then
	echo a >"$STDIN"
	pick -k "^F \\n" <<-EOF
	a
	EOF
fi
