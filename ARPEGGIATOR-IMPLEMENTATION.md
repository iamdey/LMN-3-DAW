# Arpeggiator Implementation for FourOsc

This document describes the implementation of a real-time MIDI arpeggiator for the FourOsc synthesizer plugin in LMN-3-DAW.

## Overview

The arpeggiator is implemented as a **7th tab** in the FourOsc plugin interface, providing real-time arpeggiation of incoming MIDI notes with adjustable parameters.

## Architecture

### Component Structure

```
FourOscView (TabbedComponent)
  ├── OSC1 Tab (OscillatorView)
  ├── OSC2 Tab (OscillatorView)
  ├── OSC3 Tab (OscillatorView)
  ├── OSC4 Tab (OscillatorView)
  ├── ADSR Tab (ADSRView)
  ├── FILTER Tab (FilterView)
  └── ARP Tab (ArpeggiatorView) ← NEW!
```

### MVVM Pattern

Following the project's architecture guidelines:

- **Model**: `ArpeggiatorViewModel` - Manages state and parameters
- **View**: `ArpeggiatorView` - UI rendering and MIDI encoder handling
- **Services**: Uses `MidiCommandManager` for hardware input

## Features

### Parameters

| Parameter | Control | Range | Default | Description |
|-----------|---------|-------|---------|-------------|
| **Mode** | Encoder 1 | Off, Up, Down, Up-Down, Random | Up | Arpeggio pattern |
| **Rate** | Encoder 2 | 1.0 - 16.0 Hz | 8.0 Hz | Arpeggio speed |
| **Octaves** | Encoder 3 | 1 - 4 | 1 | Octave range |
| **Gate** | Encoder 4 | 0.05 - 1.0 | 0.8 | Note duration (80% default) |

### Control Mapping

- **Encoder 1**: Cycle through arpeggio modes
- **Encoder 2**: Adjust rate (speed)
- **Encoder 3**: Change octave range
- **Encoder 4**: Modify gate length
- **Control Button**: Toggle arpeggiator ON/OFF
- **Note Input**: Add/remove notes from arpeggio buffer (toggle)

### Arpeggio Modes

1. **Off**: Arpeggiator disabled
2. **Up**: Ascending pattern through notes and octaves
3. **Down**: Descending pattern through notes and octaves
4. **Up-Down**: Ascending then descending (ping-pong)
5. **Random**: Random note and octave selection

## MIDI Injection System

### Tracktion Engine Integration

The arpeggiator uses Tracktion Engine's **real-time MIDI injection** system:

```cpp
// Find the AudioTrack containing FourOsc
tracktion::AudioTrack *audioTrack = findTrackWithPlugin(plugin);

// Create unique MIDI source identifier
tracktion::MPESourceID midiSourceID = tracktion::createUniqueMPESourceID();

// Inject note-on
auto noteOnMsg = juce::MidiMessage::noteOn(1, noteNumber, velocity);
audioTrack->injectLiveMidiMessage(noteOnMsg, midiSourceID);

// Later: inject note-off
auto noteOffMsg = juce::MidiMessage::noteOff(1, noteNumber);
audioTrack->injectLiveMidiMessage(noteOffMsg, midiSourceID);
```

### Message Flow

```
ArpeggiatorView::playNextNote()
    ↓
AudioTrack::injectLiveMidiMessage()
    ↓
LiveMidiInjectingNode (in playback graph)
    ↓
PluginNode
    ↓
FourOscPlugin::applyToBuffer()
    ↓
Sound is generated!
```

### Key Classes Used

- **`AudioTrack`**: Provides `injectLiveMidiMessage()` API
- **`LiveMidiInjectingNode`**: Merges live MIDI into playback stream
- **`PluginNode`**: Routes MIDI to plugin processing
- **`MPESourceID`**: Unique identifier for MIDI source tracking
- **`MidiMessageWithSource`**: MIDI message wrapper with source ID

## Implementation Details

### File Structure

```
Source/
├── Modules/app_view_models/Edit/Plugins/FourOsc/
│   ├── ArpeggiatorViewModel.h          # State management
│   └── ArpeggiatorViewModel.cpp        # Parameter logic
├── Views/Edit/Plugins/FourOsc/
│   ├── ArpeggiatorView.h               # UI component
│   └── ArpeggiatorView.cpp             # Rendering & MIDI injection
└── Views/Edit/Plugins/FourOsc/
    ├── FourOscView.h                   # Updated: added ARP tab
    └── FourOscView.cpp                 # Updated: tab creation
```

### Timer-based Sequencing

The arpeggiator uses `juce::Timer` for precise timing:

```cpp
class ArpeggiatorView : private juce::Timer {
    void timerCallback() override {
        if (viewModel.isEnabled() && !viewModel.getCurrentNotes().isEmpty()) {
            playNextNote();  // Generates next note in sequence
        }
    }

    void startArpeggio() {
        int intervalMs = static_cast<int>(1000.0f / viewModel.getRate());
        startTimer(intervalMs);
    }
};
```

### Note Buffer Management

Notes are stored in the ViewModel:

```cpp
// Add note when pressed
void ArpeggiatorViewModel::addNoteToBuffer(int noteNumber) {
    if (!noteBuffer.contains(noteNumber)) {
        noteBuffer.add(noteNumber);
        noteBuffer.sort();  // Keep sorted for Up/Down modes
    }
}

// Remove when released (or toggle)
void ArpeggiatorViewModel::removeNoteFromBuffer(int noteNumber) {
    noteBuffer.removeAllInstancesOf(noteNumber);
}
```

