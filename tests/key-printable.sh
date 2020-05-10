if testcase "printable characters are added to the query"; then
	{ echo BBB; echo ABC; echo AB; echo A; } >"$STDIN"
	pick -k "A \\n" <<-EOF
	A
	EOF
fi

if testcase "printable UTF-8 characters are added to the query"; then
	{ echo å; echo ä; echo ö; } >"$STDIN"
	pick -k "ö \\n" <<-EOF
	ö
	EOF
fi

if testcase "utf-8 false positive"; then
	{ echo íš á; echo háh háh; } >"$STDIN"
	pick -k "áá \\n" <<-EOF
	háh háh
	EOF
fi

if testcase "utf-8 four byte wide characters"; then
	{ echo 😀; echo 💩; } >"$STDIN"
	pick -k "💩 \\n" <<-EOF
	💩
	EOF
fi

if testcase "changing the query resets vertical scroll"; then
	{ echo 01; echo 02; echo 03; echo 04; echo 05; } >"$STDIN"
	pick -k "^N ^N ^N ^N ^N 0 \\n" -l 5 <<-EOF
	01
	EOF
fi
