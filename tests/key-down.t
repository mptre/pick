description: down selects the last choice
keys: \016 \016 \016 \016 \n # DOWN ENTER
stdin:
1
2
3
4
stdout:
4

description: down out of bounds
keys: \016 \016 \016 \n # DOWN ENTER
stdin:
1
2
stdout:
2

description: down arrow is an alias for down
keys: \033OB \n # DOWN ENTER
stdin:
1
2
3
4
stdout:
2

description: down with vertical scroll
keys: \016 \016 \016 \016 \n # DOWN ENTER
env: LINES=5
stdin:
01
02
03
04
05
stdout:
05

description: down out of bounds with vertical scroll
keys: \016 \016 \016 \016 \016 \n # DOWN ENTER
env: LINES=5
stdin:
01
02
03
04
05
stdout:
05
