if testcase "with sorting disabled"; then
	{ echo BBB; echo ABC; echo AB; echo A; } >"$STDIN"
	pick -k "A \\n" -- -S <<-EOF
	ABC
	EOF
fi
