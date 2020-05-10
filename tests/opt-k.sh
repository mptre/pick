if testcase "disable toggling of keyboard transmit mode"; then
	echo xyz >"$STDIN"
	pick -k "\\n" -- -K <<-EOF
	xyz
	EOF
fi
