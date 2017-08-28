description: non alphanumeric characters are not recognized as part of a word
keys: a/a \027 a/a \n # CTRL_W ENTER
stdin:
a/a
a/a/a
stdout:
a/a/a
