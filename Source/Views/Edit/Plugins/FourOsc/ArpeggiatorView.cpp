#include "ArpeggiatorView.h"
#include <internal_plugins/CustomFourOscPlugin/CustomFourOscPlugin.h>

ArpeggiatorView::ArpeggiatorView(tracktion::FourOscPlugin *p,
                                 app_services::MidiCommandManager &mcm)
    : viewModel(dynamic_cast<internal_plugins::CustomFourOscPlugin *>(p)),
      midiCommandManager(mcm), knobs(mcm, 4) {
    titleLabel.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(),
                                  getHeight() * 0.1f, juce::Font::plain));
    titleLabel.setText("4OSC: Arpeggiator", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    statusLabel.setJustificationType(juce::Justification::centred);
    statusLabel.setColour(juce::Label::textColourId, lookAndFeel.textColour);
    addAndMakeVisible(statusLabel);

    tempoSyncButton.setButtonText("Tempo Sync");
    tempoSyncButton.onClick = [this]() { viewModel.toggleTempoSync(); };
    tempoSyncButton.setColour(juce::ToggleButton::textColourId,
                              lookAndFeel.textColour);
    addAndMakeVisible(tempoSyncButton);

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
    attachSliderCallbacks();

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

    int buttonWidth = juce::jmin(getWidth() / 3, 200);
    int buttonHeight =
        juce::jmax(24, juce::roundToInt(getHeight() * 0.08f));
    int buttonY = juce::roundToInt(getHeight() * 0.72f);
    tempoSyncButton.setBounds((getWidth() - buttonWidth) / 2, buttonY,
                              buttonWidth, buttonHeight);

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

void ArpeggiatorView::encoder1ButtonReleased() {
    if (isShowing() && midiCommandManager.getFocusedComponent() == this) {
        viewModel.toggleTempoSync();
    }
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
    const juce::ScopedValueSetter<bool> updating(sliderUpdateInProgress, true);
    knobs.getKnob(0)->getSlider().setValue(static_cast<int>(viewModel.getMode()),
                                           juce::dontSendNotification);
    knobs.getKnob(1)->getSlider().setValue(viewModel.getRate(),
                                           juce::dontSendNotification);
    knobs.getKnob(2)->getSlider().setValue(viewModel.getOctaves(),
                                           juce::dontSendNotification);
    knobs.getKnob(3)->getSlider().setValue(viewModel.getGate(),
                                           juce::dontSendNotification);
    tempoSyncButton.setToggleState(viewModel.isTempoSyncEnabled(),
                                   juce::dontSendNotification);
    updateStatusLabel();
}

void ArpeggiatorView::updateStatusLabel() {
    juce::String status = viewModel.isEnabled() ? "ON" : "OFF";
    bool tempoSync = viewModel.isTempoSyncEnabled();
    juce::String rateValue = juce::String(viewModel.getRate(), 1);
    juce::String rateSuffix = tempoSync ? " steps/beat" : " Hz";
    juce::String syncStatus = tempoSync ? "Tempo" : "Free";
    statusLabel.setText(
        "Status: " + status + " | Mode: " + viewModel.getModeName() +
            " | Rate: " + rateValue + rateSuffix + " | Sync: " + syncStatus,
        juce::dontSendNotification);
}

void ArpeggiatorView::attachSliderCallbacks() {
    auto &modeSlider = knobs.getKnob(0)->getSlider();
    modeSlider.onValueChange = [this, &modeSlider]() {
        if (sliderUpdateInProgress)
            return;
        auto modeValue = juce::roundToInt(modeSlider.getValue());
        viewModel.setMode(static_cast<app_view_models::ArpeggiatorViewModel::Mode>(
            modeValue));
    };

    auto &rateSlider = knobs.getKnob(1)->getSlider();
    rateSlider.onValueChange = [this, &rateSlider]() {
        if (sliderUpdateInProgress)
            return;
        viewModel.setRate(rateSlider.getValue());
    };

    auto &octavesSlider = knobs.getKnob(2)->getSlider();
    octavesSlider.onValueChange = [this, &octavesSlider]() {
        if (sliderUpdateInProgress)
            return;
        viewModel.setOctaves(juce::roundToInt(octavesSlider.getValue()));
    };

    auto &gateSlider = knobs.getKnob(3)->getSlider();
    gateSlider.onValueChange = [this, &gateSlider]() {
        if (sliderUpdateInProgress)
            return;
        viewModel.setGate(gateSlider.getValue());
    };
}

void ArpeggiatorView::visibilityChanged() {
    juce::Component::visibilityChanged();
    if (isShowing())
        midiCommandManager.setFocusedComponent(this);
}