### Thread Safety

MIDI injection is **thread-safe**:
- `AudioTrack::injectLiveMidiMessage()` is called from the message thread
- `LiveMidiInjectingNode` uses `juce::CriticalSection` internally
- Timer callbacks run on the message thread

## UI Design

### Visual Elements

- **Title**: "4OSC: ARPEGGIATOR"
- **4 Knobs**: Visual feedback for all parameters
- **Status Display**: Shows ON/OFF, current mode, and active note count

### Layout

```
┌─────────────────────────────────────────┐
│        4OSC: ARPEGGIATOR                │
│                                         │
│  [Mode]  [Rate]  [Octaves]  [Gate]    │
│   Up      8Hz       1        80%       │
│    ○      ○         ○         ○        │
│                                         │
│ Status: ON | Mode: UP | Notes: 3       │
└─────────────────────────────────────────┘
```

## Usage Example

### Basic Arpeggiation

1. Open FourOsc plugin
2. Navigate to **ARP** tab (press + button 6 times)
3. Press **Control Button** to enable arpeggiator
4. Press notes on MIDI controller
5. Notes will automatically arpeggiate!

### Advanced Patterns

**Fast Up Pattern:**
- Mode: Up
- Rate: 16 Hz
- Octaves: 2
- Gate: 0.5 (50% - staccato)

**Slow Random:**
- Mode: Random
- Rate: 4 Hz
- Octaves: 4
- Gate: 0.9 (90% - legato)

**Ping-Pong:**
- Mode: Up-Down
- Rate: 8 Hz
- Octaves: 3
- Gate: 0.8

## Technical Notes

### Rate Calculation

The timer interval is calculated as:
```cpp
intervalMs = 1000.0f / rate
```

Examples:
- 1 Hz = 1000ms interval (1 note per second)
- 8 Hz = 125ms interval (8 notes per second)
- 16 Hz = 62.5ms interval (16 notes per second)

### Gate Implementation

Gate controls note duration:
- Gate = 1.0 (100%): Note held until next note
- Gate = 0.5 (50%): Note-off sent halfway through interval
- Gate = 0.05 (5%): Very short staccato notes

**Current Implementation**: Basic on/off (gate timing not yet implemented in timer)

**Future Enhancement**:
```cpp
void timerCallback() override {
    playNextNote();

    // Schedule note-off based on gate
    int gateMs = static_cast<int>(intervalMs * viewModel.getGate());
    juce::Timer::callAfterDelay(gateMs, [this]() { stopCurrentNote(); });
}
```

### Velocity

Currently fixed at **100** (medium-hard).

**Future Enhancement**: Add velocity parameter or use input note velocity.

## Limitations & Future Work

### Current Limitations

1. **Gate timing**: Note-off sent only when next note plays (gate not precisely timed)
2. **Fixed velocity**: All notes play at velocity 100
3. **Single channel**: Only MIDI channel 1
4. **Sync**: No tempo sync to DAW (uses Hz instead of musical divisions)

### Planned Enhancements

1. **Tempo Sync**: Use beat divisions (1/4, 1/8, 1/16) instead of Hz
2. **Velocity Control**: Add velocity parameter
3. **Precise Gate**: Implement gate timing with delayed note-off
4. **Swing**: Add swing/groove parameter
5. **Pattern Presets**: Save/load arpeggio patterns
6. **MIDI Channel**: Make channel configurable
7. **Latch Mode**: Continue arpeggiation after note release

## Testing

### Manual Testing Checklist

- [ ] Enable arpeggiator with Control Button
- [ ] Add notes to buffer (toggle on/off)
- [ ] Verify all 5 modes work correctly
- [ ] Test rate adjustment (1-16 Hz)
- [ ] Test octave range (1-4 octaves)
- [ ] Test gate adjustment
- [ ] Verify MIDI notes reach FourOsc
- [ ] Check UI parameter display updates
- [ ] Test with multiple simultaneous notes
- [ ] Verify note-off messages sent correctly

### Debugging Tips

If notes don't play:
1. Check `audioTrack != nullptr` in logs
2. Verify plugin is on AudioTrack (not other track types)
3. Check MIDI routing in Tracktion Engine
4. Ensure FourOsc receives MIDI (`takesMidiInput()` returns true)
5. Monitor with MIDI monitor plugin

## References

### Tracktion Engine Documentation

- **LiveMidiInjectingNode**: `tracktion_engine/playback/graph/tracktion_LiveMidiInjectingNode.h`
- **AudioTrack MIDI API**: `tracktion_engine/model/tracks/tracktion_AudioTrack.h` (lines 108-109)
- **StepClip Example**: `tracktion_engine/model/clips/tracktion_StepClip.cpp` (lines 399-400)
- **Plugin Processing**: `tracktion_engine/plugins/tracktion_Plugin.h`

### Project Architecture

- **Contributing Guide**: `contributing.md` - MVVM pattern explanation
- **Main README**: `README.md` - Project overview
- **Build Docker**: `BUILD-DOCKER.md` - Compilation instructions

## Credits

Implementation follows LMN-3-DAW's MVVM architecture and Tracktion Engine's MIDI injection patterns.

---

**Version**: 1.0
**Date**: 2025-11-23
**Status**: ✅ Complete (MIDI injection fully implemented)
