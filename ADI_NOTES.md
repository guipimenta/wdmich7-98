# SoundMAX (Analog Devices) support: what came from the XP driver

Source: the SoundMAX XP driver package (`SMAXWDM\W2K_XP\ADIHdAud.sys`
5.10.01.5710, built 2008-01-11, image base 0x10000, and `ADIHdAud.inf`). It is
not included here. The disassembly scripts are in `tools/re/` (run
`disasm.py ADIHdAud.sys out.asm` to recreate the listing). Addresses below are
virtual addresses in that image.

## Codec classes
`sub_15322` picks a codec class by vendor 0x11D4 and device ID:

| Device | Class ctor | Codec table |
|---|---|---|
| 0x1986 | sub_1458e | |
| 0x1983 | sub_14896 | |
| 0x1981 (AD1981HD) | sub_149fc -> sub_131f6 | 0x1E9E8 |
| 0x1988/0x198B/0x1987/0x1882 | sub_14ca8 | |
| 0x1984/0x1884/0x194A/0x194B/0x1883/0x184A | sub_14e8c -> sub_13378 (`ADI1984Codec`) | 0x1EFB0 |

The codec tables hold an ADI-internal widget numbering. For AD1984 it is the
NID itself. For AD1981HD it is a remap. The port-role table gives AD1984 ports
A=0x11, B=0x14, C=0x15, D=0x12, E=0x1C, F=0x16.

## Init sequence (codec base, sub_5e1dc)
1. `InitVerbs` from the driver registry key is sent first (sub_5c29e): read
   `NumVerbs`, then values named `%04d` (decimal). Zero values are skipped.
   Bits 31:28 are replaced with the codec address. All are sent as one batch.
2. Only then are the codec parameters read and the widget tree built.
3. `RemVerbs` are sent the same way from the codec destructor (sub_5dd9c),
   i.e. at unload. The TP4A list restores the BIOS pin config defaults.

## Converter format limits
`HKR,<codec>,DAC0/DAC1/ADC0/ADC1` (table at 0x223A4, sub_5d464) is ANDed into
the converter's PCM parameter (sub_30074: `caps &= value`). TP4A uses
0x000E0060, which allows 16/20/24-bit at 44.1/48 kHz only.

## EAPD / power (sub_2d6a8, sub_3077c)
`PowerSettings\PM_EAPD_Mode` sets bits 11:8 of the power policy (default 7).
`PM_EAPD_Polarity` != 0 means EAPD high = amplifier on. On a pin powered up
and unmuted, EAPD is driven active. The pin named by
`PM_EAPD_PinPowerStateCtrl` keeps EAPD set and is powered down with verb 0x705
instead. For TP4A that pin is 0x12 (port D, internal speaker).

## Models (from the XP INF, keyed by codec subsystem ID)
| INF model | Codec | Subsystems | Init verbs | Extra |
|---|---|---|---|---|
| HDA_TP / HDA_TPS | AD1981HD | 17AA:2008, 2025, 2060, 2080 | 12 | |
| HDA_TP4 | AD1984, AD1984A | 17AA:20BB, 20D6, 20D7, 20DD | 14 | |
| HDA_TP4A | AD1984A | 17AA:20FB | 22 | 5 RemVerbs, DAC mask 0x000E0060, EAPD pin 0x12 |

## How the Win98 driver uses this (src/codec_adi.cpp)
- `AdiFindModel()` matches codec ID + AFG subsystem ID.
- Init verbs go out right after the AFG reset/power-up and subsystem read,
  before WDMHDA parses the widgets, matching the XP order (some verbs rewrite
  pin config defaults).
- The DAC mask is applied to every output path's PCM caps.
- The EAPD pin gets EAPD asserted after the paths are set up.
- RemVerbs are sent from the adapter destructor before the codecs shut down.
- `HKR,InitVerbs` / `HKR,RemVerbs` in the driver key (same format as the XP
  INF) replace the built-in lists, so XP INF lines can be pasted unchanged.

Not ported (XP-only features): AEAudio effects, SoundMAX control panel IOCTLs,
DRM, digital-mic array processing, recording (WDMHDA is playback-only).
