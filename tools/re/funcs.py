"""Index functions in the annotated asm: start, instruction count, callees,
string/import refs. Usage: funcs.py <asm> [addr_hex ...]"""
import sys, re
lines = open(sys.argv[1]).read().splitlines()
funcs, cur = {}, None
for l in lines:
    if l.startswith('sub_'):
        cur = l[4:-1]; funcs[cur] = {'n': 0, 'calls': [], 'refs': []}
        continue
    if cur is None or not re.match(r'[0-9a-f]{8}  ', l):
        continue
    f = funcs[cur]; f['n'] += 1
    m = re.search(r'call\s+0x([0-9a-f]+)', l)
    if m: f['calls'].append(m.group(1))
    m = re.search(r';\s+(.*)$', l)
    if m and not m.group(1).startswith('sub_'): f['refs'].append(m.group(1))
want = sys.argv[2:] or sorted(funcs)
for a in want:
    f = funcs.get(a)
    if not f: print(a, 'not a call target'); continue
    print('sub_%s n=%d calls=%s refs=%s' % (a, f['n'], ','.join(dict.fromkeys(f['calls'])), ' | '.join(dict.fromkeys(f['refs']))[:300]))
