#!/bin/sh

# Copyright (c) 2019 Anton Lindqvist <anton@basename.se>
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

set -e

# assert_eq want got [message]
assert_eq() {
	if ! _assert_eq "$1" "$2"; then
		printf 'WANT:\t%s\nGOT:\t%s\n' "$1" "$2" | fail - "${3:-assert_eq}"
	fi
}

# refute_eq want got [message]
refute_eq() {
	if _assert_eq "$1" "$2"; then
		fail "${3:-refute_eq}"
	fi
}

# _assert_eq want got
_assert_eq() {
	[ "$1" = "$2" ]
}

# assert_file file0 file1 [message]
assert_file() {
	local _file0
	local _file1

	_file0="$1"; _file1="$2"
	if [ "$_file0" = "-" ]; then
		_file0="${TSHDIR}/assert_file"
		cat >"$_file0"
	elif [ "$_file1" = "-" ]; then
		_file1="${TSHDIR}/assert_file"
		cat >"$_file1"
	fi

	if ! _assert_file "$_file0" "$_file1"; then
		diff -u -L want -L got "$_file0" "$_file1" 2>&1 |
		fail - "${3:-assert_file}"
	fi
}

# refute_file file0 file1 [message]
refute_file() {
	if _assert_file "$1" "$2"; then
		fail "${3:-refute_file}"
	fi
}

# _assert_file file0 file1
_assert_file() {
	cmp -s "$1" "$2"
}

# fail [-] [message]
fail() {
	echo >>"$NERR"

	_report -f -p FAIL "$@"
	if [ "$FAST" -eq 1 ]; then
		exit 1
	fi
}

# testcase [-t tag] description
testcase() {
	local _tags=""

	# Report on behalf of the previous test case.
	[ -s "$NTEST" ] && _report -p PASS

	echo >>"$NTEST"

	find "$TSHDIR" -mindepth 1 -delete

	while [ $# -gt 0 ]; do
		case "$1" in
		-t)	shift; _tags="${_tags} ${1}";;
		*)	break;;
		esac
		shift
	done
	TCDESC="$*"

	if _runnable "$TCDESC" "$_tags"; then
		if command -v setup >/dev/null 2>&1; then
			setup
		fi
		return 0
	fi

	_report -p SKIP
	return 1
}

# Everything below is part of the private API, relying on it is a bad idea.

# _fatal message
_fatal() {
	echo "t.sh: ${*}" 1>&2
	exit 1
}

# _report [-] [-f] -p prefix [message]
_report() {
	local _force=0  _prefix="" _stdin=0 _tmp="${TSHDIR}/_report"

	while [ $# -gt 0 ]; do
		case "$1" in
		-)	_stdin=1;;
		-f)	_force=1;;
		-p)	shift; _prefix="$1";;
		*)	break;;
		esac
		shift
	done

	if [ "$_force" -eq 0 ] && [ -e "$TCREPORT" ]; then
		return 0
	fi
	: >"$TCREPORT"

	# Try hard to output everything to stderr in one go.
	{
		printf '%s: %s: %s' "$_prefix" "$NAME" "$TCDESC"
		[ $# -gt 0 ] && printf ': %s' "$*"
		echo
		[ $_stdin -eq 1 ] && cat
	} >"$_tmp"
	cat <"$_tmp" 1>&2
}

# _runnable description [tags]
_runnable() {
	local _desc
	local _tags

	_desc="$1"; : "${_desc:?}"
	_tags="$2"

	if [ -s "$INCLUDE" ]; then
		case "$FILTER" in
		f)	echo "$_desc";;
		t)	echo "$_tags";;
		esac | grep -q -f "$INCLUDE" && return 0
	elif [ -s "$EXCLUDE" ]; then
		case "$FILTER" in
		F)	echo "$_desc";;
		T)	echo "$_tags";;
		esac | grep -q -f "$EXCLUDE" || return 0
	else
		return 0
	fi
	return 1
}

# atexit file ...
atexit() {
	local _err="$?"

	if [ -s "$NTEST" ]; then
		# Report on behalf of the previous test case.
		if [ "$_err" -eq 0 ]; then
			_report -p PASS
		else
			_report -p FAIL
		fi
	fi

	if [ -s "$NERR" ]; then
		_err="1"
	fi

	# Remove temporary files.
	rm -rf "$@"

	exit "$_err"
}

usage() {
	echo "usage: sh t.sh [-x] [-f filter] [-t tag] file ..." 1>&2
	exit 1
}

# Keep crucial files outside of temporary directory since it's wiped between
# test cases.
INCLUDE="$(mktemp -t t.sh.XXXXXX)"
EXCLUDE="$(mktemp -t t.sh.XXXXXX)"
NERR="$(mktemp -t t.sh.XXXXXX)"
NTEST="$(mktemp -t t.sh.XXXXXX)"
TSHDIR="$(mktemp -d -t t.sh.XXXXXX)"
TSHCLEAN=""
trap 'atexit $INCLUDE $EXCLUDE $NERR $NTEST $TSHCLEAN $TSHDIR' EXIT

# Exit on first failure.
FAST=0
# Filter mode.
FILTER=""
# Test file name.
NAME=""
# Current test case description.
TCDESC=""
# Current test case called report indicator.
TCREPORT="${TSHDIR}/_tcreport"

while getopts "F:f:t:T:x" opt; do
	case "$opt" in
	f|t)	FILTER="$opt"
		echo "$OPTARG" >>"$INCLUDE"
		;;
	F|T)	FILTER="$opt"
		echo "$OPTARG" >>"$EXCLUDE"
		;;
	x)	FAST=1;;
	*)	usage;;
	esac
done
shift $((OPTIND - 1))
[ $# -eq 0 ] && usage
if [ -s "$INCLUDE" ] && [ -s "$EXCLUDE" ]; then
	_fatal "including and excluding tests is mutually exclusive"
fi

for a; do
	NAME="${a##*/}"
	. "$a"
done
