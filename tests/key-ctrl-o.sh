if testcase "ctrl-o toggles sorting"; then
	{ echo foo bar; echo foo; } >"$STDIN"
	pick -k "foo ^O \\n" <<-EOF
	foo bar
	EOF
fi

if testcase "it works in conjunction with the -S option"; then
	{ echo foo bar; echo foo; } >"$STDIN"
	pick -k "foo ^O \\n" -- -S <<-EOF
	foo
	EOF
fi
