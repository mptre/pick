description: delete words
keys: \027 ab\\ cd \027 \027 \n # CTRL_W ENTER
stdin:
ab
ab cd
stdout:
ab

description: UTF-8 delete word
keys: aa\\ Åå \027 aa\\ aa \n # CTRL_W ENTER
stdin:
aa Åå aa
aa Åå aa aa
stdout:
aa Åå aa aa

description: non alphanumeric characters are not recognized as part of a word
keys: a/a \027 a/a \n # CTRL_W ENTER
stdin:
a/a
a/a/a
stdout:
a/a/a

description: underscore is recognized as part of a word
keys: aa_aa \027 bb \n # CTRL_W ENTER
stdin:
aa_aa_bb
bb
stdout:
bb

description: Alt-Backspace is an alias for CTRL_W
keys: aa\\ Åå \033\b aa\\ aa \n # CTRL_W ENTER
stdin:
aa Åå aa
aa Åå aa aa
stdout:
aa Åå aa aa
