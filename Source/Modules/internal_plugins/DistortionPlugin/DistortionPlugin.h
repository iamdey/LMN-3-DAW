#pragma once
namespace internal_plugins {

namespace IDs {
const juce::Identifier gain("gain");
} // namespace IDs

class DistortionPlugin : public tracktion::Plugin {
  public:
    DistortionPlugin(tracktion::PluginCreationInfo);
    ~DistortionPlugin() override;

    //==============================================================================
    static const char *getPluginName() { return "Distortion"; }
    static const char *xmlTypeName;

    juce::String getName() const override { return "Distortion"; }
    juce::String getPluginType() override { return xmlTypeName; }
    juce::String getShortName(int) override { return "Disto"; }

    void initialise(const tracktion::PluginInitialisationInfo &) override;
    void deinitialise() override;
    void applyToBuffer(const tracktion::PluginRenderContext &) override;
    juce::String getSelectableDescription() override {
        return "Distortion Plugin";
    }

    juce::CachedValue<float> gain;
    tracktion::engine::AutomatableParameter *gainParam;

  private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DistortionPlugin)
};

} // namespace internal_plugins
