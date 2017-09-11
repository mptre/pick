description: non alphanumeric characters are not recognized as part of a word (Alt-Backspace alias)
keys: a/a \033\b a/a \n # ALT_BACKSPACE ENTER
stdin:
a/a
a/a/a
stdout:
a/a/a
