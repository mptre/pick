description: page up
keys: \033[6~ \033[6~ \033[5~ \n # PAGE_DOWN PAGE_UP ENTER
env: LINES=5
stdin:
01
02
03
04
05
06
07
08
09
stdout:
05

description: page up, selected choice below the first page
keys: \033[6~ \016 \033[5~ \n  # PAGE_DOWN DOWN PAGE_UP ENTER
env: LINES=5
stdin:
01
02
03
04
05
06
stdout:
02

description: page up, all choices fit on one page
keys: \033[6~ \033[5~ \n # PAGE_DOWN PAGE_UP ENTER
env: LINES=5
stdin:
01
02
03
04
stdout:
01

description: Alt-V is an alias for page up
keys: \026 \026 \033v \n # PAGE_DOWN PAGE_UP ENTER
env: LINES=5
stdin:
01
02
03
04
05
06
07
08
09
stdout:
05
