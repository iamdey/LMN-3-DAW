#include "ArpeggiatorViewModel.h"
#include <internal_plugins/CustomFourOscPlugin/CustomFourOscPlugin.h>

namespace app_view_models {

ArpeggiatorViewModel::ArpeggiatorViewModel(
    internal_plugins::CustomFourOscPlugin *p)
    : plugin(p) {}

void ArpeggiatorViewModel::addListener(Listener *l) {
    listeners.add(l);
    l->parametersChanged();
}

void ArpeggiatorViewModel::removeListener(Listener *l) { listeners.remove(l); }

ArpeggiatorViewModel::Mode ArpeggiatorViewModel::getMode() const {
    if (plugin == nullptr)
        return Mode::off;

    auto raw = juce::jlimit(0, static_cast<int>(Mode::random),
                            plugin->getArpeggiatorMode());
    return static_cast<Mode>(raw);
}

void ArpeggiatorViewModel::incrementMode() {
    auto next = static_cast<int>(getMode()) + 1;
    if (next > static_cast<int>(Mode::random))
        next = static_cast<int>(Mode::off);
    setMode(static_cast<Mode>(next));
}

void ArpeggiatorViewModel::decrementMode() {
    auto next = static_cast<int>(getMode()) - 1;
    if (next < static_cast<int>(Mode::off))
        next = static_cast<int>(Mode::random);
    setMode(static_cast<Mode>(next));
}

void ArpeggiatorViewModel::setMode(Mode newMode) {
    auto clamped =
        juce::jlimit(static_cast<int>(Mode::off),
                     static_cast<int>(Mode::random),
                     static_cast<int>(newMode));
    if (plugin != nullptr)
        plugin->setArpeggiatorMode(clamped);
    notifyParametersChanged();
}

double ArpeggiatorViewModel::getRate() const {
    if (plugin == nullptr)
        return 4.0;

    return plugin->getArpeggiatorRate();
}

void ArpeggiatorViewModel::incrementRate() {
    auto newRate = juce::jlimit(1.0, 16.0, getRate() + 0.5);
    setRate(newRate);
}

void ArpeggiatorViewModel::decrementRate() {
    auto newRate = juce::jlimit(1.0, 16.0, getRate() - 0.5);
    setRate(newRate);
}

void ArpeggiatorViewModel::setRate(double newRate) {
    auto clamped = juce::jlimit(1.0, 16.0, newRate);
    if (juce::approximatelyEqual(clamped, getRate()))
        return;
    if (plugin != nullptr)
        plugin->setArpeggiatorRate(static_cast<float>(clamped));
    notifyParametersChanged();
}

int ArpeggiatorViewModel::getOctaves() const {
    if (plugin == nullptr)
        return 1;

    return plugin->getArpeggiatorOctaves();
}

void ArpeggiatorViewModel::incrementOctaves() {
    auto newValue = juce::jlimit(1, 4, getOctaves() + 1);
    setOctaves(newValue);
}

void ArpeggiatorViewModel::decrementOctaves() {
    auto newValue = juce::jlimit(1, 4, getOctaves() - 1);
    setOctaves(newValue);
}

void ArpeggiatorViewModel::setOctaves(int newOctaves) {
    auto clamped = juce::jlimit(1, 4, newOctaves);
    if (clamped == getOctaves())
        return;
    if (plugin != nullptr)
        plugin->setArpeggiatorOctaves(clamped);
    notifyParametersChanged();
}

double ArpeggiatorViewModel::getGate() const {
    if (plugin == nullptr)
        return 0.5;

    return plugin->getArpeggiatorGate();
}

void ArpeggiatorViewModel::incrementGate() {
    auto newGate = juce::jlimit(0.05, 1.0, getGate() + 0.05);
    setGate(newGate);
}

void ArpeggiatorViewModel::decrementGate() {
    auto newGate = juce::jlimit(0.05, 1.0, getGate() - 0.05);
    setGate(newGate);
}

void ArpeggiatorViewModel::setGate(double newGate) {
    auto clamped = juce::jlimit(0.05, 1.0, newGate);
    if (juce::approximatelyEqual(clamped, getGate()))
        return;
    if (plugin != nullptr)
        plugin->setArpeggiatorGate(static_cast<float>(clamped));
    notifyParametersChanged();
}

bool ArpeggiatorViewModel::isEnabled() const {
    return plugin != nullptr && plugin->isArpeggiatorEnabled();
}

void ArpeggiatorViewModel::toggleEnabled() {
    if (plugin == nullptr)
        return;

    plugin->setArpeggiatorEnabled(!isEnabled());
    notifyParametersChanged();
}

bool ArpeggiatorViewModel::isTempoSyncEnabled() const {
    return plugin != nullptr && plugin->isArpeggiatorTempoSyncEnabled();
}

void ArpeggiatorViewModel::toggleTempoSync() {
    if (plugin == nullptr)
        return;

    plugin->setArpeggiatorTempoSyncEnabled(!isTempoSyncEnabled());
    notifyParametersChanged();
}

juce::String ArpeggiatorViewModel::getModeName() const {
    switch (getMode()) {
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

void ArpeggiatorViewModel::notifyParametersChanged() {
    listeners.call([](Listener &l) { l.parametersChanged(); });
}

} // namespace app_view_models
