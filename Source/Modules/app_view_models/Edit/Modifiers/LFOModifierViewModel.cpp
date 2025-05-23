namespace app_view_models {

LFOModifierViewModel::LFOModifierViewModel(tracktion::LFOModifier *mod)
    : ModifierViewModel(mod), lfoModifier(mod) {}

int LFOModifierViewModel::getNumberOfParameters() { return 5; }

juce::String LFOModifierViewModel::getParameterName(int index) {
    switch (index) {
    case 0:
        return "Wave Type";
    case 1:
        return "Rate";
    case 2:
        return "Depth";
    case 3:
        return "Phase";
    case 4:
        return "Offset";
    default:
        return "Parameter " + juce::String(index);
    }
}

double LFOModifierViewModel::getParameterValue(int index) {
    switch (index) {
    case 0:
        return lfoModifier->wave.get();
    case 1:
        return lfoModifier->rate.get();
    case 2:
        return lfoModifier->depth.get();
    case 3:
        return lfoModifier->phase.get();
    case 4:
        return lfoModifier->offset.get();
    default:
        return lfoModifier->depth.get();
    }
}

void LFOModifierViewModel::setParameterValue(int index, double value) {
    switch (index) {
    case 0:
        lfoModifier->waveParam->setParameter(float(value),
                                             juce::dontSendNotification);
        break;
    case 1:
        lfoModifier->rateParam->setParameter(float(value),
                                             juce::dontSendNotification);
        break;
    case 2:
        lfoModifier->depthParam->setParameter(float(value),
                                              juce::dontSendNotification);
        break;
    case 3:
        lfoModifier->phaseParam->setParameter(float(value),
                                              juce::dontSendNotification);
        break;
    case 4:
        lfoModifier->offsetParam->setParameter(float(value),
                                               juce::dontSendNotification);
        break;
    default:
        break;
    }
}

juce::Range<double> LFOModifierViewModel::getParameterRange(int index) {
    switch (index) {
    case 0:
        return juce::Range<double>(
            1, tracktion::LFOModifier::getWaveNames().size() - 1);
    case 1:
        return juce::Range<double>(0.01, 50);
    case 2:
        return juce::Range<double>(0, 1);
    case 3:
        return juce::Range<double>(0, 1);
    case 4:
        return juce::Range<double>(0, 1);
    default:
        return juce::Range<double>(0, 1);
    }
}

double LFOModifierViewModel::getParameterInterval(int index) {
    switch (index) {
    case 0:
        return 1;
    case 1:
        return .5;
    case 2:
        return .01;
    case 3:
        return .01;
    case 4:
        return .01;
    default:
        return .01;
    }
}

} // namespace app_view_models
