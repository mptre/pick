if testcase "ctrl-e moves the cursor to the end of the query"; then
	echo abc >"$STDIN"
	pick -k "b ^A a ^E c \\n" <<-EOF
	abc
	EOF
fi
