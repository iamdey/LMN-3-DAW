# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to
[Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Changed

- Step Sequencer: Edit current clip instead of creating a new one.
- Step Sequencer: Changing number of note per measure does not loop accross options anymore.

### Fixed

- Sequencer had a length of 0 on loading. It's now filled by default: 4 measures of 4 notes.
- When changing number of notes per measures, the midi clip stays the same instead of time stretched.

## [0.6.0] - 2025-11-20

### Added
- Step Sequencer: support 4 levels of velocity, 30%, 60%, 100% and 0% when pressing `+` and a note.
  **BREAKING**: The sequence index is not incremented anymore after a being added.

## [0.5.0] - 2025-11-14

### Added

- Step Sequencer: Copy and paste. Press encoder 3 to enter range select mode.
  Turn the encoder to select the range you want to copy. Press `Cut` to copy the notes
  within the selected range. Press encoder 3 to disable range select mode. Rotate
  encoder 3 to where you want to paste the previously copied notes, press `Paste`.


### Changed

- Step Sequencer: The cursor now wraps around when reaching the limits.
- Drum Sampler: Show the list of drumkits sorted alphabetically.
- Sampler: Show the list of samplers sorted alphabetically.


## [0.4.0] - 2025-05-22

### Fixed

- Update internals (tracktion_engine 3.0, c++@20).

## [0.3.0] - 2025-01-30

### Changed

- Step Sequencer: edit the sequence while playing the loop. Press `+` to toggle
  a note or press `-` to remove all the notes, same way as classic editing.
- Step Sequencer: Keep the same number of measures when changing the number of
  notes per measure. The grid can hold 64 notes.

## [0.2.0] - 2025-01-28

### Added

- Load/Save track, from settings it allows to create or select a track.

### Fixed

- Update internals (tracktion_engine 2.1).
- Change the interval of the rate selector in modifiers for .5 instead of .1.

## [0.1.1] - 2022-06-15

### Added

- Samples can be loaded from subfolders.

### Fixed

- Internal updates (tracktion_engine).

## [0.1.0] - 2022-06-04

### Added

- Initial release

[unreleased]:
  https://github.com/FundamentalFrequency/LMN-3-DAW/compare/v0.4.0...HEAD
[0.4.0]:
  https://github.com/FundamentalFrequency/LMN-3-DAW/compare/v0.3.0...v0.4.0
[0.3.0]:
  https://github.com/FundamentalFrequency/LMN-3-DAW/compare/v0.2.0...v0.3.0
[0.2.0]:
  https://github.com/FundamentalFrequency/LMN-3-DAW/compare/v0.1.1...v0.2.0
[0.1.1]:
  https://github.com/FundamentalFrequency/LMN-3-DAW/compare/v0.1.0...v0.1.1
[0.1.0]: https://github.com/FundamentalFrequency/LMN-3-DAW/releases/tag/v0.1.0
