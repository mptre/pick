description: delete removes the character under cursor
keys: ab \002 \004 c \n # LEFT DEL ENTER
stdin:
ab
ac
stdout:
ac

description: UTF-8 delete
keys: aå \002 \004 a \n # LEFT DEL ENTER
stdin:
aa
aå
stdout:
aa

description: delete key is an alias for delete
keys: ab \002 \033[3~ c \n # LEFT DEL ENTER
stdin:
ab
ac
stdout:
ac
