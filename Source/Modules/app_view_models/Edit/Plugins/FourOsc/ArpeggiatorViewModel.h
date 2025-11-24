#pragma once
namespace app_view_models {
class ArpeggiatorViewModel : public juce::ValueTree::Listener,
                              public FlaggedAsyncUpdater {
  public:
    ArpeggiatorViewModel(tracktion::FourOscPlugin *p);
    ~ArpeggiatorViewModel() override;

    // Arpeggiator modes
    enum class Mode {
        Off = 0,
        Up,
        Down,
        UpDown,
        Random
    };

    // Getters
    bool isEnabled() const;
    Mode getMode() const;
    float getRate() const;      // In Hz (1-16 Hz)
    int getOctaves() const;      // 1-4 octaves
    float getGate() const;       // 0.0-1.0 (gate length)

    // Setters (increment/decrement pattern)
    void toggleEnabled();

    void incrementMode();
    void decrementMode();

    void incrementRate();
    void decrementRate();

    void incrementOctaves();
    void decrementOctaves();

    void incrementGate();
    void decrementGate();

    // Note handling for arpeggio
    void addNoteToBuffer(int noteNumber);
    void removeNoteFromBuffer(int noteNumber);
    void clearNoteBuffer();
    juce::Array<int> getCurrentNotes() const;

    class Listener {
      public:
        virtual ~Listener() = default;
        virtual void parametersChanged() {}
    };

    void valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged,
                                  const juce::Identifier &property) override;

    void addListener(Listener *l);
    void removeListener(Listener *l);

  private:
    tracktion::FourOscPlugin *plugin;
    juce::ListenerList<Listener> listeners;

    // Arpeggiator state
    bool enabled = false;
    Mode mode = Mode::Up;
    float rate = 8.0f;           // Default 8 Hz
    int octaves = 1;             // Default 1 octave
    float gate = 0.8f;           // Default 80% gate

    // Note buffer for arpeggio
    juce::Array<int> noteBuffer;

    void handleAsyncUpdate() override;
    bool shouldUpdateParameters = false;
};

} // namespace app_view_models
