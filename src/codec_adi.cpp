/*****************************************************************************
 * codec_adi.cpp - Analog Devices SoundMAX codec models.
 *****************************************************************************
 * Released under MIT License, see LICENSE file
 *
 * The verb lists below are the InitVerbs/RemVerbs sections of the SoundMAX
 * XP driver INF (ADIHdAud.inf, sections HdATP, HdATP4 and HdATP4A), kept in
 * the same order. The XP driver (ADIHdAud.sys, function at 0x5C29E) reads
 * them from the registry, skips zero entries, forces the codec address into
 * bits 31:28 and sends them as one batch right after the audio function
 * group is reset, before it reads any widget parameters (0x5E1DC).
 */

#include "common.h"
#include "codec.h"
#include "codec_adi.h"
#include "hdac_vendor.h"

/*****************************************************************************
 * Lenovo ThinkPad, AD1981HD (XP INF model HDA_TP / HDA_TPS)
 */
static const ULONG adi_tp_init_verbs[] =
{
	0x01E3B000,	// unmute rear mic capture node
	0x00870721,	// Mic_BIAS-B to 50%
	0x0053B03B,	// master vol -3 dB
	0x0063B03B,	// HP
	0x0153B000,	// unmute ADC
	0x00837000,	// boost amp +0 dB
	0x01237080,	// mic monitor mute
	0x01D3B03B,	// CD unmute
	0x00470600,	// ADC: stream 0
	0x00150076,	// AFG: coefficient index 0x76
	0x0014F080,	// AFG: coefficient value 0xF080
	0x00A71CA0	// SPDIF pin: config default byte 0
};

/*****************************************************************************
 * Lenovo ThinkPad, AD1984 / AD1984A (XP INF model HDA_TP4)
 */
static const ULONG adi_tp4_init_verbs[] =
{
	0x00C3B027,	// unmute ADC 0
	0x00D3B027,	// unmute ADC 1
	0x0113B017,	// port A unmute 0 dB
	0x0123B017,	// port D unmute 0 dB
	0x01470724,	// port B VREF 80%
	0x01570724,	// port C VREF 80%
	0x01537000,	// mic boost amp 0 dB
	0x00737100,	// open PC beep path
	0x00A37100,	// open PC beep path
	0x0203731F,	// analog mixer amp 0 dB
	0x0213B01F,	// unmute NID 0x21
	0x01471C20,	// ext. mic highest priority
	0x01571C2E,	// int. mic lowest priority
	0x01C71C21	// dock mic middle priority
};

/*****************************************************************************
 * Lenovo ThinkPad, AD1984A subsystem 17AA:20FB (XP INF model HDA_TP4A)
 */
static const ULONG adi_tp4a_init_verbs[] =
{
	0x00154DFD,	// AFG: coefficient index 0x0DFD
	0x00146100,	// AFG: coefficient value 0x6100
	0x01470722,	// port B (front mic) VREF 0%
	0x01170742,	// port A (headphone) VREF 0%
	0x01270742,	// port D (line out) VREF 0%
	0x00C3B027,	// ADC 0: 0 dB unmute
	0x00D3B027,	// ADC 1: 0 dB unmute
	0x0033B024,	// DAC 0 (HP): -4.5 dB unmute
	0x0043B024,	// DAC 1 (front): -4.5 dB unmute
	0x0113B000,	// port A pin unmute
	0x0123B000,	// port D pin unmute
	0x00737100,	// open PC beep path
	0x00A37100,	// open PC beep path
	0x02037317,	// analog mixer amp 0 dB
	0x0213B01F,	// analog mix (NID 0x21) unmute
	0x01437000,	// port B mic boost 0 dB
	0x01171C1F,	// port A config default byte 0
	0x01471C20,	// port B config default byte 0
	0x01271E10,	// port D config default byte 2
	0x01737001,	// DMIC 1 boost +6 dB
	0x01771C2E,	// DMIC 1 config default byte 0
	0x01771EA0	// DMIC 1 config default byte 2
};

