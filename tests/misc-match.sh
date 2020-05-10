if testcase "matching is case insensitive"; then
	echo A >"$STDIN"
	pick -k "a \\n" <<-EOF
	A
	EOF
fi

if testcase "it favors the shortest match"; then
	{ echo aa/åå/aa; echo aa/åå/aa/aa; } >"$STDIN"
	pick -k "aa/aa \\n" <<-EOF
	aa/åå/aa/aa
	EOF
fi

if testcase "do not match inside a csi escape sequence"; then
	{ printf "\\033[32m33\\033[m\\n"; echo 3aaa2; } >"$STDIN"
	pick -k "32 \\n" <<-EOF
	3aaa2
	EOF
fi

if testcase "do not match inside a osc escape sequence"; then
	{
		printf "\\033]8;;example.com\\aOSC Hyperlink\\033]8;;\\a\\n"
		echo favored match since the query is not inside the escape sequence example.com
	} >"$STDIN"
	pick -k "example \\n" <<-EOF
	favored match since the query is not inside the escape sequence example.com
	EOF
fi
