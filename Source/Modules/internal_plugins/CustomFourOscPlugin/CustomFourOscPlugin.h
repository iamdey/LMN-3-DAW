#pragma once
#include <tracktion_engine/tracktion_engine.h>

namespace internal_plugins {

namespace CustomFourOscIDs {
static const juce::Identifier arpEnabled{"customArpEnabled"};
static const juce::Identifier arpMode{"customArpMode"};
static const juce::Identifier arpSync{"customArpSync"};
static const juce::Identifier arpRate{"customArpRate"};
static const juce::Identifier arpGate{"customArpGate"};
static const juce::Identifier arpOctaves{"customArpOctaves"};
} // namespace CustomFourOscIDs

class CustomFourOscPlugin : public tracktion::FourOscPlugin {
  public:
    explicit CustomFourOscPlugin(tracktion::PluginCreationInfo info);
    ~CustomFourOscPlugin() override;

    static const char *getPluginName() { return NEEDS_TRANS("4OSC"); }
    static const char *xmlTypeName;

    juce::String getName() const override { return TRANS("4OSC"); }
    juce::String getPluginType() override { return xmlTypeName; }
    juce::String getShortName(int) override { return "C4OSC"; }
    juce::String getSelectableDescription() override {
        return TRANS("4OSC with Custom Arpeggiator");
    }

    // Override to intercept MIDI processing for arpeggiator
    void applyToBuffer(const tracktion::PluginRenderContext &fc) override;
    void restorePluginStateFromValueTree(const juce::ValueTree &v) override;

    // Arpeggiator public interface
    void addNoteToArpeggiator(int noteNumber);
    void removeNoteFromArpeggiator(int noteNumber);
    void clearArpeggiatorNotes();
    juce::Array<int> getArpeggiatorNotes() const { return arpNoteBuffer; }
    bool isArpeggiatorEnabled() const;
    void setArpeggiatorEnabled(bool shouldEnable);
    bool isArpeggiatorTempoSyncEnabled() const;
    void setArpeggiatorTempoSyncEnabled(bool shouldSync);
    int getArpeggiatorMode() const;
    void setArpeggiatorMode(int modeIndex);
    float getArpeggiatorRate() const;
    void setArpeggiatorRate(float newRate);
    int getArpeggiatorOctaves() const;
    void setArpeggiatorOctaves(int newOctaves);
    float getArpeggiatorGate() const;
    void setArpeggiatorGate(float newGate);

    // Arpeggiator parameters structure
    struct ArpParams {
        ArpParams(CustomFourOscPlugin &plugin);
        void attach();
        void detach();

        juce::CachedValue<bool> enabledValue;
        juce::CachedValue<int> modeValue;
        juce::CachedValue<bool> syncValue;
        juce::CachedValue<float> rateValue, gateValue;
        juce::CachedValue<int> octavesValue;

        tracktion::AutomatableParameter::Ptr rate, gate;

        void restorePluginStateFromValueTree(const juce::ValueTree &v) {
            tracktion::copyPropertiesToCachedValues(v, enabledValue, modeValue,
                                                    syncValue, rateValue,
                                                    gateValue, octavesValue);
        }
    };

    std::unique_ptr<ArpParams> arpParams;

  private:
    friend struct ArpParams;

    tracktion::AutomatableParameter::Ptr
    addArpParam(const juce::String &paramID, const juce::String &name,
                juce::NormalisableRange<float> valueRange);

    void processArpeggiator(tracktion::MidiMessageArray &midi, int numSamples,
                            int bufferStartSample);
    void stopArpeggiator(tracktion::MidiMessageArray *midi,
                         int bufferStartSample);

    // Arpeggiator state
    juce::Array<int> arpNoteBuffer;
    int arpCurrentStep = 0;
    int arpSampleCounter = 0;
    int arpCurrentNoteNumber = -1;
    bool arpNoteIsOn = false;
    bool wasTransportPlaying = false;
    tracktion::MPESourceID arpSourceID{tracktion::createUniqueMPESourceID()};
    tracktion::tempo::Sequence::Position arpTempoPosition{
        tracktion::createPosition(edit.tempoSequence)};

    float arpCurrentTempo = 120.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomFourOscPlugin)
};

} // namespace internal_plugins
