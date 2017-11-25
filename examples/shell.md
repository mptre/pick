# Shell

## History

Pick a command to execute from the history with Ctrl-R:

### Bash (>= 4.x)

```sh
TAC=$( which tac || echo 'tail -r' )

pick-history () {
	local BUFFER=$( history | cut -c 8- | eval $TAC | env LINES=10 pick -q "${READLINE_LINE:0:READLINE_POINT}" -X )
	READLINE_LINE=${BUFFER:-$READLINE_LINE}
	READLINE_POINT=${#READLINE_LINE}
}
	bind -x '"\C-r" : pick-history'
```

### Zsh

```sh
TAC=$( which tac || echo 'tail -r' )

pick-history () {
	zle		-M ''
	BUFFER=${$( fc -ln 1 | eval $TAC | env LINES=10 pick -q "$LBUFFER" -X ):-$BUFFER}
	CURSOR=${#BUFFER}
	zle		-R -c
	zle		reset-prompt
}
	zle -N		pick-history
	bindkey '^R'	pick-history
```
