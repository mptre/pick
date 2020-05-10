if testcase "left moves the cursor to the left"; then
	echo ba >"$STDIN"
	pick -k "a ^B b \\n" <<-EOF
	ba
	EOF
fi

if testcase "left moves the cursor to the left and handles utf-8"; then
	echo öåaå >"$STDIN"
	pick -k "åaå ^B ^B ^B ö \\n" <<-EOF
	öåaå
	EOF
fi

if testcase "left arrow is an alias for left"; then
	echo ba >"$STDIN"
	pick -k "a \\033OD b \\n" <<-EOF
	ba
	EOF
fi
