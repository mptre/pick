if testcase "unknown option"; then
	pick -e -o -- -0
fi

if testcase "extra argument"; then
	pick -e -o -- a
fi
