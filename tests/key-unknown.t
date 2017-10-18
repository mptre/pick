description: non printable characters are not added to the query
keys: b \033[1337~ \033\n # PAGE_UP ALT_ENTER
stdin:
a
stdout:
b

description: ignore unrecgonized escape sequence
keys: \033[1111111111111111~ \n # ENTER
stdin:
a
stdout:
a