static const ULONG adi_tp4a_rem_verbs[] =
{
	0x01171C40,	// port A config default byte 0
	0x01471C60,	// port B config default byte 0
	0x01771C20,	// digital mic config default byte 0
	0x01271E17,	// port D config default byte 2
	0x01771EA6	// DMIC 1 config default byte 2
};

// 16/20/24-bit, 44.1/48 kHz (XP INF: HKR,AD1984A,"DAC0"/"DAC1",0x000E0060)
#define ADI_TP4A_DAC_PCM_MASK 0x000E0060

#define VERBS(a) a, sizeof(a) / sizeof((a)[0])
#define NO_VERBS NULL, 0

static const ADI_MODEL adi_models[] =
{
	{ HDA_CODEC_AD1981HD, 0x17AA2008, "ThinkPad AD1981HD", VERBS(adi_tp_init_verbs), NO_VERBS, 0, 0, FALSE },
	{ HDA_CODEC_AD1981HD, 0x17AA2025, "ThinkPad AD1981HD", VERBS(adi_tp_init_verbs), NO_VERBS, 0, 0, FALSE },
	{ HDA_CODEC_AD1981HD, 0x17AA2060, "ThinkPad AD1981HD", VERBS(adi_tp_init_verbs), NO_VERBS, 0, 0, FALSE },
	{ HDA_CODEC_AD1981HD, 0x17AA2080, "ThinkPad AD1981HD", VERBS(adi_tp_init_verbs), NO_VERBS, 0, 0, FALSE },

	{ HDA_CODEC_AD1984,   0x17AA20BB, "ThinkPad AD1984",  VERBS(adi_tp4_init_verbs), NO_VERBS, 0, 0, FALSE },
	{ HDA_CODEC_AD1984,   0x17AA20D6, "ThinkPad AD1984",  VERBS(adi_tp4_init_verbs), NO_VERBS, 0, 0, FALSE },
	{ HDA_CODEC_AD1984,   0x17AA20D7, "ThinkPad AD1984",  VERBS(adi_tp4_init_verbs), NO_VERBS, 0, 0, FALSE },
	{ HDA_CODEC_AD1984,   0x17AA20DD, "ThinkPad AD1984",  VERBS(adi_tp4_init_verbs), NO_VERBS, 0, 0, FALSE },
	{ HDA_CODEC_AD1984A,  0x17AA20BB, "ThinkPad AD1984A", VERBS(adi_tp4_init_verbs), NO_VERBS, 0, 0, FALSE },
	{ HDA_CODEC_AD1984A,  0x17AA20D6, "ThinkPad AD1984A", VERBS(adi_tp4_init_verbs), NO_VERBS, 0, 0, FALSE },
	{ HDA_CODEC_AD1984A,  0x17AA20D7, "ThinkPad AD1984A", VERBS(adi_tp4_init_verbs), NO_VERBS, 0, 0, FALSE },
	{ HDA_CODEC_AD1984A,  0x17AA20DD, "ThinkPad AD1984A", VERBS(adi_tp4_init_verbs), NO_VERBS, 0, 0, FALSE },

	{ HDA_CODEC_AD1984A,  0x17AA20FB, "ThinkPad AD1984A", VERBS(adi_tp4a_init_verbs),
	  VERBS(adi_tp4a_rem_verbs), ADI_TP4A_DAC_PCM_MASK, 0x12, TRUE },
};

const ADI_MODEL* AdiFindModel(ULONG codec_id, ULONG subsystem_id)
{
	for (ULONG i = 0; i < sizeof(adi_models) / sizeof(adi_models[0]); ++i) {
		if (adi_models[i].codec_id == codec_id &&
			(adi_models[i].subsystem_id == subsystem_id ||
			 adi_models[i].subsystem_id == HDA_MATCH_ALL)) {
			return &adi_models[i];
		}
	}
	return NULL;
}
