description: backspace deletes the ASCII character behind the cursor
keys: abb \b \b c \n # BACKSPACE ENTER
stdin:
ab
ac
stdout:
ac

description: backspace deletes the UTF-8 character behind the cursor
keys: ö \b \n # BACKSPACE ENTER
stdin:
å
ä
ö
stdout:
å

description: del is an alias for backspace
keys: a \177 c \n # BACKSPACE ENTER
stdin:
a
c
stdout:
c
