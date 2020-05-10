if testcase "ctrl-a moves the cursor to beginning of the query"; then
	echo bba >"$STDIN"
	pick -k "a ^A bb \\n" <<-EOF
	bba
	EOF
fi
