"""Dump sections, imports and strings (ASCII + UTF-16LE) with RVAs from a PE file."""
import sys, re, pefile

pe = pefile.PE(sys.argv[1])
base = pe.OPTIONAL_HEADER.ImageBase
print("ImageBase %08x" % base)
for s in pe.sections:
    print("SECTION %-8s rva=%06x vsz=%06x raw=%06x rsz=%06x ch=%08x" % (
        s.Name.rstrip(b'\0').decode(), s.VirtualAddress, s.Misc_VirtualSize,
        s.PointerToRawData, s.SizeOfRawData, s.Characteristics))
for d in pe.DIRECTORY_ENTRY_IMPORT:
    for i in d.imports:
        print("IMPORT %08x %s!%s" % (i.address, d.dll.decode(), i.name.decode() if i.name else "#%d" % i.ordinal))

data = pe.get_memory_mapped_image()
for m in re.finditer(rb'[\x20-\x7e]{5,}\x00', data):
    print("A %06x %s" % (m.start(), m.group()[:-1].decode()))
for m in re.finditer(rb'(?:[\x20-\x7e]\x00){4,}\x00\x00', data):
    print("W %06x %s" % (m.start(), m.group()[:-2].decode('utf-16le')))
