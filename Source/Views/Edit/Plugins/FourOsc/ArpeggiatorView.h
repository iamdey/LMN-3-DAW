#pragma once
#include "AppLookAndFeel.h"
#include "Knobs.h"
#include <app_services/app_services.h>
#include <app_view_models/app_view_models.h>
#include <juce_gui_basics/juce_gui_basics.h>

class ArpeggiatorView : public juce::Component,
                         public app_services::MidiCommandManager::Listener,
                         public app_view_models::ArpeggiatorViewModel::Listener,
                         private juce::Timer {
  public:
    ArpeggiatorView(tracktion::FourOscPlugin *p,
                    app_services::MidiCommandManager &mcm);

    ~ArpeggiatorView() override;

    void paint(juce::Graphics &g) override;
    void resized() override;

    // MIDI encoder handlers
    void encoder1Increased() override;
    void encoder1Decreased() override;

    void encoder2Increased() override;
    void encoder2Decreased() override;

    void encoder3Increased() override;
    void encoder3Decreased() override;

    void encoder4Increased() override;
    void encoder4Decreased() override;

    // MIDI note handlers
    void noteOnPressed(int noteNumber) override;
    void noteOffPressed(int noteNumber) override;

    void controlButtonPressed() override;
    void controlButtonReleased() override;

    // ViewModel listener
    void parametersChanged() override;

  private:
    tracktion::FourOscPlugin *plugin;
    app_view_models::ArpeggiatorViewModel viewModel;
    app_services::MidiCommandManager &midiCommandManager;

    juce::Label titleLabel;
    Knobs pluginKnobs;
    AppLookAndFeel appLookAndFeel;

    // MIDI injection
    tracktion::AudioTrack *audioTrack = nullptr;
    tracktion::MPESourceID midiSourceID;

    // Arpeggiator state
    int currentNoteIndex = 0;
    int currentOctave = 0;
    bool noteIsPlaying = false;
    int lastPlayedNote = -1;

    // Timer callback for arpeggio
    void timerCallback() override;

    // Helper methods
    void startArpeggio();
    void stopArpeggio();
    void playNextNote();
    void stopCurrentNote();
    juce::String getModeString() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ArpeggiatorView)
};
