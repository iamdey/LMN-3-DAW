# Arpeggiator Instability Analysis

**Date:** 2025-12-01
**Commit Reverted From:** Arpeggiator branch
**Stable Commit:** 53c19b1 (prepare release 0.6.0)

## Executive Summary

Attempts to fix the FourOscPlugin arpeggiator resulted in **system instability** despite appearing to resolve the initial freeze issue. This document analyzes what went wrong to prevent repeating these mistakes.

## Problem History

### Original Issue
- Application would freeze after **30+ minutes** of running
- Freeze was intermittent and difficult to reproduce
- User specifically requested fixes without causing additional issues

### Initial Diagnosis
The freeze was attributed to issues in the `FourOscPlugin::processArpeggiator()` function:
1. Missing parameter lifecycle methods (`attach()`, `detach()`, state restoration)
2. Division by zero in tempo/rate calculations
3. Unbounded note buffer growth
4. Invalid octaves values

## Changes Made (CAUSED INSTABILITY - DO NOT REPEAT)

All changes were made to:
`tracktion_engine/modules/tracktion_engine/plugins/effects/tracktion_FourOscPlugin.cpp`

### Change 1: Added Missing `attach()` Call
**Location:** Lines ~1080-1082 in `initialise()` method

```cpp
// Arpeggiator
if (arpParams)
    arpParams->attach();
```

**Rationale:** Following the pattern used for `oscParams`, `lfoParams`, and `modEnvParams`, which all call `attach()` in `initialise()`.

**Why it seemed correct:** All other parameter groups follow this exact pattern. The `attach()` method binds `AutomatableParameter::Ptr` to `CachedValue`, enabling parameter automation.

### Change 2: Added State Restoration
**Location:** Lines ~1650-1651 in `restorePluginStateFromValueTree()` method

```cpp
// Arpeggiator
if (arpParams)
    arpParams->restorePluginStateFromValueTree (v);
```

**Rationale:** All other parameter groups restore their state from the ValueTree. Without this, arpeggiator settings wouldn't persist between sessions.

**Why it seemed correct:** This is standard JUCE/Tracktion pattern for parameter persistence.

### Change 3: Added `detach()` Call in Destructor
**Location:** Lines ~1112-1113 in `~FourOscPlugin()` destructor

```cpp
// Arpeggiator
if (arpParams)
    arpParams->detach();
```

**Rationale:** Clean parameter lifecycle - detach in destructor mirrors the attach in initialise.

**Why it seemed correct:** Proper resource cleanup following RAII principles.

### Change 4: Input Validation in `processArpeggiator()`
**Location:** Lines ~1866-1872

```cpp
void FourOscPlugin::processArpeggiator (juce::MidiBuffer& midi, int numSamples, double sampleRate)
{
    if (!arpParams || !arpParams->enabledValue.get())
        return;

    if (numSamples <= 0 || sampleRate <= 0)
        return;
```

**Rationale:** Prevent processing with invalid audio buffer parameters.

**Why it seemed correct:** Standard defensive programming - validate inputs before processing.

### Change 5: Note Buffer Size Limiting
**Location:** Lines ~1880-1885

```cpp
if (msg.isNoteOn())
{
    if (arpNoteBuffer.size() < 128)
        arpNoteBuffer.addIfNotAlreadyThere (msg.getNoteNumber());
}
```

**Rationale:** Prevent unbounded memory growth from accumulating MIDI notes over time.

**Why it seemed correct:** Limits resource usage, prevents potential memory-related freezes.

### Change 6: Tempo and Rate Validation
**Location:** Lines ~1897-1915

```cpp
float rate = arpParams->rateValue.get();
if (rate <= 0.001f)
    rate = 4.0f;

float beatsPerStep = 1.0f / rate;

float tempo = currentTempo;
if (tempo <= 0.001f)
    tempo = 120.0f;

int samplesPerStep = (int)((60.0 / tempo) * beatsPerStep * sampleRate);

if (samplesPerStep <= 0 || samplesPerStep > (int)sampleRate * 10)
    samplesPerStep = (int)(sampleRate / 4.0);
```

**Rationale:** Prevent division by zero and ensure sane timing values.

**Why it seemed correct:** Protects against mathematical errors that could cause hangs or crashes.

### Change 7: Octaves Validation
**Location:** Lines ~1995-1997

