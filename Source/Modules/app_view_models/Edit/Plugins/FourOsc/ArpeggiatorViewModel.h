#pragma once
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <tracktion_engine/tracktion_engine.h>

namespace app_view_models {

class ArpeggiatorViewModel {
  public:
    enum class Mode { off = 0, up, down, upDown, random };

    class Listener {
      public:
        virtual ~Listener() = default;
        virtual void parametersChanged() {}
        virtual void notesChanged() {}
    };

    explicit ArpeggiatorViewModel(tracktion::FourOscPlugin *p);

    void addListener(Listener *l);
    void removeListener(Listener *l);

    Mode getMode() const;
    void incrementMode();
    void decrementMode();

    double getRate() const;
    void incrementRate();
    void decrementRate();

    int getOctaves() const;
    void incrementOctaves();
    void decrementOctaves();

    double getGate() const;
    void incrementGate();
    void decrementGate();

    bool isEnabled() const;
    void toggleEnabled();

    const juce::Array<int> &getCurrentNotes() const;
    void addNote(int noteNumber);
    void removeNote(int noteNumber);
    int getNextNote();

    int getIntervalMs() const;
    juce::String getModeName() const;

  private:
    tracktion::FourOscPlugin *plugin;
    Mode mode{Mode::up};
    double rateHz{8.0};
    int octaves{1};
    double gate{0.8};
    bool enabled{false};

    juce::Array<int> noteBuffer;
    int currentNoteIndex{0};
    int direction{1};
    juce::Random random;

    juce::ListenerList<Listener> listeners;

    void notifyParametersChanged();
    void notifyNotesChanged();
    void clampSettings();
};

} // namespace app_view_models
