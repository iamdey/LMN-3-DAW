#include "FilterView.h"
#include <cmath>
#include <functional>

FilterView::FilterView(tracktion::FourOscPlugin *p,
                       app_services::MidiCommandManager &mcm)
    : viewModel(p), midiCommandManager(mcm), knobs(mcm, 8),
      filterAdsrView(p, midiCommandManager) {
    titleLabel.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(),
                                  getHeight() * 0.1f, juce::Font::plain));
    titleLabel.setText("4OSC: Filter", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    addChildComponent(filterAdsrView);

    knobs.getKnob(0)->getLabel().setText("Frequency",
                                         juce::dontSendNotification);
    knobs.getKnob(0)->getSlider().setRange(
        viewModel.getFrequencyRange().getStart(),
        viewModel.getFrequencyRange().getEnd(), 0);
    knobs.getKnob(0)->getSlider().setNumDecimalPlacesToDisplay(2);

    knobs.getKnob(1)->getLabel().setText("Resonance",
                                         juce::dontSendNotification);
    knobs.getKnob(1)->getSlider().setRange(0, 1, 0);
    knobs.getKnob(1)->getSlider().setNumDecimalPlacesToDisplay(2);

    knobs.getKnob(2)->getLabel().setText("Envelope Amount",
                                         juce::dontSendNotification);
    knobs.getKnob(2)->getSlider().setRange(0, 1, 0);
    knobs.getKnob(2)->getSlider().setNumDecimalPlacesToDisplay(2);

    knobs.getKnob(3)->getLabel().setText("Filter Type",
                                         juce::dontSendNotification);
    knobs.getKnob(3)->getSlider().setRange(0, 4, 1);

    addAndMakeVisible(knobs);

    midiCommandManager.addListener(this);
    viewModel.addListener(this);
    attachSliderCallbacks();
}

FilterView::~FilterView() {
    midiCommandManager.removeListener(this);
    viewModel.removeListener(this);
}

void FilterView::paint(juce::Graphics & /*g*/) {}

void FilterView::resized() {
    titleLabel.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(),
                                  getHeight() * 0.1f, juce::Font::plain));
    titleLabel.setBounds(0, getHeight() * .05, getWidth(), getHeight() * .1);

    filterAdsrView.setBounds(getLocalBounds());

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
}

void FilterView::encoder1Increased() {
    if (isShowing()) {
        if (midiCommandManager.getFocusedComponent() == this) {
            if (midiCommandManager.isControlDown)
                viewModel.incrementAttack();
            else
                viewModel.incrementFrequency();
        }
    }
}

void FilterView::encoder1Decreased() {
    if (isShowing()) {
        if (midiCommandManager.getFocusedComponent() == this) {
            if (midiCommandManager.isControlDown)
                viewModel.decrementAttack();
            else
                viewModel.decrementFrequency();
        }
    }
}

void FilterView::encoder2Increased() {
    if (isShowing()) {
        if (midiCommandManager.getFocusedComponent() == this) {
            if (midiCommandManager.isControlDown)
                viewModel.incrementDecay();
            else
                viewModel.incrementResonance();
        }
    }
}

void FilterView::encoder2Decreased() {
    if (isShowing()) {
        if (midiCommandManager.getFocusedComponent() == this) {
            if (midiCommandManager.isControlDown)
                viewModel.decrementDecay();
            else
                viewModel.decrementResonance();
        }
    }
}

void FilterView::encoder3Increased() {
    if (isShowing()) {
        if (midiCommandManager.getFocusedComponent() == this) {
            if (midiCommandManager.isControlDown)
                viewModel.incrementSustain();
            else
                viewModel.incrementEnvelopeAmount();
        }
    }
}

void FilterView::encoder3Decreased() {
    if (isShowing()) {
        if (midiCommandManager.getFocusedComponent() == this) {
            if (midiCommandManager.isControlDown)
                viewModel.decrementSustain();
            else
                viewModel.decrementEnvelopeAmount();
        }
    }
}

