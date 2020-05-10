if testcase "output description"; then
	{ echo a b; echo aaab; } >"$STDIN"
	pick -k "a \\n" -- -do <<-EOF
	a
	b
	EOF
fi
