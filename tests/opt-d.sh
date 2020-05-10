if testcase "do not search descriptions"; then
	{ echo a b; echo aaab; } >"$STDIN"
	pick -k "b \\n" -- -d <<-EOF
	aaab
	EOF
fi
