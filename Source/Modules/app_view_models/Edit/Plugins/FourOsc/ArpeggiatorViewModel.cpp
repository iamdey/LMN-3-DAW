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
    return enabled;
}

ArpeggiatorViewModel::Mode ArpeggiatorViewModel::getMode() const {
    return mode;
}

float ArpeggiatorViewModel::getRate() const {
    return rate;
}

int ArpeggiatorViewModel::getOctaves() const {
    return octaves;
}

float ArpeggiatorViewModel::getGate() const {
    return gate;
}

void ArpeggiatorViewModel::toggleEnabled() {
    enabled = !enabled;
    if (!enabled) {
        clearNoteBuffer();
    }
    markAndUpdate(shouldUpdateParameters);
}

void ArpeggiatorViewModel::incrementMode() {
    int modeValue = static_cast<int>(mode);
    if (modeValue < static_cast<int>(Mode::Random)) {
        mode = static_cast<Mode>(modeValue + 1);
        markAndUpdate(shouldUpdateParameters);
    }
}

void ArpeggiatorViewModel::decrementMode() {
    int modeValue = static_cast<int>(mode);
    if (modeValue > static_cast<int>(Mode::Off)) {
        mode = static_cast<Mode>(modeValue - 1);
        markAndUpdate(shouldUpdateParameters);
    }
}

void ArpeggiatorViewModel::incrementRate() {
    rate = juce::jmin(rate + 0.5f, 16.0f);
    markAndUpdate(shouldUpdateParameters);
}

void ArpeggiatorViewModel::decrementRate() {
    rate = juce::jmax(rate - 0.5f, 1.0f);
    markAndUpdate(shouldUpdateParameters);
}

void ArpeggiatorViewModel::incrementOctaves() {
    if (octaves < 4) {
        octaves++;
        markAndUpdate(shouldUpdateParameters);
    }
}

void ArpeggiatorViewModel::decrementOctaves() {
    if (octaves > 1) {
        octaves--;
        markAndUpdate(shouldUpdateParameters);
    }
}

void ArpeggiatorViewModel::incrementGate() {
    gate = juce::jmin(gate + 0.05f, 1.0f);
    markAndUpdate(shouldUpdateParameters);
}

void ArpeggiatorViewModel::decrementGate() {
    gate = juce::jmax(gate - 0.05f, 0.05f);
    markAndUpdate(shouldUpdateParameters);
}

void ArpeggiatorViewModel::addNoteToBuffer(int noteNumber) {
    if (!noteBuffer.contains(noteNumber)) {
        noteBuffer.add(noteNumber);
        noteBuffer.sort();
    }
}

void ArpeggiatorViewModel::removeNoteFromBuffer(int noteNumber) {
    noteBuffer.removeAllInstancesOf(noteNumber);
}

void ArpeggiatorViewModel::clearNoteBuffer() {
    noteBuffer.clear();
}

juce::Array<int> ArpeggiatorViewModel::getCurrentNotes() const {
    return noteBuffer;
}

void ArpeggiatorViewModel::handleAsyncUpdate() {
    if (compareAndReset(shouldUpdateParameters))
        listeners.call([this](Listener &l) { l.parametersChanged(); });
}

void ArpeggiatorViewModel::valueTreePropertyChanged(
    juce::ValueTree &treeWhosePropertyHasChanged,
    const juce::Identifier &property) {
    // Listen for changes if needed in the future
    if (treeWhosePropertyHasChanged == plugin->state) {
        // Handle plugin state changes if needed
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
