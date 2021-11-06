─ │ ┌ ┐ └ ┘ ├ ┤ ┬ ┴ ┼
# ---TRACKER'S FILE FORMAT SPECIFICATION---

this document should help you read the soundtracker file format.

## Header, 384 bytes
---- -0- -1- -2- -3- -4- -5- -6- -7- -8- -9- -A- -B- -C- -D- -E- -F-
    ┌───┬───┬───┬───┬───┬───┬───┬───┬───────┬───┬───┬───┬───┬───┬───┐
  00│ T │ R │ A │ C │ K │ 8 │ B │ T │Version│Ins│Pat│Ord│Spd│Seq│Tmp│
    ├───┴───┴───┴───┴───┴───┴───┴───┴───────┴───┴───┴───┴───┴───┴───┤
  10│ song name, 32 chars max, padded with 0x00s                    │
    ├───┬───┬───────┬───┬───┬───────────────┬───────────────┬───┬───┤
  30│DFM│#CH│ flags │gvl│gpn│  PCM pointer  │Comment pointer│DFA│Len│
    ├───┴───┴───────┴───┴───┴───────────────┴───────────────┴───┴───┤
  40│ default volume per channel, 32 bytes                          │
    ├───────────────────────────────────────────────────────────────┤
  60│ default panning per channel, 32 bytes                         │
    ├───────────────────────────────────────────────────────────────┤
  80│ order list, 256 bytes                                         │
    ├───────────────────────────────────────────────────────────────┤
 180│ pointers to instruments, 4 bytes per pointer                  │
    ├───────────────────────────────────────────────────────────────┤
xxxx│ pointers to macros or sequence tables (pre-r152), 4 bytes/ptr │
    ├───────────────────────────────────────────────────────────────┤
xxxx│ pointers to patterns, 4 bytes per pointer                     │
    └───────────────────────────────────────────────────────────────┘
    Note: DFA: detune
          Flags: bit 0: NTSC
                 bit 1: NTSC50
                 bit 2: noise compatibility mode
                        if set then noise frequency is divided by 4.

## Macros, new format
read macros like this:

1. read the macro length (4 bytes).
2. read the "jump on release" position (4 bytes).
  * if this is -1, then it is "don't jump".
3. read the macro intended use (1 byte):
  - 0: generic
  - 1: shape
  - 2: pitch
  - 3: panning
  - 4: volume sweep
  - 5: other sweep
3. skip 7 bytes (reserved section).
4. read the type. then switch based on it:
  - 0: end of macro.
  - 1: set. read four bytes for value.
  - 2: wait. read four bytes for length.
  - 3: wait for release.
  - 4: loop. read the position (4 bytes).
  - 5: loop until release. read the position (4 bytes).
  - 6: add. read four bytes for value.
  - 7: subtract. read four bytes for value.
  * if bit 7 is set, then it means end of tick (ignored on wait).
 
## Sequence tables, legacy (pre-r152) format, 2048 bytes each
---- -0- -1- -2- -3- -4- -5- -6- -7- -8- -9- -A- -B- -C- -D- -E- -F-
    ┌───────────────────────────────────────────────────────────────┐
  00│ main sequence, 253 bytes                                      │
    ├───────────────────────────────────────────────────┬───┬───┬───┤
  F0│                                                   │Len│Lps│Rps│
    └───────────────────────────────────────────────────┴───┴───┴───┘

read 8 times to get the tables in this order:
- 0: volume
- 1: cutoff
- 2: resonance
- 3: duty
- 4: shape
- 5: pitch
- 6: fine pitch
- 7: panning
    
## Instruments, new format, 96 bytes each
---- -0- -1- -2- -3- -4- -5- -6- -7- -8- -9- -A- -B- -C- -D- -E- -F-
    ┌───────────────────────────────────────────────────────────────┐
  00│ instrument name, 32 bytes, padded with 0x00s                  │
    ├───┬───┬───────┬───────┬───────┬───────┬───┬───┬───┬───┬───┬───┤
  20│Ins│???│volumeM│cutoffM│resonaM│pitchM │???│Off│FPt│FPR│DFM│LFO│
    ├───┼───┼───────┼───────┼───┬───┴───┬───┴───┼───┼───┴───┼───┼───┤
  30│Vol│Pit│PCMSlen│FilterH│Res│FTm│PCMSptr│PCMloop│Version│flg│RMF│
    ├───┴───┼───────┼───────┼───┴───┬───┴───┬───┴───┼───────┼───┴───┤
  40│finepiM│shapeM │ dutyM │ panM  │filterM│volswpM│frqswpM│cutswpM│
    ├───────┼───────┴───────┴───────┴───────┴───────┴───────┴───────┤
  50│pcmposM│                     r e s e r v e d                   │
    └───────┴───────────────────────────────────────────────────────┘
    
NOTE: Instrument files start with a header which is 8 bytes long and reads "TRACKINS".
      Also, instrument files don't require the envelope IDs as they're saved along with the file.