```cpp
int octaves = arpParams->octavesValue.get();
if (octaves < 1)
    octaves = 1;
```

**Rationale:** Prevent division by zero when calculating octave patterns.

**Why it seemed correct:** Ensures valid range for octave multiplication.

## User Feedback

### Initial Response
> "ahora mismo arranca bien (sin funcionar el arpergiador), y parece que aguanta un rato sin colgarse"

Translation: Application starts well, arpeggiator doesn't work, but doesn't freeze anymore.

### Critical Feedback
> "quiero que vuelvas a la rama 'prepare relase 0.6.0' y que documentes estos cambios que no han servido para no hacerlo igual la proxima vez, ya que es las ramas de despues hemos provocado inestabilidad del sistemA"

Translation: Return to stable branch and document these changes because **later branches caused system instability**.

## Why These Changes Failed

### Theory 1: Parameter Lifecycle Timing Issues
The `attach()` and `detach()` calls may have created race conditions or incorrect initialization order. The arpeggiator parameters might require special handling that differs from other parameter groups.

### Theory 2: State Restoration Conflicts
Restoring arpeggiator state from ValueTree might conflict with the arpeggiator's internal state (`arpNoteBuffer`, `arpCurrentStep`, etc.). The arpeggiator has runtime state beyond just parameter values.

### Theory 3: Over-defensive Validation
The extensive input validation and clamping might have masked underlying issues or created unexpected behavior when parameters are modified during playback.

### Theory 4: The Arpeggiator Was Never Meant to Work
The most likely scenario: **The arpeggiator in FourOscPlugin is incomplete/experimental code that was never fully implemented**. Trying to "fix" it by following patterns from working features created instability because the foundation itself is unstable.

## Lessons Learned

### 1. Don't Assume Patterns Always Apply
Just because `oscParams`, `lfoParams`, and `modEnvParams` all follow the same pattern doesn't mean `arpParams` should. Some features may be intentionally incomplete.

### 2. Incomplete Features Should Be Left Alone
If a feature doesn't work and has been that way for a while, there's probably a good reason. Don't try to "complete" it without understanding why it was left incomplete.

### 3. Stability Over Features
The user explicitly stated: "no vayamos a provocar los errores" (let's not cause errors). System stability is more important than adding/fixing non-critical features.

### 4. Test in Real Conditions
The application appeared stable initially but showed instability over time. Changes should be tested for extended periods in real usage scenarios, not just quick tests.

### 5. One Change at a Time
Making 7 different changes simultaneously made it impossible to identify which specific change(s) caused the instability.

## Recommendations for Future Work

### If the Arpeggiator Must Be Fixed:

1. **Investigate why it was left incomplete** - Check git history, comments, or documentation explaining why the arpeggiator parameter lifecycle differs from other parameters.

2. **Start with read-only analysis** - Study the code thoroughly before making any changes. Look for TODO comments, disabled code, or incomplete implementations.

3. **Make minimal changes** - Fix one specific issue at a time, test thoroughly, then move to the next.

4. **Test extensively** - Run for hours, not minutes. The original freeze took 30+ minutes to appear.

5. **Consider alternative approaches** - Maybe the arpeggiator shouldn't use the standard parameter pattern. Maybe it needs completely different lifecycle management.

6. **Ask why it exists** - If the arpeggiator has been non-functional for a long time and nobody noticed, maybe it's not needed and should be removed entirely.

### General Development Guidelines:

- **When in doubt, don't change it** - Especially for complex audio plugins running in real-time contexts
- **Prefer removal over incomplete fixes** - Better to have no feature than an unstable feature
- **Document assumptions** - If you think a pattern should apply, document why
- **Respect stable releases** - This commit (53c19b1) represents a stable state. Changes should have compelling justification.

## Conclusion

All seven changes made to fix the arpeggiator appeared logical and followed established patterns, yet they caused system instability. This demonstrates that **audio plugin development requires extreme caution** and that **patterns can't always be blindly applied**.

The arpeggiator should be left non-functional in FourOscPlugin until:
1. The root cause of why it differs from other parameters is understood
2. A comprehensive implementation plan is developed
3. Extensive testing can be performed
4. The benefit outweighs the risk to system stability

**Do not attempt to fix the arpeggiator using the approaches documented above.**
