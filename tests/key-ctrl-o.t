description: Ctrl-O toggles sorting
keys: foo \017 \n # CTRL_O ENTER
stdin:
foo bar
foo
stdout:
foo bar

description: it works in conjuction with the -S option
args: -S
keys: foo \017 \n # CTRL_O ENTER
stdin:
foo bar
foo
stdout:
foo
