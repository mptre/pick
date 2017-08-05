#!/bin/sh

usage() {
  echo "usage: sh tests/pick-test.sh file ..." 1>&2
  exit 1
}

[ $# -eq 0 ] && usage

fail=

stdin=$(mktemp -t pick.XXXXXX)
stdout=$(mktemp -t pick.exp.XXXXXX)
out=$(mktemp -t pick.act.XXXXXX)
keys=$(mktemp -t pick.keys.XXXXXX)
trap 'rm $stdin $stdout $out $keys' EXIT

for testcase; do
  exit=0

  while IFS=: read -r key val; do
    if [ "$key" = "keys" ]; then
      printf "${val%%#*}" >$keys
    elif [ -n "$val" ]; then
      eval "${key}='${val%%#*}'"
    else
      case "$key" in
      stdin)  { tmpfile=$stdin; >$tmpfile; } ;;
      stdout) { tmpfile=$stdout; >$tmpfile; } ;;
      *)      printf "${key}\n" >>$tmpfile ;;
      esac
    fi
  done <$testcase

  env $env tests/pick-test -k $keys -- $args <$stdin >$out 2>&1; e=$?
  if [ "$exit" -ne "$e" ]; then
    echo "${testcase}: expected exit code ${exit}, got ${e}" 1>&2
    cat "$out" 1>&2
    fail=1
  fi
  if [ -s "$stdout" ] && ! diff -u "$stdout" "$out"; then
    fail=1
  fi
done

exit $fail
