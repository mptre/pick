if testcase "enable alternate screen option"; then
	echo a >"$STDIN"
	pick -k "\\n" -- -x <<-EOF
	a
	EOF
fi

if testcase "disable alternate screen option"; then
	echo a >"$STDIN"
	pick -k "\\n" -- -X <<-EOF
	a
	EOF
fi
