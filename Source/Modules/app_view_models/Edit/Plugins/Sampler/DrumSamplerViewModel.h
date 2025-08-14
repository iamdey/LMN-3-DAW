#include <yaml-cpp/yaml.h>
namespace app_view_models {
namespace IDs {
const juce::Identifier DRUM_SAMPLER_VIEW_STATE("DRUM_SAMPLER_VIEW_STATE");
}

struct DrumKitEntry {
    juce::String name;
    juce::File mappingFile;

    // Optional: Allow sorting by name
    bool operator<(const DrumKitEntry& other) const {
        return name.compareIgnoreCase(other.name) < 0;
    }
};


class DrumSamplerViewModel : public app_view_models::SamplerViewModel {
  public:
    explicit DrumSamplerViewModel(internal_plugins::DrumSamplerPlugin *sampler);

    juce::StringArray getItemNames() override;

    juce::String getTitle() override { return "Kits"; }

    juce::String getSelectedItemName() override;

    void setSelectedSoundIndex(int noteNumber) override;

    void selectedIndexChanged(int newIndex) override;

    void valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged,
                                  const juce::Identifier &property) override;

  private:
    juce::Array<DrumKitEntry> drumKits;
    juce::Array<juce::File> drumSampleFiles;

    void readMappingFileIntoSampler(const juce::File &mappingFile,
                                    bool shouldUpdateSounds);
    void updateDrumKits();
    void updateThumb();
};

} // namespace app_view_models
