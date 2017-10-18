description: page down scrolls a page worth of choices
keys: \033[6~ \n # PAGE_DOWN ENTER
env: LINES=5
stdin:
01
02
03
04
05
stdout:
05

description: page down, first choice not selected
keys: \016 \033[6~ \n # DOWN PAGE_DOWN ENTER
env: LINES=5
stdin:
01
02
03
04
05
06
stdout:
06

description: page down, all choices fit on one page
keys: \033[6~ \n # PAGE_DOWN ENTER
env: LINES=5
stdin:
01
02
03
04
stdout:
04

description: Ctrl-V is an alias for page down
keys: \026 \n # PAGE_DOWN ENTER
env: LINES=5
stdin:
01
02
03
04
05
stdout:
05

description: Alt-Space alias for PAGE_DOWN
keys: \033\\  \n # PAGE_DOWN ENTER
env: LINES=5
stdin:
01
02
03
04
05
stdout:
05
