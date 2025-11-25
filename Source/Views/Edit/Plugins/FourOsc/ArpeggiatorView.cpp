#include "ArpeggiatorView.h"

ArpeggiatorView::ArpeggiatorView(tracktion::FourOscPlugin *p,
                                 app_services::MidiCommandManager &mcm)
    : plugin(p), viewModel(p), midiCommandManager(mcm), pluginKnobs(mcm, 4) {

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

void ArpeggiatorView::controlButtonPressed() {
    // Toggle arpeggiator on/off with control button
    viewModel.toggleEnabled();
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

    repaint();
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
