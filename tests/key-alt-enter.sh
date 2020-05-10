if testcase "alt enter selects the current query"; then
	echo aaaa >"$STDIN"
	pick -k "aa \\033\\n" <<-EOF
	aa
	EOF
fi

if testcase "alt carriage return is an alias for alt enter"; then
	echo aaaa >"$STDIN"
	pick -k "aa \\033\\r" <<-EOF
	aa
	EOF
fi
