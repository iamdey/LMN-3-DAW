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
    if (plugin != nullptr)
        plugin->setArpeggiatorMode(next);
    notifyParametersChanged();
}

void ArpeggiatorViewModel::decrementMode() {
    auto next = static_cast<int>(getMode()) - 1;
    if (next < static_cast<int>(Mode::off))
        next = static_cast<int>(Mode::random);
    if (plugin != nullptr)
        plugin->setArpeggiatorMode(next);
    notifyParametersChanged();
}

double ArpeggiatorViewModel::getRate() const {
    if (plugin == nullptr)
        return 4.0;

    return plugin->getArpeggiatorRate();
}

void ArpeggiatorViewModel::incrementRate() {
    auto newRate = juce::jlimit(1.0, 16.0, getRate() + 0.5);
    if (plugin != nullptr)
        plugin->setArpeggiatorRate(static_cast<float>(newRate));
    notifyParametersChanged();
}

void ArpeggiatorViewModel::decrementRate() {
    auto newRate = juce::jlimit(1.0, 16.0, getRate() - 0.5);
    if (plugin != nullptr)
        plugin->setArpeggiatorRate(static_cast<float>(newRate));
    notifyParametersChanged();
}

int ArpeggiatorViewModel::getOctaves() const {
    if (plugin == nullptr)
        return 1;

    return plugin->getArpeggiatorOctaves();
}

void ArpeggiatorViewModel::incrementOctaves() {
    auto newValue = juce::jlimit(1, 4, getOctaves() + 1);
    if (plugin != nullptr)
        plugin->setArpeggiatorOctaves(newValue);
    notifyParametersChanged();
}

void ArpeggiatorViewModel::decrementOctaves() {
    auto newValue = juce::jlimit(1, 4, getOctaves() - 1);
    if (plugin != nullptr)
        plugin->setArpeggiatorOctaves(newValue);
    notifyParametersChanged();
}

double ArpeggiatorViewModel::getGate() const {
    if (plugin == nullptr)
        return 0.5;

    return plugin->getArpeggiatorGate();
}

void ArpeggiatorViewModel::incrementGate() {
    auto newGate = juce::jlimit(0.05, 1.0, getGate() + 0.05);
    if (plugin != nullptr)
        plugin->setArpeggiatorGate(static_cast<float>(newGate));
    notifyParametersChanged();
}

void ArpeggiatorViewModel::decrementGate() {
    auto newGate = juce::jlimit(0.05, 1.0, getGate() - 0.05);
    if (plugin != nullptr)
        plugin->setArpeggiatorGate(static_cast<float>(newGate));
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
