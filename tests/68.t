description: do not match inside a OSC escape sequence
keys: example \n # ENTER
stdin:
\033]8;;example.com\aOSC Hyperlink\e]8;;\a
favored match since the query is not inside the escape sequence example.com
stdout:
favored match since the query is not inside the escape sequence example.com
