description: up with vertical scroll
keys: \016 \016 \016 \016 \020 \020 \020 \020 \n # DOWN UP ENTER
env: LINES=5
stdin:
01
02
03
04
05
stdout:
01

description: up out of bounds with vertical scroll
keys: \016 \016 \016 \016 \020 \020 \020 \020 \020 \n # DOWN UP ENTER
env: LINES=5
stdin:
01
02
03
04
05
stdout:
01

description: up arrow is an alias for up
keys: \016 \016 \016 \016 \020 \eOA \eOA \eOA \n # DOWN UP ENTER
env: LINES=5
stdin:
01
02
03
04
05
stdout:
01
