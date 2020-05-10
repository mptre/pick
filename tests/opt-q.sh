if testcase "filter using the query option"; then
	{ echo b; echo a; } >"$STDIN"
	pick -k "\\n" -- -q a <<-EOF
	a
	EOF
fi
