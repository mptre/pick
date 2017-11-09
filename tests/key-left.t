description: left moves the cursor to the left
keys: a \002 b \n # LEFT ENTER
stdin:
ba
stdout:
ba

description: left moves the cursor to the left and handles UTF-8
keys: åaå \002 \002 \002 ö \n # LEFT LEFT LEFT ENTER
stdin:
öåaå
stdout:
öåaå

description: left arrow is an alias for left
keys: a \033OD b \n # LEFT ENTER
stdin:
ba
stdout:
ba
