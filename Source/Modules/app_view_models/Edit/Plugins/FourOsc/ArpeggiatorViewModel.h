#pragma once
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <tracktion_engine/tracktion_engine.h>

namespace internal_plugins {
class CustomFourOscPlugin;
}

namespace app_view_models {

class ArpeggiatorViewModel {
  public:
    enum class Mode { off = 0, up, down, upDown, random };

    class Listener {
      public:
        virtual ~Listener() = default;
        virtual void parametersChanged() {}
    };

    explicit ArpeggiatorViewModel(internal_plugins::CustomFourOscPlugin *p);

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
    bool isTempoSyncEnabled() const;
    void toggleTempoSync();

    juce::String getModeName() const;

  private:
    internal_plugins::CustomFourOscPlugin *plugin;

    juce::ListenerList<Listener> listeners;

    void notifyParametersChanged();
};

} // namespace app_view_models
