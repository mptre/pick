#!/bin/sh

set -e

# Strip leading spaces and trailing comment from $1.
strip() {
	echo "$1" | sed -e 's/^ *//' -e 's/ *#.*$//'
}

run_test() {
	local _cause= _diff=$stdout

	env $env tests/pick-test -k $input -- $args <$stdin >$out 2>&1; e=$?

	if [ -s "$stdout" ] && ! cmp -s "$stdout" "$out"; then
		_cause="wrong output"
	elif [ "${exit:-0}" -ne "$e" ]; then
		_cause="want exit code ${exit}, got ${e}"
		_diff="/dev/null"
	fi

	if [ -n "$_cause" ]; then
		printf 'FAIL:\t%s\n' "$description" 1>&2
		printf 'CAUSE:\t%s\n' "$_cause" 1>&2
		diff -u -L stdout-want -L stdout-got "$_diff" "$out"
		return 1
	fi

	return 0
}

usage() {
	echo "usage: sh tests/pick-test.sh [-e env] file ..." 1>&2
	exit 1
}

# Default environment applied to all test cases.
defenv=

while getopts "e:" opt; do
	case "$opt" in
	e)	defenv="${defenv} ${OPTARG}";;
	*)	usage;;
	esac
done
shift $((OPTIND - 1))
[ $# -eq 0 ] && usage

nerr=0

in=$(mktemp -t pick-test.XXXXXX)
out=$(mktemp -t pick-test.XXXXXX)
stdin=$(mktemp -t pick-test.XXXXXX)
stdout=$(mktemp -t pick-test.XXXXXX)
input=$(mktemp -t pick-test.XXXXXX)
trap "rm -f $in $out $stdin $stdout $input" EXIT

for f; do
	(cat "$f"; echo) >$in
	while IFS=: read -r key val; do
		if [ -z "$key" ]; then
			env="${defenv} ${env}"
			run_test || nerr=$((nerr + 1))

			# Reset environment.
			args= description= env= exit= keys=
			>$out; >$stdin; >$stdout; >$input
		elif [ "$key" = "keys" ]; then
			printf "${val%%#*}" >$input
		elif [ -n "$val" ]; then
			eval "${key}='$(strip "$val")'"
		else
			case "$key" in
			stdin)	tmpfile=$stdin;;
			stdout)	tmpfile=$stdout;;
			*)	printf "${key}\n" >>$tmpfile;;
			esac
		fi
	done <$in
done

exit $((nerr > 0))