NOTE: If (DFM&8) then it's a PCM instrument.
      If (DFM&16) then enable ring modulation and LFO sets the ring modulation frequency.
      RMF sets the ring modulator flags:
      (RMF&7): waveform
      the rest of bits: duty
      Also, (DFM&128) will be used for the MSB of PCMSptr and (DFM&64) the MSB of PCMloop.
      Also, (DFM&32) enables looping.
      PCm: PCM multiplier. Max value is 127.
      PCm&128: MSB of length
      flg: other flags:
        bit 0: reset oscillator on new note
        bit 1: MSB of duty
        bit 2: reset filter on new note
        bit 3: reset RM oscillator on new note
        bit 4: sync modulation
        bit 5: PCM loop enabled
        bit 6-7: auto-cut

## Instruments, legacy (pre-r152) format, 64 bytes each
---- -0- -1- -2- -3- -4- -5- -6- -7- -8- -9- -A- -B- -C- -D- -E- -F-
    ┌───────────────────────────────────────────────────────────────┐
  00│ instrument name, 32 bytes, padded with 0x00s                  │
    ├───┬───┬───┬───────────────────────────────┬───┬───┬───┬───┬───┤
  20│Ins│PCm│Env│vol cut res dty shp pit hpi pan│Off│FPt│FPR│DFM│LFO│
    ├───┼───┼───┴───┬───────┬───┬───────┬───────┼───┼───┴───┼───┼───┤
  30│Vol│Pit│PCMSlen│FilterH│Res│PCMSptr│PCMloop│FTm│Version│flg│RMF│
    └───┴───┴───────┴───────┴───┴───────┴───────┴───┴───────┴───┴───┘
    
NOTE: Instrument files start with a header which is 8 bytes long and reads "TRACKINS".
      Also, instrument files don't require the envelope IDs as they're saved along with the file.
NOTE: If (DFM&8) then it's a PCM instrument.
      If (DFM&16) then enable ring modulation and LFO sets the ring modulation frequency.
      RMF sets the ring modulator flags:
      (RMF&7): waveform
      the rest of bits: duty
      Also, (DFM&128) will be used for the MSB of PCMSptr and (DFM&64) the MSB of PCMloop.
      Also, (DFM&32) enables looping.
      PCm: PCM multiplier. Max value is 127.
      PCm&128: MSB of length
      flg: other flags:
        bit 0: reset oscillator on new note
        bit 1: MSB of duty
        bit 2: reset filter on new note
        bit 3: reset RM oscillator on new note
        bit 4: sync modulation
        bit 5: PCM loop enabled
        bit 6-7: auto-cut
      
## Patterns, 16 byte header
---- -0- -1- -2- -3- -4- -5- -6- -7- -8- -9- -A- -B- -C- -D- -E- -F-
    ┌───────────────────────────────────────────────────────────────┐
  00│PID│     Length    |Siz| Reserved, 11 bytes                    |
    ├───────────────────────────────────────────────────────────────┤
  10│ Pattern data                                                  │
    └───────────────────────────────────────────────────────────────┘

Pattern data is organized like this:
NOTE INST VOLU FXID FXVL
NOTE: channel's note, high nibble is octave, low nibble:
-0: no note
-1-12: notes from C to B
-13: note off
-14: note fade
-15: note cut
INST: channel's instrument
VOLU: channel's volume value:
-0: nothing
-1-63: panning (1-63)
-64-127: volume, overrides instrument's default volume
-128-137: fine volume slide up
-138-147: fine volume slide down
-160-175: volume slide up
-176-183: volume slide down
-184-193: pitch slide down
-194-203: pitch slide up
-204-213: portamento
-214-223: vibrato depth
-224-239: high offset
-240: panning (0)
-241: panning (64)
-242-248: panning slide left (0-7)\*2
-249-255: panning slide right (0-7)\*2
FXID: channel's effect number
FXVL: channel's effect value

please note that the patterns are also compressed,
in a way pretty similar to how ST3 does.

in case you don't know how ST3 "packs" patterns, here it is:
1. read maskbyte in seek position
2. if (maskbyte==0) then row is done
3. channel=maskbyte%32
4. if (maskbyte&32) then read NOTE and INSTRUMENT
5. if (maskbyte&64) then read VOLUME
6. if (maskbyte&128) then read EFFECT and EFFECTVALUE
7. repeat until pattern is done or `seek>size`

## Sound effect structure (perhaps legacy, I don't remember)
---- -0- -1- -2- -3- -4- -5- -6- -7- -8- -9- -A- -B- -C- -D- -E- -F-
    ┌───────────────────────────────────────────────────────────────┐
  00│ Packed effect data                                            |
    └───────────────────────────────────────────────────────────────┘

and this is how soundtracker packs sound effects:
1. read maskbyte in seek position
2. if (maskbyte==0) then effect is done
3. if (maskbyte&1) then PERIOD (hl) and VOLUME follows
4. if (maskbyte&2) then PAN follows
3. if (maskbyte&4) then DUTY and SHAPE follows
4. if (maskbyte&8) then RM, RMPERIOD (hl), RMSHAPE and RMDUTY follows
5. if (maskbyte&16) then PCMpos, PCMbound and PCMloop follow
6. if (maskbyte&32) then FILTERMODE, CUTOFF and RESONANCE follow
