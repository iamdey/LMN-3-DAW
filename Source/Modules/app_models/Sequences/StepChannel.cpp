namespace app_models {

const int StepChannel::maxNumberOfChannels = 24;
const int StepChannel::maxNumberOfMeasures = 4;

StepChannel::StepChannel(juce::ValueTree v) : state(v) {
    jassert(state.hasType(IDs::STEP_CHANNEL));

    std::function<int(int)> channelIndexConstrainer = [this](int param) {
        return juce::jlimit(0, maxNumberOfChannels, param);
    };

    std::function<juce::String(juce::String)> patternConstrainer =
        [this](juce::String param) {
            // Idk how to compute maxNumberOfNotes dynamically in the
            // constructor or in the pattern constrainer (and may be it's not
            // important). Set a maximal initial value: 16 notes per measure × 4
            // measures
            const int maxNumberOfNotes = 64;
            if (param.length() > maxNumberOfNotes)
                return param.dropLastCharacters(param.length() -
                                                maxNumberOfNotes);
            else
                return param;
        };

    channelIndex.setConstrainer(channelIndexConstrainer);
    channelIndex.referTo(state, IDs::stepChannelIndex, nullptr, -1);

    pattern.setConstrainer(patternConstrainer);
    pattern.referTo(state, IDs::stepPattern, nullptr, "0000000000000000");
}

void StepChannel::setIndex(int index) { channelIndex.setValue(index, nullptr); }

void StepChannel::setPattern(juce::BigInteger &b) {
    pattern.setValue(b.toString(2), nullptr);
}

int StepChannel::getIndex() { return channelIndex.get(); }

juce::BigInteger StepChannel::getPattern() {
    juce::BigInteger b;
    b.parseString(pattern.get(), 2);
    return b;
}

void StepChannel::setNote(int noteIndex, bool value) {
    juce::BigInteger b(getPattern());
    b.setBit(noteIndex, value);
    setPattern(b);
}

bool StepChannel::getNote(int noteIndex) { return getPattern()[noteIndex]; }

int StepChannel::getMaxNumberOfNotes(int notesPerMeasure) {
    return maxNumberOfMeasures * notesPerMeasure;
}

} // namespace app_models
