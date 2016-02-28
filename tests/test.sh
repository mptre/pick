#!/bin/sh

field() {
  awk '
  /^'$1':/ { sub(/^[^:]+: */, ""); print; exit 0 }
  END { exit 1 }'
}

input() {
  awk '
  /^$/ { input = 1; next }
  input { print $0 }
  '
}

pick() {
  args=$(field arguments <"$1")
  input=$(field input <"$1")

  input <"$1" >"$in"
  $DIR/test -i "$input" -- $args <"$in"
}

main() {
  DIR=$(dirname "$0")
  PATH="${DIR}/../src:${PATH}"

  in=$(mktemp pick.XXXXXX)
  exp=$(mktemp pick.XXXXXX)
  act=$(mktemp pick.XXXXXX)
  err=$(mktemp pick.XXXXXX)
  trap 'rm "$in" "$exp" "$act" "$err"' EXIT

  [ $# -eq 0 ] && set $(find "$DIR" -name '*.in')
  for a
  do
    e=$(field exit <"$a" || echo 0)

    field output <"$a" >"$exp"
    pick "$a" >"$act" 2>"$err"
    if [ $? -ne 0 -a $e -eq 0 ]
    then
      echo "${a}: wrong exit code" 1>&2
      cat "$err"
      exit 1
    fi
    ! diff -c "$exp" "$act" && { cat "$err"; exit 1; }
  done
  return 0
}

main $@
