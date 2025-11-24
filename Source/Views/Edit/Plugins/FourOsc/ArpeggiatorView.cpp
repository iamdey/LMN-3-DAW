#include "ArpeggiatorView.h"

ArpeggiatorView::ArpeggiatorView(tracktion::FourOscPlugin *p,
                                 app_services::MidiCommandManager &mcm)
    : plugin(p), viewModel(p), midiCommandManager(mcm), pluginKnobs(mcm, 4),
      midiSourceID(tracktion::createUniqueMPESourceID()) {

    // Find the AudioTrack that contains this plugin
    for (auto at : tracktion::getAudioTracks(plugin->edit)) {
        // Check if this track contains our plugin
        for (int i = 0; i < at->pluginList.size(); ++i) {
            if (at->pluginList[i] == plugin) {
                audioTrack = at;
                break;
            }
        }
        if (audioTrack != nullptr)
            break;
    }
    titleLabel.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(),
                                  getHeight() * 0.1f, juce::Font::plain));
    titleLabel.setText("4OSC: ARPEGGIATOR", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    // Knob 1: Mode (Off, Up, Down, UpDown, Random)
    pluginKnobs.getKnob(0)->getLabel().setText("Mode", juce::dontSendNotification);
    pluginKnobs.getKnob(0)->getSlider().setRange(0, 4, 1);

    // Knob 2: Rate (1-16 Hz)
    pluginKnobs.getKnob(1)->getLabel().setText("Rate", juce::dontSendNotification);
    pluginKnobs.getKnob(1)->getSlider().setRange(1.0, 16.0, 0.5);
    pluginKnobs.getKnob(1)->getSlider().setNumDecimalPlacesToDisplay(1);

    // Knob 3: Octaves (1-4)
    pluginKnobs.getKnob(2)->getLabel().setText("Octaves", juce::dontSendNotification);
    pluginKnobs.getKnob(2)->getSlider().setRange(1, 4, 1);

    // Knob 4: Gate (0.05-1.0)
    pluginKnobs.getKnob(3)->getLabel().setText("Gate", juce::dontSendNotification);
    pluginKnobs.getKnob(3)->getSlider().setRange(0.05, 1.0, 0.05);
    pluginKnobs.getKnob(3)->getSlider().setNumDecimalPlacesToDisplay(2);

    addAndMakeVisible(pluginKnobs);

    midiCommandManager.addListener(this);
    viewModel.addListener(this);
}

ArpeggiatorView::~ArpeggiatorView() {
    stopArpeggio();
    midiCommandManager.removeListener(this);
    viewModel.removeListener(this);
}

void ArpeggiatorView::paint(juce::Graphics &g) {
    g.fillAll(appLookAndFeel.backgroundColour);

    // Draw status info
    g.setColour(appLookAndFeel.whiteColour);
    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(),
                         getHeight() * 0.04f, juce::Font::plain));

    juce::String statusText = "Status: ";
    statusText += viewModel.isEnabled() ? "ON" : "OFF";
    statusText += " | Mode: " + getModeString();
    statusText += " | Notes: " + juce::String(viewModel.getCurrentNotes().size());

    g.drawText(statusText, 0, getHeight() * 0.85f, getWidth(), getHeight() * 0.05f,
               juce::Justification::centred);
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
    pluginKnobs.setGridSpacing(knobSpacing);
    pluginKnobs.setBounds(bounds);
}

void ArpeggiatorView::encoder1Increased() {
    if (isShowing()) {
        if (midiCommandManager.getFocusedComponent() == this) {
            viewModel.incrementMode();
        }
    }
}

void ArpeggiatorView::encoder1Decreased() {
    if (isShowing()) {
        if (midiCommandManager.getFocusedComponent() == this) {
            viewModel.decrementMode();
        }
    }
}

void ArpeggiatorView::encoder2Increased() {
    if (isShowing()) {
        if (midiCommandManager.getFocusedComponent() == this) {
            viewModel.incrementRate();
        }
    }
}

