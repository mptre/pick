description: enter selects the first choice
keys: \n # ENTER
stdin:
1
2
3
4
stdout:
1

description: enter does nothing if no choices are present
keys: 2 \n \033\n # ENTER ALT_ENTER
stdin:
1
stdout:
2

description: tabs in input
keys: \n # ENTER
stdin:
	a
stdout:
	a

description: carriage return is recognized as enter
keys: \r # ENTER
stdin:
a
stdout:
a
