#!/bin/sh

usage() {
  echo "usage: sh tests/pick-test.sh file ..." 1>&2
  exit 1
}

[ $# -eq 0 ] && usage

fail=

stdout=$(mktemp -t pick.act.XXXXXX)
stdin=$(mktemp -t pick.XXXXXX)
out=$(mktemp -t pick.exp.XXXXXX)
trap "rm "$stdout" "$stdin" "$out"" EXIT

for testcase; do
  exit=0

  while IFS=: read -r key val; do
    if [ -n "$val" ]; then
      eval "${key}='${val%%#*}'"
    else
      case "$key" in
      stdin)  tmpfile=$stdin; >$tmpfile ;;
      stdout) tmpfile=$stdout; >$tmpfile ;;
      *)      echo "$key" >>$tmpfile ;;
      esac
    fi
  done <$testcase

  tests/pick-test -k "$(printf "$keys")" -- $args <$stdin >$out 2>&1; e=$?
  if [ "$exit" -ne "$e" ]; then
    echo "${testcase}: expected exit code ${exit}, got ${e}" 1>&2
    cat "$out" 1>&2
    fail=1
  fi
  if [ -s "$stdout" ] && ! diff -u "$out" "$stdout"; then
    fail=1
  fi
done

exit $fail
