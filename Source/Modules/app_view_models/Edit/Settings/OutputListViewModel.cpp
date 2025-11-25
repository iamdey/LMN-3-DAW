namespace app_view_models {
OutputListViewModel::OutputListViewModel(tracktion::Edit &e,
                                         juce::AudioDeviceManager &dm)
    : deviceManager(dm),
      state(
          e.state.getOrCreateChildWithName(IDs::SETTINGS_VIEW_STATE, nullptr)
              .getOrCreateChildWithName(IDs::OUTPUT_LIST_VIEW_STATE, nullptr)),
      itemListState(state, outputs.size()) {
    for (const auto &deviceName :
         deviceManager.getCurrentDeviceTypeObject()->getDeviceNames()) {
        outputs.add(deviceName);
    }
    itemListState.listSize = outputs.size();

    auto currentDevice = deviceManager.getAudioDeviceSetup().outputDeviceName;
    int currentOutputIndex = 0;
    if (currentDevice != "") {
        currentOutputIndex =
            outputs.indexOf(deviceManager.getCurrentAudioDevice()->getName());
        if (currentOutputIndex == -1) {
            currentOutputIndex = 0;
        }
    } else {
        juce::Logger::writeToLog("current output device name is empty");
    }
    itemListState.setSelectedItemIndex(currentOutputIndex);
    itemListState.addListener(this);
}

OutputListViewModel::~OutputListViewModel() {
    itemListState.removeListener(this);
}

juce::StringArray OutputListViewModel::getItemNames() { return outputs; }

juce::String OutputListViewModel::getSelectedItem() {
    return outputs[itemListState.getSelectedItemIndex()];
}

void OutputListViewModel::updateOutput() {
    auto setup = deviceManager.getAudioDeviceSetup();
    setup.outputDeviceName = getSelectedItem();
    auto result = deviceManager.setAudioDeviceSetup(setup, true);
    if (result != "") {
        juce::Logger::writeToLog("Error setting output device to " +
                                 getSelectedItem() + ": " + result);
    } else {
        // Save the selected output device to PropertiesFile for persistence
        saveOutputDevicePreference(getSelectedItem());
    }
}

void OutputListViewModel::saveOutputDevicePreference(const juce::String& deviceName) {
    auto userAppDataDirectory = juce::File::getSpecialLocation(
        juce::File::userApplicationDataDirectory);
    auto settingsFile = userAppDataDirectory
        .getChildFile(juce::JUCEApplication::getInstance()->getApplicationName())
        .getChildFile("audio_settings.xml");

    juce::PropertiesFile::Options options;
    options.applicationName = juce::JUCEApplication::getInstance()->getApplicationName();
    options.filenameSuffix = ".xml";
    options.osxLibrarySubFolder = "Application Support";

    juce::PropertiesFile props(settingsFile, options);
    props.setValue("outputDevice", deviceName);
    props.saveIfNeeded();

    juce::Logger::writeToLog("Saved output device preference: " + deviceName);
}

void OutputListViewModel::selectedIndexChanged(int /*newIndex*/) {
    updateOutput();
}
} // namespace app_view_models
