description: move backward and use DEL key to remove character under cursor
keys: ab \002 \033[3~ c \\n
stdin:
ab
ac
stdout:
ac
