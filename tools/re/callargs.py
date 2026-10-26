"""For each call to a target in the annotated asm, print the enclosing function
and the operands of the N pushes preceding it (first pushed = last arg).
Usage: callargs.py <asm> <target_hex> <nargs>"""
import sys, re
asm, target, nargs = sys.argv[1], sys.argv[2].lower(), int(sys.argv[3])
lines = open(asm).read().splitlines()
func = '?'
for i, l in enumerate(lines):
    if l.startswith('sub_'):
        func = l[:-1]
    m = re.match(r'([0-9a-f]{8})  call\s+0x' + target + r'\b', l)
    if not m:
        continue
    pushes, j = [], i - 1
    while j > 0 and len(pushes) < nargs and i - j < 40:
        p = re.match(r'[0-9a-f]{8}  push\s+(.*?)(\s+;.*)?$', lines[j])
        if p:
            pushes.append(p.group(1))
        if lines[j].startswith('sub_'):
            break
        j -= 1
    print('%s %-10s args(%s)' % (m.group(1), func, ', '.join(pushes)))
