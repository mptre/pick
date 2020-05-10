if testcase "ctrl-l does not clear the selection"; then
	{ echo a; echo b; } >"$STDIN"
	pick -k "^N ^L \\n" <<-EOF
	b
	EOF
fi
