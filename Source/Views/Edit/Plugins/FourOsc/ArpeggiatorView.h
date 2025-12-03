#pragma once
#include "AppLookAndFeel.h"
#include "Knobs.h"
#include <app_services/app_services.h>
#include <app_view_models/app_view_models.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <tracktion_engine/tracktion_engine.h>

class ArpeggiatorView : public juce::Component,
                        private juce::Timer,
                        public app_services::MidiCommandManager::Listener,
                        public app_view_models::ArpeggiatorViewModel::Listener {
  public:
    ArpeggiatorView(tracktion::FourOscPlugin *p,
                    app_services::MidiCommandManager &mcm);
    ~ArpeggiatorView() override;

    void paint(juce::Graphics &g) override;
    void resized() override;

    void encoder1Increased() override;
    void encoder1Decreased() override;
    void encoder2Increased() override;
    void encoder2Decreased() override;
    void encoder3Increased() override;
    void encoder3Decreased() override;
    void encoder4Increased() override;
    void encoder4Decreased() override;
    void controlButtonPressed() override;

    void noteOnPressed(int noteNumber) override;
    void noteOffPressed(int noteNumber) override;

    void parametersChanged() override;
    void notesChanged() override;

  private:
    app_view_models::ArpeggiatorViewModel viewModel;
    app_services::MidiCommandManager &midiCommandManager;
    AppLookAndFeel lookAndFeel;
    Knobs knobs;
    juce::Label titleLabel;
    juce::Label statusLabel;

    tracktion::AudioTrack *targetTrack{nullptr};
    tracktion::MPESourceID sourceId{tracktion::createUniqueMPESourceID()};
    int lastNote{-1};

    void timerCallback() override;
    void startIfNeeded();
    void stopCurrentNote();
    void playNextNote();
    void updateStatusLabel();
    tracktion::AudioTrack *findTrackForPlugin(tracktion::FourOscPlugin *plugin);
};
