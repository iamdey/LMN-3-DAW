#pragma once

namespace internal_plugins
{

    class DrumSamplerPlugin  : public tracktion_engine::SamplerPlugin
    {
    public:

        DrumSamplerPlugin(tracktion_engine::PluginCreationInfo info);

        static const char *getPluginName() { return NEEDS_TRANS("DrumSampler"); }

        static const char *xmlTypeName;

        juce::String getName() override { return TRANS("DrumSampler"); }

        juce::String getPluginType() override { return xmlTypeName; }

        juce::String getShortName(int) override { return "DrmSmplr"; }

        juce::String getSelectableDescription() override { return TRANS("DrumSampler"); }

    };

}


