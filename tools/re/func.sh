#!/bin/sh
# Print the whole function containing address $2 from asm $1
awk -v a="$2" '
/^sub_/ {buf=$0"\n"; next}
/^[0-9a-f]{8}  / {buf=buf $0 "\n"; if (substr($1,1,8)==sprintf("%08s",a)) hit=1; next}
/^$/ { if (hit) {printf "%s", buf; exit} }
' "$1"
