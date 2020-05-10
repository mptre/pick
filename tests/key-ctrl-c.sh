if testcase "ctrl-c aborts without outputting the selected choice"; then
	echo a >"$STDIN"
	pick -e -k "^C" </dev/null
fi
