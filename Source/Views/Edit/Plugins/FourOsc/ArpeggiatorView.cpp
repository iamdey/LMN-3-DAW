#include "ArpeggiatorView.h"

ArpeggiatorView::ArpeggiatorView(tracktion::FourOscPlugin *p,
                                 app_services::MidiCommandManager &mcm)
    : viewModel(p), midiCommandManager(mcm), knobs(mcm, 4) {
    targetTrack = findTrackForPlugin(p);

    titleLabel.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(),
                                  getHeight() * 0.1f, juce::Font::plain));
    titleLabel.setText("4OSC: Arpeggiator", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    statusLabel.setJustificationType(juce::Justification::centred);
    statusLabel.setColour(juce::Label::textColourId, lookAndFeel.textColour);
    addAndMakeVisible(statusLabel);

    knobs.getKnob(0)->getLabel().setText("Mode", juce::dontSendNotification);
    knobs.getKnob(0)->getSlider().setRange(0, 4, 1);

    knobs.getKnob(1)->getLabel().setText("Rate (Hz)",
                                         juce::dontSendNotification);
    knobs.getKnob(1)->getSlider().setRange(1.0, 16.0, 0.5);
    knobs.getKnob(1)->getSlider().setNumDecimalPlacesToDisplay(1);

    knobs.getKnob(2)->getLabel().setText("Octaves",
                                         juce::dontSendNotification);
    knobs.getKnob(2)->getSlider().setRange(1, 4, 1);

    knobs.getKnob(3)->getLabel().setText("Gate",
                                         juce::dontSendNotification);
    knobs.getKnob(3)->getSlider().setRange(0.05, 1.0, 0.05);
    knobs.getKnob(3)->getSlider().setNumDecimalPlacesToDisplay(2);

    addAndMakeVisible(knobs);

    midiCommandManager.addListener(this);
    viewModel.addListener(this);
    parametersChanged();
}

ArpeggiatorView::~ArpeggiatorView() {
    stopTimer();
    stopCurrentNote();
    midiCommandManager.removeListener(this);
    viewModel.removeListener(this);
}

void ArpeggiatorView::paint(juce::Graphics &g) {
    g.fillAll(
        getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void ArpeggiatorView::resized() {
    titleLabel.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(),
                                  getHeight() * 0.1f, juce::Font::plain));
    titleLabel.setBounds(0, getHeight() * .05, getWidth(), getHeight() * .1);

    int knobWidth = getWidth() / 8;
    int knobHeight = getHeight() / 3;
    int knobSpacing = knobWidth;
    int width = (4 * knobWidth) + (3 * knobSpacing);
    int height = knobHeight;
    int startX = (getWidth() / 2) - (width / 2);
    int startY = (getHeight() / 2) - (knobHeight / 2);
    juce::Rectangle<int> bounds(startX, startY, width, height);
    knobs.setGridSpacing(knobSpacing);
    knobs.setBounds(bounds);

    statusLabel.setBounds(0, getHeight() * .82, getWidth(), getHeight() * .1);
}

void ArpeggiatorView::encoder1Increased() {
    if (isShowing() && midiCommandManager.getFocusedComponent() == this) {
        viewModel.incrementMode();
        startIfNeeded();
    }
}

void ArpeggiatorView::encoder1Decreased() {
    if (isShowing() && midiCommandManager.getFocusedComponent() == this) {
        viewModel.decrementMode();
        startIfNeeded();
    }
}

void ArpeggiatorView::encoder2Increased() {
    if (isShowing() && midiCommandManager.getFocusedComponent() == this) {
        viewModel.incrementRate();
        startIfNeeded();
    }
}

void ArpeggiatorView::encoder2Decreased() {
    if (isShowing() && midiCommandManager.getFocusedComponent() == this) {
        viewModel.decrementRate();
        startIfNeeded();
    }
}

void ArpeggiatorView::encoder3Increased() {
    if (isShowing() && midiCommandManager.getFocusedComponent() == this) {
        viewModel.incrementOctaves();
    }
}

void ArpeggiatorView::encoder3Decreased() {
    if (isShowing() && midiCommandManager.getFocusedComponent() == this) {
        viewModel.decrementOctaves();
    }
}

