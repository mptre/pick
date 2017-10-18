description: backspace deletes the ASCII character behind the cursor
keys: abb \177 \b c \n ## BACKSPACE BACKSPACE ENTER
stdin:
ab
ac
stdout:
ac

description: backspace deletes the UTF-8 character behind the cursor
keys: ö \177 \n # BACKSPACE ENTER
stdin:
å
ä
ö
stdout:
å
