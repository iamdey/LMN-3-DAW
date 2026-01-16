#pragma once
namespace app_view_models {
class DistortionPluginViewModel
    : public app_view_models::InternalPluginViewModel {
  public:
    explicit DistortionPluginViewModel(internal_plugins::DistortionPlugin *p);

    int getNumberOfParameters() override;

    juce::String getParameterName(int index) override;
    double getParameterValue(int index) override;
    void setParameterValue(int index, double value) override;
    juce::Range<double> getParameterRange(int index) override;
    double getParameterInterval(int index) override;

  private:
    internal_plugins::DistortionPlugin *distortionPlugin;
};
} // namespace app_view_models
