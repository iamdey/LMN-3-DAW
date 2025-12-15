#include "CustomFourOscPlugin.h"

namespace internal_plugins {

const char *CustomFourOscPlugin::xmlTypeName = "custom4osc";

CustomFourOscPlugin::ArpParams::ArpParams(CustomFourOscPlugin &plugin) {
    auto um = plugin.getUndoManager();

    using namespace CustomFourOscIDs;

    enabledValue.referTo(plugin.state, arpEnabled, um, false);
    modeValue.referTo(plugin.state, arpMode, um, 0);
    syncValue.referTo(plugin.state, arpSync, um, true);
    rateValue.referTo(plugin.state, arpRate, um, 4.0f);
    gateValue.referTo(plugin.state, arpGate, um, 0.5f);
    octavesValue.referTo(plugin.state, arpOctaves, um, 1);

    rate = plugin.addArpParam("arpRate", TRANS("Arp Rate"), {1.0f, 16.0f});
    gate = plugin.addArpParam("arpGate", TRANS("Arp Gate"), {0.05f, 1.0f});
}

void CustomFourOscPlugin::ArpParams::attach() {
    rate->attachToCurrentValue(rateValue);
    gate->attachToCurrentValue(gateValue);
}

void CustomFourOscPlugin::ArpParams::detach() {
    rate->detachFromCurrentValue();
    gate->detachFromCurrentValue();
}

CustomFourOscPlugin::CustomFourOscPlugin(tracktion::PluginCreationInfo info)
    : tracktion::FourOscPlugin(info) {
    // Initialize arpeggiator
    arpParams = std::make_unique<ArpParams>(*this);

    if (arpParams)
        arpParams->attach();
}

CustomFourOscPlugin::~CustomFourOscPlugin() {
    notifyListenersOfDeletion();

    if (arpParams)
        arpParams->detach();
}

void CustomFourOscPlugin::applyToBuffer(
    const tracktion::PluginRenderContext &fc) {
    if (arpParams != nullptr && fc.bufferForMidiMessages != nullptr) {
        arpTempoPosition.set(fc.editTime.getStart());
        arpCurrentTempo = float(arpTempoPosition.getTempo());
        processArpeggiator(*fc.bufferForMidiMessages, fc.bufferNumSamples,
                           fc.bufferStartSample);
    }

    tracktion::FourOscPlugin::applyToBuffer(fc);
}

void CustomFourOscPlugin::restorePluginStateFromValueTree(
    const juce::ValueTree &v) {
    // Call base class first
    tracktion::FourOscPlugin::restorePluginStateFromValueTree(v);

    // Restore arpeggiator state
    if (arpParams)
        arpParams->restorePluginStateFromValueTree(v);
}

void CustomFourOscPlugin::addNoteToArpeggiator(int noteNumber) {
    arpNoteBuffer.addIfNotAlreadyThere(noteNumber);
}

void CustomFourOscPlugin::removeNoteFromArpeggiator(int noteNumber) {
    arpNoteBuffer.removeAllInstancesOf(noteNumber);
}

void CustomFourOscPlugin::clearArpeggiatorNotes() { arpNoteBuffer.clear(); }

bool CustomFourOscPlugin::isArpeggiatorEnabled() const {
    return arpParams != nullptr && arpParams->enabledValue.get();
}

void CustomFourOscPlugin::setArpeggiatorEnabled(bool shouldEnable) {
    if (arpParams != nullptr)
        arpParams->enabledValue = shouldEnable;
}

bool CustomFourOscPlugin::isArpeggiatorTempoSyncEnabled() const {
    return arpParams != nullptr && arpParams->syncValue.get();
}

void CustomFourOscPlugin::setArpeggiatorTempoSyncEnabled(bool shouldSync) {
    if (arpParams != nullptr)
        arpParams->syncValue = shouldSync;
}

int CustomFourOscPlugin::getArpeggiatorMode() const {
    if (arpParams == nullptr)
        return 0;

    return juce::jlimit(0, 4, arpParams->modeValue.get());
}

void CustomFourOscPlugin::setArpeggiatorMode(int modeIndex) {
    if (arpParams == nullptr)
        return;

    arpParams->modeValue = juce::jlimit(0, 4, modeIndex);
}

float CustomFourOscPlugin::getArpeggiatorRate() const {
    if (arpParams == nullptr)
        return 4.0f;

    return arpParams->rateValue.get();
}

void CustomFourOscPlugin::setArpeggiatorRate(float newRate) {
    if (arpParams == nullptr)
        return;

    arpParams->rateValue = juce::jlimit(1.0f, 16.0f, newRate);
}

int CustomFourOscPlugin::getArpeggiatorOctaves() const {
    if (arpParams == nullptr)
        return 1;

    return juce::jlimit(1, 4, arpParams->octavesValue.get());
}

void CustomFourOscPlugin::setArpeggiatorOctaves(int newOctaves) {
    if (arpParams == nullptr)
        return;

    arpParams->octavesValue = juce::jlimit(1, 4, newOctaves);
}

float CustomFourOscPlugin::getArpeggiatorGate() const {
    if (arpParams == nullptr)
        return 0.5f;

    return arpParams->gateValue.get();
}

void CustomFourOscPlugin::setArpeggiatorGate(float newGate) {
    if (arpParams == nullptr)
        return;

    arpParams->gateValue = juce::jlimit(0.05f, 1.0f, newGate);
}

tracktion::AutomatableParameter::Ptr
CustomFourOscPlugin::addArpParam(const juce::String &paramID,
                                 const juce::String &name,
                                 juce::NormalisableRange<float> valueRange) {
    auto *newParam =
        new tracktion::AutomatableParameter(paramID, name, *this, valueRange);
    addAutomatableParameter(*newParam);
    return newParam;
}

void CustomFourOscPlugin::processArpeggiator(tracktion::MidiMessageArray &midi,
                                             int numSamples,
                                             int bufferStartSample) {
    if (!arpParams || !arpParams->enabledValue.get())
        return;

    const double sr = this->tracktion::Plugin::sampleRate;

    if (numSamples <= 0 || sr <= 0.0)
        return;

    tracktion::MidiMessageArray processedMidi;
    processedMidi.isAllNotesOff = midi.isAllNotesOff;

    const int bufferEndSample = bufferStartSample + numSamples;

    for (const auto &metadata : midi) {
        const double eventTime = metadata.getTimeStamp();

        if (eventTime < bufferStartSample || eventTime >= bufferEndSample) {
            processedMidi.addMidiMessage(metadata, eventTime,
                                         metadata.mpeSourceID);
            continue;
        }

        const juce::MidiMessage &msg = metadata;

        if (msg.isNoteOn()) {
            if (arpNoteBuffer.size() < 128)
                arpNoteBuffer.addIfNotAlreadyThere(msg.getNoteNumber());
        } else if (msg.isNoteOff()) {
            arpNoteBuffer.removeAllInstancesOf(msg.getNoteNumber());
        } else {
            processedMidi.addMidiMessage(msg, eventTime, metadata.mpeSourceID);
        }
    }

    if (arpNoteBuffer.isEmpty()) {
        if (arpNoteIsOn && arpCurrentNoteNumber >= 0) {
            processedMidi.addMidiMessage(
                juce::MidiMessage::noteOff(1, arpCurrentNoteNumber),
                bufferStartSample, arpSourceID);
            arpNoteIsOn = false;
        }

        midi.swapWith(processedMidi);
        return;
    }

    const bool syncToTempo = arpParams->syncValue.get();
    float rate = arpParams->rateValue.get();

    if (rate <= 0.001f)
        rate = 4.0f;

    int samplesPerStep = 0;

    if (syncToTempo) {
        const float beatsPerStep = 1.0f / rate;
        float tempo = arpCurrentTempo;
        if (tempo <= 0.001f)
            tempo = 120.0f;

        samplesPerStep = int((60.0f / tempo) * beatsPerStep * sr);
    } else {
        samplesPerStep = int((1.0f / rate) * sr);
    }

    if (samplesPerStep <= 0 || samplesPerStep > int(sr) * 10)
        samplesPerStep = int(sr / 4.0);

    samplesPerStep = std::max(1, samplesPerStep);

    float gate = juce::jlimit(0.05f, 1.0f, arpParams->gateValue.get());
    int noteOnDuration =
        juce::jlimit(1, samplesPerStep, int(std::floor(samplesPerStep * gate)));

    int mode = arpParams->modeValue.get();
    juce::Array<int> sortedNotes = arpNoteBuffer;

    if (mode == 0) {
        sortedNotes.sort();
    } else if (mode == 1) {
        sortedNotes.sort();
        for (int i = 0; i < sortedNotes.size() / 2; ++i)
            std::swap(sortedNotes.getReference(i),
                      sortedNotes.getReference(sortedNotes.size() - 1 - i));
    } else if (mode == 2) {
        sortedNotes.sort();
        juce::Array<int> upDown;
        for (auto note : sortedNotes)
            upDown.add(note);
        for (int i = sortedNotes.size() - 2; i > 0; --i)
            upDown.add(sortedNotes[i]);
        sortedNotes = upDown;
    } else if (mode == 3) {
        sortedNotes.sort();
        juce::Random random;
        for (int i = 0; i < sortedNotes.size(); ++i) {
            const int j = random.nextInt(sortedNotes.size());
            std::swap(sortedNotes.getReference(i), sortedNotes.getReference(j));
        }
    }

    for (int sample = 0; sample < numSamples; ++sample) {
        const int absoluteSample = bufferStartSample + sample;

        if (arpNoteIsOn && arpSampleCounter >= noteOnDuration) {
            processedMidi.addMidiMessage(
                juce::MidiMessage::noteOff(1, arpCurrentNoteNumber),
                absoluteSample, arpSourceID);
            arpNoteIsOn = false;
        }

        if (arpSampleCounter >= samplesPerStep) {
            arpSampleCounter = 0;

            if (arpNoteIsOn && arpCurrentNoteNumber >= 0) {
                processedMidi.addMidiMessage(
                    juce::MidiMessage::noteOff(1, arpCurrentNoteNumber),
                    absoluteSample, arpSourceID);
                arpNoteIsOn = false;
            }

            if (!sortedNotes.isEmpty()) {
                arpCurrentStep = (arpCurrentStep + 1) % sortedNotes.size();
                int baseNote = sortedNotes[arpCurrentStep];

                int octaves = juce::jlimit(1, 4, arpParams->octavesValue.get());
                int octaveOffset =
                    (arpCurrentStep / sortedNotes.size()) % octaves;

                arpCurrentNoteNumber =
                    juce::jlimit(0, 127, baseNote + octaveOffset * 12);

                processedMidi.addMidiMessage(
                    juce::MidiMessage::noteOn(1, arpCurrentNoteNumber, 1.0f),
                    absoluteSample, arpSourceID);
                arpNoteIsOn = true;
            }
        }

        arpSampleCounter++;
    }

    midi.swapWith(processedMidi);
}

} // namespace internal_plugins
