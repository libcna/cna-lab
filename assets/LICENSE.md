# Asset licenses

## Sounds

The seven WAV files in this directory come from the **[Essentials Series / NOX SOUND](https://nox-sound-design.itch.io/essentials-series-sfx-nox-sound)** collection. The author's product page and the collection's `Essentials_Series_README.pdf` both state that all sounds are released under CC0. CC0 permits redistribution and use in this game.

| Local file | Original file in the collection |
| --- | --- |
| `hum.wav` | `Electromagnetic_NOX_SOUND/Electromagnetic_Neon_Light_Loop_Mono_Elektrousi_01.wav` |
| `step.wav` | `Footsteps_Essentials_NOX_SOUND/Footsteps_Tile/Footsteps_Tile_Walk/Footsteps_Tile_Walk_01.wav` |
| `step-carpet.wav` | `Footsteps_Essentials_NOX_SOUND/Footsteps_Wood/Footsteps_Wood_Walk/Footsteps_Wood_Walk_01.wav` |
| `transition.wav` | `Sample_A_Sound_Effect/Household_Door_Wood_Open_Stereo.wav` |
| `entity-wanderer.wav` | `Voices_Essentials_NOX_SOUND/Voice_Essential_Male/Voice_Male_Breath/Voice_Male_V1_Breath_Nose_Single_Mono_01.wav` |
| `entity-watcher.wav` | `Voices_Essentials_NOX_SOUND/Voice_Essential_Male/Voice_Male_Breath/Voice_Male_V2_Breath_Frozen_Mono_03.wav` |
| `entity-crawler.wav` | `Sample_A_Sound_Effect/Cloth_Coat_PickUp_Stereo.wav` |

Source collection supplied locally at `/rv/tmp/Essentials_Series_NOX_SOUND`. The source README is retained in the original collection; the game ships only these selected sounds.

The hum has been gain-adjusted by +8 dB. The hard-floor step has +13 dB gain. The carpet step uses a 1.4 kHz low-pass filter and +10 dB gain to soften the source impact. These steps are mono 48 kHz PCM; their measured peaks remain below -7 dBFS before game volume is applied. The CC0 license covers these edits.

Creature sounds are mono 48 kHz PCM excerpts with short fades: the wanderer
uses the first 3.25 seconds of nasal breathing (+14 dB, 3.2 kHz low-pass), the
watcher uses the first four seconds of the frozen breath (+9 dB, 2.8 kHz
low-pass), and the crawler uses 2.2 seconds of coat movement starting at 1.15
seconds (+1 dB, 3.5 kHz low-pass). Playback adds a slight downward pitch and
distance attenuation. The license covers these edits and creature uses.

## Original wallpaper

`wallpaper-v1.png` is an original AI-generated visual asset created specifically for this project, with no input reference image. It is an original project asset, alongside the materials and geometry generated in code. Its prompt and processing are recorded in [wallpaper-generation.md](wallpaper-generation.md). The image is not copied from a Backrooms reference photograph.

## Original carpet

`carpet-v1.png` is an original AI-generated visual asset created specifically for this project with no input reference image. Its prompt, source fingerprint and runtime processing are recorded in [carpet-generation.md](carpet-generation.md). It is not a downloaded photograph or third-party material.

## Original concrete

`concrete-v1.png` is an original AI-generated visual asset created for this project with no input reference image. Its prompt, source fingerprint and runtime processing are recorded in [concrete-generation.md](concrete-generation.md). It is not a downloaded photograph or third-party material.
