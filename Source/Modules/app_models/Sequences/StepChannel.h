namespace app_models {

namespace IDs {

const juce::Identifier STEP_CHANNEL("STEP_CHANNEL");
const juce::Identifier stepChannelIndex("channelIndex");
const juce::Identifier stepPattern("stepPattern");

} // namespace IDs
class StepChannel {
  public:
    StepChannel(juce::ValueTree v);

    void setIndex(int index);
    void setPattern(juce::BigInteger &b);
    int getIndex();

    juce::BigInteger getPattern();
    /**
     * Set the note intensity between 0 & 7
     */
    void setNote(int noteIndex, int value);
    /**
     * Return the note intensity between 0 & 7
     */
    int getNote(int noteIndex);
    /**
     * Maximum of notes based on notes per measure
     * Initially we wanted 4 notes in 4 measures but with if we increase the
     * number of notes per measure we get only 1 measure. To have 4 measures
     * with 16 notes per measure we need 64.
     */
    static int getMaxNumberOfNotes(int notesPerMeasure);

    static const int maxNumberOfChannels;
    static const int maxNumberOfMeasures;
    static const int maxNumberOfVelocityBits;

    // State must be public for value tree object list
    juce::ValueTree state;

  private:
    tracktion::ConstrainedCachedValue<int> channelIndex;
    tracktion::ConstrainedCachedValue<juce::String> pattern;
};

} // namespace app_models
