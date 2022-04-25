#pragma once

namespace app_view_models {
    namespace IDs {
        const juce::Identifier SETTINGS_VIEW_STATE("SETTINGS_VIEW_STATE");
        const juce::Identifier SETTINGS_LIST_VIEW_STATE("SETTINGS_LIST_VIEW_STATE");
    }

    class SettingsListViewModel {
    public:
        SettingsListViewModel(tracktion_engine::Edit& e, juce::AudioDeviceManager& dm);

        juce::StringArray getItemNames();
        juce::String getSelectedItem();

        const juce::String deviceTypeSettingName = "Device Type";
        const juce::String outputSettingName = "Output";
        const juce::String sampleRateSettingName = "Sample Rate";
        const juce::String audioBufferSizeSettingName = "Audio Buffer Size";

    private:
        juce::AudioDeviceManager& deviceManager;
        juce::ValueTree state;
        juce::StringArray settingNames = juce::StringArray(juce::Array<juce::String>({deviceTypeSettingName,
                                                                                            outputSettingName,
                                                                                            sampleRateSettingName,
                                                                                            audioBufferSizeSettingName,
                                                                                            }));

    public:
        // Must appear below the other variables since it needs to be initialized last
        ItemListState itemListState;
    };

}



