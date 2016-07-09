mktmp() {
  f=$(mktemp -t pick.XXXXXX)
  trap "rm "$f"" EXIT
  echo "$f"
}

usage() {
  echo "usage: sh tests/test.sh file ..." 1>&2
  exit 1
}

[ $# -eq 0 ] && usage

fail=

stdout=$(mktmp)
stdin=$(mktmp)
out=$(mktmp)

for testcase; do
  exit=0

  while IFS=: read -r key val; do
    if [ -n "$val" ]; then
      eval "$key='$val'"
    else
      case "$key" in
      stdin)  tmpfile=$stdin; >$tmpfile ;;
      stdout) tmpfile=$stdout; >$tmpfile ;;
      *)      echo "$key" >>$tmpfile ;;
      esac
    fi
  done <$testcase

  env PATH="./src:${PATH}" tests/pick-test \
    -i "$(printf "$keys")" -- $args <$stdin >$out 2>&1; e=$?
  if [ "$exit" -ne "$e" ]; then
    echo "${testcase}: expected exit code ${exit}, got ${e}" 1>&2
    fail=1
  fi
  if [ -s "$stdout" ] && ! diff -c "$stdout" "$out"; then
    fail=1
  fi
done

exit $fail