void ArpeggiatorView::encoder4Increased() {
    if (isShowing() && midiCommandManager.getFocusedComponent() == this) {
        viewModel.incrementGate();
    }
}

void ArpeggiatorView::encoder4Decreased() {
    if (isShowing() && midiCommandManager.getFocusedComponent() == this) {
        viewModel.decrementGate();
    }
}

void ArpeggiatorView::controlButtonPressed() { viewModel.toggleEnabled(); }

void ArpeggiatorView::noteOnPressed(int noteNumber) {
    if (isShowing() && midiCommandManager.getFocusedComponent() == this) {
        viewModel.addNote(noteNumber);
        startIfNeeded();
    }
}

void ArpeggiatorView::noteOffPressed(int noteNumber) {
    if (isShowing() && midiCommandManager.getFocusedComponent() == this) {
        viewModel.removeNote(noteNumber);
        if (viewModel.getCurrentNotes().isEmpty()) {
            stopTimer();
            stopCurrentNote();
        } else {
            startIfNeeded();
        }
    }
}

void ArpeggiatorView::parametersChanged() {
    knobs.getKnob(0)->getSlider().setValue(static_cast<int>(viewModel.getMode()),
                                           juce::dontSendNotification);
    knobs.getKnob(1)->getSlider().setValue(viewModel.getRate(),
                                           juce::dontSendNotification);
    knobs.getKnob(2)->getSlider().setValue(viewModel.getOctaves(),
                                           juce::dontSendNotification);
    knobs.getKnob(3)->getSlider().setValue(viewModel.getGate(),
                                           juce::dontSendNotification);
    updateStatusLabel();
}

void ArpeggiatorView::notesChanged() { startIfNeeded(); }

void ArpeggiatorView::timerCallback() { playNextNote(); }

void ArpeggiatorView::startIfNeeded() {
    if (viewModel.isEnabled() && !viewModel.getCurrentNotes().isEmpty() &&
        viewModel.getMode() != app_view_models::ArpeggiatorViewModel::Mode::off) {
        startTimer(viewModel.getIntervalMs());
    } else {
        stopTimer();
        stopCurrentNote();
    }
    updateStatusLabel();
}

void ArpeggiatorView::stopCurrentNote() {
    if (lastNote < 0 || targetTrack == nullptr)
        return;

    auto noteOff = juce::MidiMessage::noteOff(1, lastNote);
    targetTrack->injectLiveMidiMessage(noteOff, sourceId);
    lastNote = -1;
}

void ArpeggiatorView::playNextNote() {
    if (targetTrack == nullptr) {
        stopTimer();
        return;
    }

    stopCurrentNote();

    auto nextNote = viewModel.getNextNote();
    if (nextNote < 0)
        return;

    auto noteOn = juce::MidiMessage::noteOn(1, nextNote, (juce::uint8)100);
    targetTrack->injectLiveMidiMessage(noteOn, sourceId);
    lastNote = nextNote;

    auto gateMs =
        static_cast<int>(viewModel.getIntervalMs() * viewModel.getGate());
    juce::Component::SafePointer<ArpeggiatorView> safeThis(this);
    juce::Timer::callAfterDelay(gateMs, [safeThis, nextNote]() {
        if (safeThis == nullptr)
            return;
        if (safeThis->targetTrack != nullptr && safeThis->lastNote == nextNote) {
            auto noteOff = juce::MidiMessage::noteOff(1, nextNote);
            safeThis->targetTrack->injectLiveMidiMessage(noteOff,
                                                         safeThis->sourceId);
            safeThis->lastNote = -1;
        }
    });
}

void ArpeggiatorView::updateStatusLabel() {
    juce::String status = viewModel.isEnabled() ? "ON" : "OFF";
    statusLabel.setText("Status: " + status + " | Mode: " +
                            viewModel.getModeName() + " | Notes: " +
                            juce::String(viewModel.getCurrentNotes().size()),
                        juce::dontSendNotification);
}

tracktion::AudioTrack *
ArpeggiatorView::findTrackForPlugin(tracktion::FourOscPlugin *plugin) {
    auto &edit = plugin->edit;
    for (auto track : tracktion::getAudioTracks(edit)) {
        if (track->pluginList.contains(plugin))
            return track;
    }
    return nullptr;
}
