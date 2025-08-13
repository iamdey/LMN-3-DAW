#pragma once

namespace app_view_models {

namespace IDs {

const juce::Identifier MIXER_TRACK_VIEW_STATE("MIXER_TRACK_VIEW_STATE");

}

class MixerTrackViewModel : public juce::ValueTree::Listener,
                            public FlaggedAsyncUpdater {
  public:
    MixerTrackViewModel(tracktion::Track::Ptr t);
    ~MixerTrackViewModel() override;

    tracktion::VolumeAndPanPlugin *getVolumeAndPanPlugin();

    class Listener {
      public:
        virtual ~Listener() = default;

        virtual void panChanged(double) {}
        virtual void volumeChanged(double) {}
        virtual void soloStateChanged(bool) {}
        virtual void muteStateChanged(bool) {}
    };

    void addListener(Listener *l);
    void removeListener(Listener *l);

  private:
    tracktion::Track::Ptr track;
    juce::ValueTree state;
    juce::ListenerList<Listener> listeners;

    // Async updater flags
    bool shouldUpdateVolume = false;
    bool shouldUpdatePan = false;
    bool shouldUpdateMute = false;
    bool shouldUpdateSolo = false;

    void handleAsyncUpdate() override;

    void valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged,
                                  const juce::Identifier &property) override;
};

} // namespace app_view_models
