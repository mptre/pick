# pick [-e] [-o] [-k keys] [-l lines] -- [pick-argument ...]
pick() {
	local _env=""
	local _exit1=0
	local _exit2=0
	local _keys="${TSHDIR}/_keys"
	local _out="${TSHDIR}/_out"
	local _output=1
	local _sig=""

	while [ "$#" -gt 0 ]; do
		case "$1" in
		-e)	_exit1=1;;
		-k)	shift; printf "$1" >"$_keys";;
		-l)	shift; _env="${_env} LINES=${1}";;
		-o)	shift; _output=0;;
		*)	break;;
		esac
		shift
	done
	[ "$1" = "--" ] && shift

	[ -e "$STDIN" ] || : >"$STDIN"
	[ -e "$_keys" ] || : >"$_keys"

	# shellcheck disable=SC2086
	env $_env "$PTY" -k "$_keys" -- $EXEC "$PICK" "$@" \
		<"$STDIN" >"$_out" 2>&1 || _exit2="$?"
	if [ "$_exit1" -ne "$_exit2" ]; then
		if [ "$_exit2" -gt 128 ]; then
			_sig=" (signal $((_exit2 - 128)))"
		fi
		fail - "want exit ${_exit1}, got ${_exit2}${_sig}" <"$_out"
		# Output already displayed, prevent from doing it twice.
		_output=0
	fi

	if [ "$_output" -eq 1 ]; then
		assert_file - "$_out"
	fi
}

ls "${PICK:?}" "${PTY:?}" >/dev/null

# Enable hardening malloc(3) options on OpenBSD.
case "$(uname -s)" in
OpenBSD)	export MALLOC_OPTIONS="RS";;
esac

STDIN="${TSHDIR}/stdin"; export STDIN
