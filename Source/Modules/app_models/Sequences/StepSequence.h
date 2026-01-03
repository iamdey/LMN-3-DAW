
namespace app_models {

namespace IDs {

const juce::Identifier STEP_SEQUENCE("STEP_SEQUENCE");

}
class StepSequence {
  public:
    explicit StepSequence(juce::ValueTree v);

    StepChannel *getChannel(int index);
    /**
     * remove and init patterns
     */
    void clear();

  private:
    /**
     * init patterns
     */
    void init();
    juce::ValueTree state;
    StepChannelList channelList;
};

} // namespace app_models
