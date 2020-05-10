if testcase "unrecgonized escape sequences are ignored"; then
	echo a >"$STDIN"
	pick -k "\\033[1111111111111111~ \\n" <<-EOF
	a
	EOF
fi
