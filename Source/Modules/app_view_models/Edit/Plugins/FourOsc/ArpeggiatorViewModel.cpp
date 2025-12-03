#include "ArpeggiatorViewModel.h"
namespace app_view_models {

ArpeggiatorViewModel::ArpeggiatorViewModel(tracktion::FourOscPlugin *p)
    : plugin(p) {}

void ArpeggiatorViewModel::addListener(Listener *l) {
    listeners.add(l);
    l->parametersChanged();
}

void ArpeggiatorViewModel::removeListener(Listener *l) { listeners.remove(l); }

ArpeggiatorViewModel::Mode ArpeggiatorViewModel::getMode() const {
    return mode;
}

void ArpeggiatorViewModel::incrementMode() {
    auto next = static_cast<int>(mode) + 1;
    if (next > static_cast<int>(Mode::random))
        next = static_cast<int>(Mode::off);
    mode = static_cast<Mode>(next);
    notifyParametersChanged();
}

void ArpeggiatorViewModel::decrementMode() {
    auto next = static_cast<int>(mode) - 1;
    if (next < static_cast<int>(Mode::off))
        next = static_cast<int>(Mode::random);
    mode = static_cast<Mode>(next);
    notifyParametersChanged();
}

double ArpeggiatorViewModel::getRate() const { return rateHz; }

void ArpeggiatorViewModel::incrementRate() {
    rateHz = std::min(16.0, rateHz + 0.5);
    notifyParametersChanged();
}

void ArpeggiatorViewModel::decrementRate() {
    rateHz = std::max(1.0, rateHz - 0.5);
    notifyParametersChanged();
}

int ArpeggiatorViewModel::getOctaves() const { return octaves; }

void ArpeggiatorViewModel::incrementOctaves() {
    octaves = std::min(4, octaves + 1);
    notifyParametersChanged();
}

void ArpeggiatorViewModel::decrementOctaves() {
    octaves = std::max(1, octaves - 1);
    notifyParametersChanged();
}

double ArpeggiatorViewModel::getGate() const { return gate; }

void ArpeggiatorViewModel::incrementGate() {
    gate = std::min(1.0, gate + 0.05);
    notifyParametersChanged();
}

void ArpeggiatorViewModel::decrementGate() {
    gate = std::max(0.05, gate - 0.05);
    notifyParametersChanged();
}

bool ArpeggiatorViewModel::isEnabled() const { return enabled; }

void ArpeggiatorViewModel::toggleEnabled() {
    enabled = !enabled;
    notifyParametersChanged();
}

const juce::Array<int> &ArpeggiatorViewModel::getCurrentNotes() const {
    return noteBuffer;
}

void ArpeggiatorViewModel::addNote(int noteNumber) {
    if (!noteBuffer.contains(noteNumber)) {
        noteBuffer.add(noteNumber);
        noteBuffer.sort();
    }
    currentNoteIndex = 0;
    direction = 1;
    notifyNotesChanged();
}

void ArpeggiatorViewModel::removeNote(int noteNumber) {
    if (noteBuffer.contains(noteNumber)) {
        noteBuffer.removeAllInstancesOf(noteNumber);
        currentNoteIndex = 0;
        direction = 1;
        notifyNotesChanged();
    }
}

int ArpeggiatorViewModel::getIntervalMs() const {
    return static_cast<int>(1000.0 / rateHz);
}

juce::String ArpeggiatorViewModel::getModeName() const {
    switch (mode) {
    case Mode::off:
        return "Off";
    case Mode::up:
        return "Up";
    case Mode::down:
        return "Down";
    case Mode::upDown:
        return "Up-Down";
    case Mode::random:
        return "Random";
    default:
        return "Off";
    }
}

int ArpeggiatorViewModel::getNextNote() {
    if (noteBuffer.isEmpty())
        return -1;

    int baseNote = 0;
    switch (mode) {
    case Mode::off:
        return -1;
    case Mode::up:
        baseNote = noteBuffer[(currentNoteIndex++) % noteBuffer.size()];
        break;
    case Mode::down:
        baseNote = noteBuffer[(currentNoteIndex--) % noteBuffer.size()];
        if (currentNoteIndex < 0)
            currentNoteIndex = noteBuffer.size() - 1;
        break;
    case Mode::upDown: {
        baseNote = noteBuffer[currentNoteIndex];
        currentNoteIndex += direction;
        if (currentNoteIndex >= noteBuffer.size()) {
            direction = -1;
            currentNoteIndex = noteBuffer.size() - 2;
        } else if (currentNoteIndex < 0) {
            direction = 1;
            currentNoteIndex = 1;
        }
        break;
    }
    case Mode::random:
        baseNote = noteBuffer[random.nextInt(noteBuffer.size())];
        break;
    }

    int octaveOffset = 12 * (random.nextInt(juce::jmax(1, octaves)) + 0);
    return baseNote + octaveOffset;
}

void ArpeggiatorViewModel::notifyParametersChanged() {
    clampSettings();
    listeners.call([](Listener &l) { l.parametersChanged(); });
}

void ArpeggiatorViewModel::notifyNotesChanged() {
    listeners.call([](Listener &l) { l.notesChanged(); });
}

void ArpeggiatorViewModel::clampSettings() {
    rateHz = juce::jlimit(1.0, 16.0, rateHz);
    octaves = juce::jlimit(1, 4, octaves);
    gate = juce::jlimit(0.05, 1.0, gate);
}

} // namespace app_view_models