void ArpeggiatorView::encoder2Decreased() {
    if (isShowing()) {
        if (midiCommandManager.getFocusedComponent() == this) {
            viewModel.decrementRate();
        }
    }
}

void ArpeggiatorView::encoder3Increased() {
    if (isShowing()) {
        if (midiCommandManager.getFocusedComponent() == this) {
            viewModel.incrementOctaves();
        }
    }
}

void ArpeggiatorView::encoder3Decreased() {
    if (isShowing()) {
        if (midiCommandManager.getFocusedComponent() == this) {
            viewModel.decrementOctaves();
        }
    }
}

void ArpeggiatorView::encoder4Increased() {
    if (isShowing()) {
        if (midiCommandManager.getFocusedComponent() == this) {
            viewModel.incrementGate();
        }
    }
}

void ArpeggiatorView::encoder4Decreased() {
    if (isShowing()) {
        if (midiCommandManager.getFocusedComponent() == this) {
            viewModel.decrementGate();
        }
    }
}

void ArpeggiatorView::noteOnPressed(int noteNumber) {
    if (isShowing()) {
        if (midiCommandManager.getFocusedComponent() == this) {
            // Add note to buffer when pressed
            viewModel.addNoteToBuffer(noteNumber);

            // Start arpeggio if enabled and this is the first note
            if (viewModel.isEnabled()) {
                startArpeggio();
            }

            repaint();
        }
    }
}

void ArpeggiatorView::noteOffPressed(int noteNumber) {
    if (isShowing()) {
        if (midiCommandManager.getFocusedComponent() == this) {
            // Remove note from buffer when released
            viewModel.removeNoteFromBuffer(noteNumber);

            // Stop arpeggio if no notes left
            if (viewModel.getCurrentNotes().isEmpty()) {
                stopArpeggio();
            }

            repaint();
        }
    }
}

void ArpeggiatorView::controlButtonPressed() {
    // Toggle arpeggiator on/off with control button
    viewModel.toggleEnabled();

    if (viewModel.isEnabled() && !viewModel.getCurrentNotes().isEmpty()) {
        startArpeggio();
    } else {
        stopArpeggio();
    }

    repaint();
}

void ArpeggiatorView::controlButtonReleased() {
    // No action needed on release
}

void ArpeggiatorView::parametersChanged() {
    pluginKnobs.getKnob(0)->getSlider().setValue(
        static_cast<int>(viewModel.getMode()), juce::dontSendNotification);
    pluginKnobs.getKnob(1)->getSlider().setValue(
        viewModel.getRate(), juce::dontSendNotification);
    pluginKnobs.getKnob(2)->getSlider().setValue(
        viewModel.getOctaves(), juce::dontSendNotification);
    pluginKnobs.getKnob(3)->getSlider().setValue(
        viewModel.getGate(), juce::dontSendNotification);

    // Update timer interval based on rate
    if (isTimerRunning()) {
        int intervalMs = static_cast<int>(1000.0f / viewModel.getRate());
        startTimer(intervalMs);
    }

    repaint();
}

void ArpeggiatorView::timerCallback() {
    if (viewModel.isEnabled() && !viewModel.getCurrentNotes().isEmpty()) {
        playNextNote();
    } else {
        // If no notes or disabled, ensure we stop any playing note
        stopArpeggio();
    }
}

void ArpeggiatorView::startArpeggio() {
    if (!isTimerRunning() && viewModel.isEnabled()) {
        currentNoteIndex = 0;
        currentOctave = 0;
        int intervalMs = static_cast<int>(1000.0f / viewModel.getRate());
        startTimer(intervalMs);
    }
}

void ArpeggiatorView::stopArpeggio() {
    stopTimer();
    stopCurrentNote();
    currentNoteIndex = 0;
    currentOctave = 0;
}

