#
# pick command from history with Ctrl-R (for Zsh)
#

TAC=$( which tac || echo 'tail -r' )

pick-history () {
	BUFFER=${$( fc -ln 1 | eval $TAC | env LINES=10 pick -q "$LBUFFER" -X ):-$BUFFER}
	CURSOR=${#BUFFER}
	zle		reset-prompt
}
	zle -N		pick-history
	bindkey '^R'	pick-history
