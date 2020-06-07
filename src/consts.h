#ifndef CONSTS_H
#define CONSTS_H

// KeyMaster constants

#define UNDEFINED (-1)
#define UNDEFINED_ID (-1LL)
#define CONNECTION_ALL_CHANNELS (-1)
#define CONTROLLER_BLOCK (-1)

// MIDI constants

// Number of MIDI channels
#define MIDI_CHANNELS 16
// Number of note per MIDI channel
#define NOTES_PER_CHANNEL 128

// Channel messages

// Note, val
#define NOTE_OFF 0x80
// Note, val
#define NOTE_ON 0x90
// Note, val
#define POLY_PRESSURE 0xA0
// Controller #, val
#define CONTROLLER 0xB0
// Program number
#define PROGRAM_CHANGE 0xC0
// Channel pressure
#define CHANNEL_PRESSURE 0xD0
// LSB, MSB
#define PITCH_BEND 0xE0

// System common messages

// System exclusive start
#define SYSEX 0xF0
// Beats from top: LSB/MSB 6 ticks 1 beat
#define SONG_POINTER 0xF2
// Val number of song
#define SONG_SELECT 0xF3
// Tune request
#define TUNE_REQUEST 0xF6
// End of system exclusive
#define EOX 0xF7

// System realtime messages
// MIDI clock (24 per quarter note)
#define CLOCK 0xF8
// Sequence start
#define START 0xFA
// Sequence continue
#define CONTINUE 0xFB
// Sequence stop
#define STOP 0xFC
// Active sensing (sent every 300 ms when nothing else being sent)
#define ACTIVE_SENSE 0xFE
// System reset
#define SYSTEM_RESET 0xFF

// Standard MIDI File meta event defs.
#define META_EVENT 0xff
#define META_SEQ_NUM 0x00
#define META_TEXT 0x01
#define META_COPYRIGHT 0x02
#define META_SEQ_NAME 0x03
#define META_INSTRUMENT 0x04
#define META_LYRIC 0x05
#define META_MARKER 0x06
#define META_CUE 0x07
#define META_MIDI_CHAN_PREFIX 0x20
#define META_TRACK_END 0x2f
#define META_SET_TEMPO 0x51
#define META_SMPTE 0x54
#define META_TIME_SIG 0x58
#define META_PATCH_SIG 0x59
#define META_SEQ_SPECIF 0x7f

// Controller numbers
//    0 - 31 = continuous, MSB
//   32 - 63 = continuous, LSB
//   64 - 97 = momentary switches
#define CC_BANK_SELECT_MSB 0
#define CC_BANK_SELECT CC_BANK_SELECT_MSB
#define CC_MOD_WHEEL_MSB 1
#define CC_MOD_WHEEL CC_MOD_WHEEL_MSB
#define CC_BREATH_CONTROLLER_MSB 2
#define CC_BREATH_CONTROLLER CC_BREATH_CONTROLLER_MSB
#define CC_FOOT_CONTROLLER_MSB 4
#define CC_FOOT_CONTROLLER CC_FOOT_CONTROLLER_MSB
#define CC_PORTAMENTO_TIME_MSB 5
#define CC_PORTAMENTO_TIME CC_PORTAMENTO_TIME_MSB
#define CC_DATA_ENTRY_MSB 6
#define CC_DATA_ENTRY CC_DATA_ENTRY_MSB
#define CC_VOLUME_MSB 7
#define CC_VOLUME CC_VOLUME_MSB
#define CC_BALANCE_MSB 8
#define CC_BALANCE CC_BALANCE_MSB
#define CC_PAN_MSB 10
#define CC_PAN CC_PAN_MSB
#define CC_EXPRESSION_CONTROLLER_MSB 11
#define CC_EXPRESSION_CONTROLLER CC_EXPRESSION_CONTROLLER_MSB
#define CC_GEN_PURPOSE_1_MSB 16
#define CC_GEN_PURPOSE_1 CC_GEN_PURPOSE_1_MSB
#define CC_GEN_PURPOSE_2_MSB 17
#define CC_GEN_PURPOSE_2 CC_GEN_PURPOSE_2_MSB
#define CC_GEN_PURPOSE_3_MSB 18
#define CC_GEN_PURPOSE_3 CC_GEN_PURPOSE_3_MSB
#define CC_GEN_PURPOSE_4_MSB 19
#define CC_GEN_PURPOSE_4 CC_GEN_PURPOSE_4_MSB

// [32 - 63] are LSB for [0 - 31]
#define CC_BANK_SELECT_LSB (CC_BANK_SELECT_MSB + 32)
#define CC_MOD_WHEEL_LSB (CC_MOD_WHEEL_MSB + 32)
#define CC_BREATH_CONTROLLER_LSB (CC_BREATH_CONTROLLER_MSB + 32)
#define CC_FOOT_CONTROLLER_LSB (CC_FOOT_CONTROLLER_MSB + 32)
#define CC_PORTAMENTO_TIME_LSB (CC_PORTAMENTO_TIME_MSB + 32)
#define CC_DATA_ENTRY_LSB (CC_DATA_ENTRY_MSB + 32)
#define CC_VOLUME_LSB (CC_VOLUME_MSB + 32)
#define CC_BALANCE_LSB (CC_BALANCE_MSB + 32)
#define CC_PAN_LSB (CC_PAN_MSB + 32)
#define CC_EXPRESSION_CONTROLLER_LSB (CC_EXPRESSION_CONTROLLER_MSB + 32)
#define CC_GEN_PURPOSE_1_LSB (CC_GEN_PURPOSE_1_MSB + 32)
#define CC_GEN_PURPOSE_2_LSB (CC_GEN_PURPOSE_2_MSB + 32)
#define CC_GEN_PURPOSE_3_LSB (CC_GEN_PURPOSE_3_MSB + 32)
#define CC_GEN_PURPOSE_4_LSB (CC_GEN_PURPOSE_4_MSB + 32)

// Momentary MSB switches
#define CC_SUSTAIN 64
#define CC_PORTAMENTO 65
#define CC_SUSTENUTO 66
#define CC_SOFT_PEDAL 67
#define CC_HOLD_2 69
#define CC_GEN_PURPOSE_5 50
#define CC_GEN_PURPOSE_6 51
#define CC_GEN_PURPOSE_7 52
#define CC_GEN_PURPOSE_8 53
#define CC_EXT_EFFECTS_DEPTH 91
#define CC_TREMELO_DEPTH 92
#define CC_CHORUS_DEPTH 93
#define CC_DETUNE_DEPTH 94
#define CC_PHASER_DEPTH 95
#define CC_DATA_INCREMENT 96
#define CC_DATA_DECREMENT 97
#define CC_NREG_PARAM_LSB 98
#define CC_NREG_PARAM_MSB 99
#define CC_REG_PARAM_LSB 100
#define CC_REG_PARAM_MSB 101

// Channel mode message values
// Val 0 == off, 0x7f == on
#define CM_RESET_ALL_CONTROLLERS 0x79
#define CM_LOCAL_CONTROL 0x7A
// Val must be 0
#define CM_ALL_NOTES_OFF 0x7B
// Val must be 0
#define CM_OMNI_MODE_OFF 0x7C
// Val must be 0
#define CM_OMNI_MODE_ON 0x7D
// Val = # chans
#define CM_MONO_MODE_ON 0x7E
// Val must be 0
#define CM_POLY_MODE_ON 0x7F

#endif /* CONSTS_H */