void ArpeggiatorView::playNextNote() {
    auto notes = viewModel.getCurrentNotes();
    if (notes.isEmpty()) {
        stopArpeggio();
        return;
    }

    // Stop previous note
    stopCurrentNote();

    auto mode = viewModel.getMode();
    int octaves = viewModel.getOctaves();

    // Calculate next note based on mode
    int noteToPlay = -1;

    switch (mode) {
        case app_view_models::ArpeggiatorViewModel::Mode::Off:
            stopArpeggio();
            return;

        case app_view_models::ArpeggiatorViewModel::Mode::Up:
            noteToPlay = notes[currentNoteIndex] + (currentOctave * 12);
            currentNoteIndex++;
            if (currentNoteIndex >= notes.size()) {
                currentNoteIndex = 0;
                currentOctave++;
                if (currentOctave >= octaves) {
                    currentOctave = 0;
                }
            }
            break;

        case app_view_models::ArpeggiatorViewModel::Mode::Down:
            noteToPlay = notes[currentNoteIndex] + ((octaves - 1 - currentOctave) * 12);
            currentNoteIndex++;
            if (currentNoteIndex >= notes.size()) {
                currentNoteIndex = 0;
                currentOctave++;
                if (currentOctave >= octaves) {
                    currentOctave = 0;
                }
            }
            break;

        case app_view_models::ArpeggiatorViewModel::Mode::UpDown: {
            int totalSteps = (notes.size() * octaves * 2) - 2;
            int step = (currentNoteIndex + currentOctave * notes.size());

            if (step < notes.size() * octaves) {
                // Going up
                int octaveOffset = step / notes.size();
                int noteIndex = step % notes.size();
                noteToPlay = notes[noteIndex] + (octaveOffset * 12);
            } else {
                // Going down
                int downStep = step - (notes.size() * octaves);
                int octaveOffset = (octaves - 1) - (downStep / notes.size());
                int noteIndex = notes.size() - 1 - (downStep % notes.size());
                noteToPlay = notes[noteIndex] + (octaveOffset * 12);
            }

            currentNoteIndex++;
            if (currentNoteIndex >= totalSteps) {
                currentNoteIndex = 0;
            }
            break;
        }

        case app_view_models::ArpeggiatorViewModel::Mode::Random: {
            juce::Random random;
            int randomNoteIndex = random.nextInt(notes.size());
            int randomOctave = random.nextInt(octaves);
            noteToPlay = notes[randomNoteIndex] + (randomOctave * 12);
            break;
        }
    }

    // Play the note
    if (noteToPlay >= 0 && noteToPlay < 128 && audioTrack != nullptr) {
        lastPlayedNote = noteToPlay;
        noteIsPlaying = true;

        // Inject MIDI note-on message to the track containing FourOsc
        // This uses Tracktion Engine's real-time MIDI injection system
        auto noteOnMessage = juce::MidiMessage::noteOn(1, noteToPlay, (juce::uint8)100);
        audioTrack->injectLiveMidiMessage(noteOnMessage, midiSourceID);
    }
}

void ArpeggiatorView::stopCurrentNote() {
    if (noteIsPlaying && lastPlayedNote >= 0 && audioTrack != nullptr) {
        // Inject MIDI note-off message
        auto noteOffMessage = juce::MidiMessage::noteOff(1, lastPlayedNote);
        audioTrack->injectLiveMidiMessage(noteOffMessage, midiSourceID);

        noteIsPlaying = false;
        lastPlayedNote = -1;
    }
}

juce::String ArpeggiatorView::getModeString() const {
    switch (viewModel.getMode()) {
        case app_view_models::ArpeggiatorViewModel::Mode::Off:
            return "OFF";
        case app_view_models::ArpeggiatorViewModel::Mode::Up:
            return "UP";
        case app_view_models::ArpeggiatorViewModel::Mode::Down:
            return "DOWN";
        case app_view_models::ArpeggiatorViewModel::Mode::UpDown:
            return "UP-DOWN";
        case app_view_models::ArpeggiatorViewModel::Mode::Random:
            return "RANDOM";
        default:
            return "UNKNOWN";
    }
}
