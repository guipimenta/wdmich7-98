/*****************************************************************************
 * codec_adi.h - Analog Devices SoundMAX codec models.
 *****************************************************************************
 * Released under MIT License, see LICENSE file
 *
 * Per-model settings recovered from the Analog Devices SoundMAX XP driver
 * (ADIHdAud.sys 5.10.01.5710 and its ADIHdAud.inf). See ADI_NOTES.md.
 */

#ifndef _CODEC_ADI_H_
#define _CODEC_ADI_H_

typedef struct _ADI_MODEL
{
	ULONG		codec_id;		// vendor/device from parameter 0 of node 0
	ULONG		subsystem_id;	// AFG subsystem ID (verb F20), HDA_MATCH_ALL for any
	const char*	name;

	// Raw 32-bit codec commands (address bits 31:28 are replaced with the
	// codec's own address), sent right after the AFG is reset and powered,
	// before the widget tree is parsed. Same format as the XP INF's
	// HKR,InitVerbs entries.
	const ULONG* init_verbs;
	ULONG		num_init_verbs;

	// Commands sent when the driver is unloaded (XP: HKR,RemVerbs, sent from
	// the codec destructor). Used to restore pin configuration defaults.
	const ULONG* rem_verbs;
	ULONG		num_rem_verbs;

	// Mask ANDed into the PCM size/rate parameter (0x0A) of every output
	// converter in use. 0 = no restriction. (XP: HKR,<codec>,DAC0/DAC1)
	ULONG		dac_pcm_mask;

	// Pin whose EAPD must stay asserted while the driver is loaded
	// (XP: PowerSettings\PM_EAPD_PinPowerStateCtrl). 0 = none.
	ULONG		eapd_pin;

	// Turn the speaker pin off while headphones are plugged in. The XP INF
	// sets this only for HDA_TP4A (FrontShutsOffInternalSpeaker=1); HDA_TP
	// keeps HpShutsOffRearSpeaker=0 and HDA_TP4 sets
	// DisableHpShutsOffRearSpeaker=1.
	BOOLEAN		hp_mutes_speaker;
} ADI_MODEL;

const ADI_MODEL* AdiFindModel(ULONG codec_id, ULONG subsystem_id);

#endif // _CODEC_ADI_H_
