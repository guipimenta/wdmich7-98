"""Dump dwords at a VA range of a PE image. Usage: dumpva.py <pe> <va_hex> <count_dwords>"""
import sys, struct, pefile
pe = pefile.PE(sys.argv[1]); base = pe.OPTIONAL_HEADER.ImageBase
img = pe.get_memory_mapped_image()
va, n = int(sys.argv[2], 16), int(sys.argv[3])
for k in range(0, n, 8):
    off = va - base + 4 * k
    ws = struct.unpack_from('<%dI' % min(8, n - k), img, off)
    print('%08x: ' % (va + 4 * k) + ' '.join('%08x' % w for w in ws))
