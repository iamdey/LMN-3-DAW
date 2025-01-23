#include "LoadSaveSongListViewModel.h"

namespace app_view_models {

LoadSaveSongListViewModel::LoadSaveSongListViewModel(
    tracktion::Edit &e, juce::AudioDeviceManager &dm,
    const juce::String &appName)
    : deviceManager(dm), state(e.state.getOrCreateChildWithName(
                             IDs::LOAD_SAVE_SONG_VIEW_STATE, nullptr)),
      itemListState(state, songNames.size()) {
    auto userAppDataDirectory = juce::File::getSpecialLocation(
        juce::File::userApplicationDataDirectory);
    juce::File savedDirectory =
        userAppDataDirectory.getChildFile(appName).getChildFile("saved");

    loadSongList(savedDirectory);
    itemListState.addListener(this);
}

LoadSaveSongListViewModel::~LoadSaveSongListViewModel() {
    itemListState.removeListener(this);
}

juce::StringArray LoadSaveSongListViewModel::getItemNames() {
    return songNames;
}

juce::String LoadSaveSongListViewModel::getSelectedItem() {
    return songNames[itemListState.getSelectedItemIndex()];
}

void LoadSaveSongListViewModel::selectedIndexChanged(int newIndex) {
    // Here you can add the logic you need when you change the index selected
}

void LoadSaveSongListViewModel::loadSongList(const juce::File &directory) {
    songNames.clear();
    songNames.add("Add");

    juce::Array<juce::File> songFiles =
        directory.findChildFiles(juce::File::findFiles, false, "*.xml");

    for (const auto &file : songFiles) {
        songNames.add(file.getFileNameWithoutExtension());
    }

    itemListState.listSize = songNames.size();
    itemListState.setSelectedItemIndex(
        0); // Optional: Reset to the first item or any other
    itemListState.addListener(this);
}
void saveEditState(const tracktion::engine::Edit &edit,
                   const juce::File &file) {
    // Serialize Edit status to XML
    auto xml = edit.state.createXml();

    // Save the XML to a file
    if (xml) {
        if (!xml->writeTo(file)) {
            // Handle error if write fails
            DBG("Failed to write Edit state to file.");
        }
    } else {
        // Handling error if serialization fails
        DBG("Failed to create XML from Edit state.");
    }
}
void loadEditState(tracktion::engine::Edit &edit, const juce::File &file) {
    // Read the XML from the file
    auto xml = juce::XmlDocument::parse(file);

    if (xml) {
        // Deserialize XML to restore Edit status
        edit.state = juce::ValueTree::fromXml(*xml);
    } else {
        // Handle error if file reading fails
        DBG("Failed to read Edit state from file.");
    }
}

} // namespace app_view_models
