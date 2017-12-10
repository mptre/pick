description: right moves the cursor to the right
keys: a \002 b \006 b \n # LEFT RIGHT ENTER
stdin:
bab
stdout:
bab

description: right moves the cursor to the right and handles UTF-8
keys: åaå \001 \006 \006 \006 ö \n # CTRL_A RIGHT RIGHT RIGHT ENTER
stdin:
åaåö
stdout:
åaåö

description: right arrow is an alias for right
keys: a \002 b \033OC b \n # LEFT RIGHT ENTER
stdin:
bab
stdout:
bab

description: right end of line
keys: \006 \n # RIGHT ENTER
stdin:
a
stdout:
a
