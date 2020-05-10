if testcase "ctrl-u deletes til the cursor"; then
	{ echo abc; echo c; } >"$STDIN"
	pick -k "abc ^B ^U \\n" -- <<-EOF
	c
	EOF
fi
