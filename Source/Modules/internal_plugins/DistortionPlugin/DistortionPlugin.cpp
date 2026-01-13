namespace internal_plugins {

DistortionPlugin::DistortionPlugin(tracktion::PluginCreationInfo info)
    : tracktion::Plugin(info) {
    auto um = getUndoManager();

    gain.referTo(state, IDs::gain, um, 0.5f);

    gainParam = addParam("gain", "Gain", {0.1f, 20.0f});
    gainParam->attachToCurrentValue(gain);
}

DistortionPlugin::~DistortionPlugin() {
    notifyListenersOfDeletion();
    gainParam->detachFromCurrentValue();
}

const char *DistortionPlugin::xmlTypeName = "distortion";

void DistortionPlugin::initialise(
    const tracktion::PluginInitialisationInfo &info) {}

void DistortionPlugin::deinitialise() {}

void DistortionPlugin::applyToBuffer(const tracktion::PluginRenderContext &fc) {
    if (fc.destBuffer == nullptr)
        return;

    for (int channel = 0; channel < fc.destBuffer->getNumChannels();
         ++channel) {
        auto dest = fc.destBuffer->getWritePointer(channel);

        for (int i = 0; i < fc.bufferNumSamples; ++i)
            dest[i] = std::tanh(gain * dest[i]);
    }
}

} // namespace internal_plugins