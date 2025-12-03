#include "ArpeggiatorView.h"

ArpeggiatorView::ArpeggiatorView(tracktion::FourOscPlugin *p,
                                 app_services::MidiCommandManager &mcm)
    : viewModel(p), midiCommandManager(mcm), knobs(mcm, 4) {
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
    if (isShowing() && midiCommandManager.getFocusedComponent() == this)
        viewModel.incrementMode();
}

void ArpeggiatorView::encoder1Decreased() {
    if (isShowing() && midiCommandManager.getFocusedComponent() == this)
        viewModel.decrementMode();
}

void ArpeggiatorView::encoder2Increased() {
    if (isShowing() && midiCommandManager.getFocusedComponent() == this)
        viewModel.incrementRate();
}

void ArpeggiatorView::encoder2Decreased() {
    if (isShowing() && midiCommandManager.getFocusedComponent() == this)
        viewModel.decrementRate();
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

void ArpeggiatorView::updateStatusLabel() {
    juce::String status = viewModel.isEnabled() ? "ON" : "OFF";
    juce::String rate = juce::String(viewModel.getRate(), 1);
    statusLabel.setText("Status: " + status + " | Mode: " +
                            viewModel.getModeName() + " | Rate: " + rate +
                            " Hz",
                        juce::dontSendNotification);
}
