#include "ArpeggiatorViewModel.h"

namespace app_view_models {

ArpeggiatorViewModel::ArpeggiatorViewModel(tracktion::FourOscPlugin *p)
    : plugin(p) {
    plugin->state.addListener(this);
}

ArpeggiatorViewModel::~ArpeggiatorViewModel() {
    plugin->state.removeListener(this);
}

bool ArpeggiatorViewModel::isEnabled() const {
    return plugin->arpParams->enabledValue.get();
}

ArpeggiatorViewModel::Mode ArpeggiatorViewModel::getMode() const {
    return static_cast<Mode>(plugin->arpParams->modeValue.get());
}

float ArpeggiatorViewModel::getRate() const {
    return plugin->arpParams->rateValue.get();
}

int ArpeggiatorViewModel::getOctaves() const {
    return plugin->arpParams->octavesValue.get();
}

float ArpeggiatorViewModel::getGate() const {
    return plugin->arpParams->gateValue.get();
}

void ArpeggiatorViewModel::toggleEnabled() {
    bool currentEnabled = plugin->arpParams->enabledValue.get();
    plugin->arpParams->enabledValue.setValue(!currentEnabled, nullptr);
    if (!plugin->arpParams->enabledValue.get()) {
        clearNoteBuffer();
    }
    markAndUpdate(shouldUpdateParameters);
}

void ArpeggiatorViewModel::incrementMode() {
    int currentMode = plugin->arpParams->modeValue.get();
    if (currentMode < static_cast<int>(Mode::Random)) {
        plugin->arpParams->modeValue.setValue(currentMode + 1, nullptr);
        markAndUpdate(shouldUpdateParameters);
    }
}

void ArpeggiatorViewModel::decrementMode() {
    int currentMode = plugin->arpParams->modeValue.get();
    if (currentMode > static_cast<int>(Mode::Off)) {
        plugin->arpParams->modeValue.setValue(currentMode - 1, nullptr);
        markAndUpdate(shouldUpdateParameters);
    }
}

void ArpeggiatorViewModel::incrementRate() {
    float currentRate = plugin->arpParams->rateValue.get();
    plugin->arpParams->rateValue.setValue(juce::jmin(currentRate + 0.5f, 16.0f), nullptr);
    markAndUpdate(shouldUpdateParameters);
}

void ArpeggiatorViewModel::decrementRate() {
    float currentRate = plugin->arpParams->rateValue.get();
    plugin->arpParams->rateValue.setValue(juce::jmax(currentRate - 0.5f, 1.0f), nullptr);
    markAndUpdate(shouldUpdateParameters);
}

void ArpeggiatorViewModel::incrementOctaves() {
    int currentOctaves = plugin->arpParams->octavesValue.get();
    if (currentOctaves < 4) {
        plugin->arpParams->octavesValue.setValue(currentOctaves + 1, nullptr);
        markAndUpdate(shouldUpdateParameters);
    }
}

void ArpeggiatorViewModel::decrementOctaves() {
    int currentOctaves = plugin->arpParams->octavesValue.get();
    if (currentOctaves > 1) {
        plugin->arpParams->octavesValue.setValue(currentOctaves - 1, nullptr);
        markAndUpdate(shouldUpdateParameters);
    }
}

void ArpeggiatorViewModel::incrementGate() {
    float currentGate = plugin->arpParams->gateValue.get();
    plugin->arpParams->gateValue.setValue(juce::jmin(currentGate + 0.05f, 1.0f), nullptr);
    markAndUpdate(shouldUpdateParameters);
}

void ArpeggiatorViewModel::decrementGate() {
    float currentGate = plugin->arpParams->gateValue.get();
    plugin->arpParams->gateValue.setValue(juce::jmax(currentGate - 0.05f, 0.05f), nullptr);
    markAndUpdate(shouldUpdateParameters);
}

void ArpeggiatorViewModel::addNoteToBuffer(int noteNumber) {
    plugin->addNoteToArpeggiator(noteNumber);
}

void ArpeggiatorViewModel::removeNoteFromBuffer(int noteNumber) {
    plugin->removeNoteFromArpeggiator(noteNumber);
}

void ArpeggiatorViewModel::clearNoteBuffer() {
    plugin->clearArpeggiatorNotes();
}

juce::Array<int> ArpeggiatorViewModel::getCurrentNotes() const {
    return plugin->getArpeggiatorNotes();
}

void ArpeggiatorViewModel::handleAsyncUpdate() {
    if (compareAndReset(shouldUpdateParameters))
        listeners.call([this](Listener &l) { l.parametersChanged(); });
}

void ArpeggiatorViewModel::valueTreePropertyChanged(
    juce::ValueTree &treeWhosePropertyHasChanged,
    const juce::Identifier &property) {
    if (treeWhosePropertyHasChanged == plugin->state) {
        // Check if any arpeggiator property changed
        if (property.toString().contains("arp")) {
            markAndUpdate(shouldUpdateParameters);
        }
    }
}

void ArpeggiatorViewModel::addListener(Listener *l) {
    listeners.add(l);
    l->parametersChanged();
}

void ArpeggiatorViewModel::removeListener(Listener *l) {
    listeners.remove(l);
}

} // namespace app_view_models
