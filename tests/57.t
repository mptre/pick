description: delete word until slash
keys: aa/bc \027 bb \n # CTRL_W ENTER
stdin:
aa/bb
aa/bc
stdout:
aa/bb
