"""Decode the StateInit/StateStream codec snapshots that hda98 writes into
the registry (HDA_Codec::DumpState) from a REGEDIT4 export.
Usage: decode_state.py <export.reg>"""
import re, struct, sys

TYPES = ['DAC', 'ADC', 'MIX', 'SEL', 'PIN', 'PWR', 'KNOB', 'BEEP'] + ['?'] * 7 + ['VEND']
DEVICES = ['LineOut', 'Speaker', 'HP', 'CD', 'SPDIFout', 'DigOut', 'ModemLine', 'ModemHand',
           'LineIn', 'AUX', 'Mic', 'Telephony', 'SPDIFin', 'DigIn', 'Reserved', 'Other']
CONN = ['Jack', 'None', 'Fixed', 'Both']


def values(text):
    text = text.replace('\\\r\n', '').replace('\\\n', '')
    key = None
    for line in text.splitlines():
        if line.startswith('['):
            key = line.strip('[]')
        m = re.match(r'"(State\w+)"=hex:(.*)', line.strip())
        if m:
            data = bytes(int(b, 16) for b in m.group(2).replace(' ', '').split(',') if b)
            yield key, m.group(1), data


def amp(v):
    return ('M' if v & 0x80 else '') + '%02x' % (v & 0x7F)


def decode(data):
    d = struct.unpack('<%dI' % (len(data) // 4), data)
    if len(d) < 48 or d[0] != 0x44414448:
        print('  not a snapshot'); return
    print('  codec %08x subsys %08x soundmax-model=%d afg=%02x nodes %02x..%02x' %
          (d[2], d[3], d[4], d[5], d[6], d[6] + d[7] - 1))
    print('  paths=%d output pin=%02x hp pin=%02x selected=%02x hp present=%d' % tuple(d[8:13]))
    for i in range(8):
        p = d[16 + 4 * i:20 + 4 * i]
        if i < d[8]:
            print('  path %d: %-8s dac=%02x vol amp=%02x mute amp=%02x' % (i, DEVICES[p[0] & 15], p[1], p[2], p[3]))
    o = 16 + 32
    while o + 14 <= len(d):
        w = d[o:o + 14]; o += 14
        nid, typ = w[0] & 0xFF, (w[0] >> 8) & 0xF
        s = '  %02x %-4s caps=%08x ' % (nid, TYPES[typ], w[1])
        if typ == 4:
            cfg = w[2]
            s += 'cfg=%08x(%s,%s) ctl=%02x eapd=%x sense=%08x ' % (
                cfg, CONN[cfg >> 30], DEVICES[(cfg >> 20) & 15], w[3], w[4], w[5])
        elif typ in (0, 1):
            s += 'fmt=%04x stream=%02x ' % (w[2], w[3])
        s += 'out=%s/%s in0=%s/%s sel=%d conn=%08x pwr=%02x' % (
            amp(w[6]), amp(w[7]), amp(w[8]), amp(w[9]), w[10], w[11], w[12])
        print(s)


text = open(sys.argv[1], 'rb').read().decode('latin-1')
for key, name, data in values(text):
    print('%s  %s' % (name, key))
    decode(data)
