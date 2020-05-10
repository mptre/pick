if testcase "enter selects the first choice"; then
	{ echo 1; echo 2; echo 3; echo 4; } >"$STDIN"
	pick -k "\\n" <<-EOF
	1
	EOF
fi

if testcase "enter does nothing if no choices are present"; then
	echo 1 >"$STDIN"
	pick -k "2 \\n \\033\\n" <<-EOF
	2
	EOF
fi

if testcase "tabs in input"; then
	printf '\ta\n' >"$STDIN"
	pick -k "\\n" <<EOF
	a
EOF
fi

if testcase "carriage return is an alias for enter"; then
	echo a >"$STDIN"
	pick -k "\\r" <<-EOF
	a
	EOF
fi

if testcase "key enter is an alias for enter"; then
	echo a >"$STDIN"
	pick -k "\\033OM" <<-EOF
	a
	EOF
fi
