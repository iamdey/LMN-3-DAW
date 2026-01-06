namespace app_models {

const int StepChannel::maxNumberOfChannels = 24;
const int StepChannel::maxNumberOfMeasures = 4;
/**
 * A maximum of bits for velocity dataset: 16 notes per measure × 4 measures × 3 = 192
 * bits (octal value for velocity).
 * 111 is 7 as maximum velocity
 */
const int StepChannel::maxNumberOfVelocityBits = 4 * 16 * 3;

StepChannel::StepChannel(juce::ValueTree v) : state(v) {
    jassert(state.hasType(IDs::STEP_CHANNEL));

    std::function<int(int)> channelIndexConstrainer = [this](int param) {
        return juce::jlimit(0, maxNumberOfChannels, param);
    };

    std::function<juce::String(juce::String)> patternConstrainer =
        [this](juce::String param) {
            if (param.length() > maxNumberOfVelocityBits)
                // keep only last bits: the first note index is the last 3 bits in the pattern
                // cf. `setBitRangeAsInt`
                return param.getLastCharacters(maxNumberOfVelocityBits);
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

/**
 * Set the note intensity between 0-7
 * noteIndex is constrained by pattern max length.
 * cf. maxNumberOfVelocityBits: 192
 */
void StepChannel::setNote(int noteIndex, int value) {
    jassert(value >= 0 && value < 8);

    juce::BigInteger b(getPattern());
    // octal requires 3 bits
    b.setBitRangeAsInt(noteIndex * 3, 3, value);
    setPattern(b);
}

/**
 * Get the note intensity between 0-7
 */
int StepChannel::getNote(int noteIndex) {
    return getPattern().getBitRangeAsInt(noteIndex * 3, 3);
}

int StepChannel::getMaxNumberOfNotes(int notesPerMeasure) {
    return maxNumberOfMeasures * notesPerMeasure;
}

} // namespace app_models