void FilterView::encoder4Increased() {
    if (isShowing()) {
        if (midiCommandManager.getFocusedComponent() == this) {
            if (midiCommandManager.isControlDown)
                viewModel.incrementRelease();
            else
                viewModel.incrementFilterType();
        }
    }
}

void FilterView::encoder4Decreased() {
    if (isShowing()) {
        if (midiCommandManager.getFocusedComponent() == this) {
            if (midiCommandManager.isControlDown)
                viewModel.decrementRelease();
            else
                viewModel.decrementFilterType();
        }
    }
}

void FilterView::controlButtonPressed() {
    filterAdsrView.setVisible(true);
    knobs.setVisible(false);
}

void FilterView::controlButtonReleased() {
    filterAdsrView.setVisible(false);
    knobs.setVisible(true);
}

void FilterView::parametersChanged() {
    const juce::ScopedValueSetter<bool> updating(sliderUpdateInProgress, true);
    knobs.getKnob(0)->getSlider().setValue(viewModel.getFrequency(),
                                           juce::dontSendNotification);
    knobs.getKnob(1)->getSlider().setValue(viewModel.getResonance(),
                                           juce::dontSendNotification);
    knobs.getKnob(2)->getSlider().setValue(viewModel.getEnvelopeAmount(),
                                           juce::dontSendNotification);
    knobs.getKnob(3)->getSlider().setValue(viewModel.getFilterType(),
                                           juce::dontSendNotification);
}

void FilterView::visibilityChanged() {
    juce::Component::visibilityChanged();
    if (isShowing())
        midiCommandManager.setFocusedComponent(this);
}

void FilterView::attachSliderCallbacks() {
    auto &frequencySlider = knobs.getKnob(0)->getSlider();
    frequencySlider.onValueChange = [this, &frequencySlider]() {
        if (sliderUpdateInProgress)
            return;
        double target = frequencySlider.getValue();
        double current = viewModel.getFrequency();
        int iterations = 0;
        while (std::abs(target - current) > 1.0 && iterations++ < 400) {
            if (target > current)
                viewModel.incrementFrequency();
            else
                viewModel.decrementFrequency();
            current = viewModel.getFrequency();
        }
    };

    auto bindNormalised =
        [this](int knobIndex,
               void (app_view_models::FilterViewModel::*inc)(),
               void (app_view_models::FilterViewModel::*dec)(),
               std::function<double()> getter) {
            auto &slider = knobs.getKnob(knobIndex)->getSlider();
            slider.onValueChange = [this, inc, dec, getter, &slider]() {
                if (sliderUpdateInProgress)
                    return;
                double target = slider.getValue();
                double current = getter();
                int iterations = 0;
                while (std::abs(target - current) > 0.005 &&
                       iterations++ < 200) {
                    if (target > current)
                        (viewModel.*inc)();
                    else
                        (viewModel.*dec)();
                    current = getter();
                }
            };
        };

    bindNormalised(1, &app_view_models::FilterViewModel::incrementResonance,
                   &app_view_models::FilterViewModel::decrementResonance,
                   [this]() { return viewModel.getResonance(); });
    bindNormalised(
        2, &app_view_models::FilterViewModel::incrementEnvelopeAmount,
        &app_view_models::FilterViewModel::decrementEnvelopeAmount,
        [this]() { return viewModel.getEnvelopeAmount(); });

    auto &filterTypeSlider = knobs.getKnob(3)->getSlider();
    filterTypeSlider.onValueChange = [this, &filterTypeSlider]() {
        if (sliderUpdateInProgress)
            return;
        int target = juce::roundToInt(filterTypeSlider.getValue());
        int current = viewModel.getFilterType();
        int iterations = 0;
        while (current != target && iterations++ < 16) {
            if (target > current)
                viewModel.incrementFilterType();
            else
                viewModel.decrementFilterType();
            current = viewModel.getFilterType();
        }
    };
}
