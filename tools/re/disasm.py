"""Linear-sweep disassembly of a PE driver's code sections with import, string
and function-start annotations. Usage: disasm.py <pe> <out.asm>"""
import sys, re, pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_32

pe = pefile.PE(sys.argv[1])
base = pe.OPTIONAL_HEADER.ImageBase
img = pe.get_memory_mapped_image()

imports = {}
for d in pe.DIRECTORY_ENTRY_IMPORT:
    for i in d.imports:
        imports[i.address] = (i.name or b'#%d' % i.ordinal).decode()

strings = {}
for m in re.finditer(rb'[\x20-\x7e]{4,}\x00', img):
    strings[base + m.start()] = '"%s"' % m.group()[:-1].decode()
for m in re.finditer(rb'(?:[\x20-\x7e]\x00){3,}\x00\x00', img):
    strings[base + m.start()] = 'L"%s"' % m.group()[:-2].decode('utf-16le')

md = Cs(CS_ARCH_X86, CS_MODE_32)
md.skipdata = True
code_secs = [s for s in pe.sections if s.Characteristics & 0x20000000]

# first pass: call targets, plus code pointers stored in data (vtables, dispatch tables)
targets = set()
code_ranges = [(base + c.VirtualAddress, base + c.VirtualAddress + c.Misc_VirtualSize) for c in code_secs]
import struct
if hasattr(pe, 'DIRECTORY_ENTRY_BASERELOC'):
    for blk in pe.DIRECTORY_ENTRY_BASERELOC:
        for e in blk.entries:
            if e.type != 3: continue
            sec = pe.get_section_by_rva(e.rva)
            if sec is None or sec.Characteristics & 0x20000000: continue
            v = struct.unpack_from('<I', img, e.rva)[0]
            if any(a <= v < b for a, b in code_ranges): targets.add(v)
insns = []
for s in code_secs:
    start = s.VirtualAddress
    code = img[start:start + s.Misc_VirtualSize]
    for ins in md.disasm(code, base + start):
        insns.append((s.Name.rstrip(b'\0').decode(), ins))
        if ins.mnemonic == 'call' and ins.op_str.startswith('0x'):
            targets.add(int(ins.op_str, 16))

def annotate(op):
    notes = []
    for v in re.findall(r'0x[0-9a-f]+', op):
        n = int(v, 16)
        if n in imports: notes.append(imports[n])
        elif n in strings: notes.append(strings[n][:80])
        elif n in targets: notes.append('sub_%x' % n)
    return notes

with open(sys.argv[2], 'w') as f:
    cur = None
    for sec, ins in insns:
        if sec != cur:
            f.write('\n;;;;; section %s\n' % sec); cur = sec
        if ins.address in targets:
            f.write('\nsub_%x:\n' % ins.address)
        n = annotate(ins.op_str)
        f.write('%08x  %-7s %s%s\n' % (ins.address, ins.mnemonic, ins.op_str,
                ('   ; ' + ', '.join(n)) if n else ''))
print(len(insns), 'instructions,', len(targets), 'call targets')
