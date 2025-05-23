namespace app_view_models {
// For playing clip only!!
// https://forum.juce.com/t/createeditforpreviewingclip-how-is-it-used/32757
StepSequencerViewModel::StepSequencerViewModel(tracktion::AudioTrack::Ptr t)
    : track(t), state(track->state.getOrCreateChildWithName(
                    IDs::STEP_SEQUENCER_STATE, nullptr)),
      editState(track->edit.state.getChildWithName(IDs::EDIT_VIEW_STATE)),
      stepSequence(state.getOrCreateChildWithName(
          app_models::IDs::STEP_SEQUENCE, nullptr)) {
    jassert(state.hasType(IDs::STEP_SEQUENCER_STATE));
    jassert(editState.hasType(IDs::EDIT_VIEW_STATE));

    state.addListener(this);
    // So we can listen for octave changes
    // Note that editState must be a member of this class for the listener to
    // work
    // https://forum.juce.com/t/adding-listener-to-valuetree-child-does-not-work/25229
    // An alternative would be to listen to the midi command manager octave
    // change, but I think I like letting the editTabBarView solely do the
    // listening for that, and listen to the state directly here
    editState.addListener(this);

    std::function<int(int)> numberOfNotesConstrainer = [this](int param) {
        // numberOfNotes cannot be less than 1
        // it also cannot be greater than the maximum number of notes allowed
        if (param <= 1)
            return 1;
        else if (param >= app_models::StepChannel::getMaxNumberOfNotes(
                              notesPerMeasure.get()))
            return app_models::StepChannel::getMaxNumberOfNotes(
                notesPerMeasure.get());
        else
            return param;
    };

    numberOfNotes.setConstrainer(numberOfNotesConstrainer);
    numberOfNotes.referTo(
        state, IDs::numberOfNotes, nullptr,
        app_models::StepChannel::getMaxNumberOfNotes(notesPerMeasure.get()));

    std::function<int(int)> selectedNoteIndexConstrainer = [this](int param) {
        // selected index cannot be less than 0
        // it also cannot be greater than or equal to the number of notes
        if (param <= 0)
            return 0;
        else if (param >= numberOfNotes.get())
            return numberOfNotes.get() - 1;
        else
            return param;
    };

    selectedNoteIndex.setConstrainer(selectedNoteIndexConstrainer);
    selectedNoteIndex.referTo(state, IDs::selectedNoteIndex, nullptr, 0);

    notesPerMeasure.referTo(state, IDs::notesPerMeasure, nullptr, 4);

    double secondsPerBeat = 1.0 / track->edit.tempoSequence.getBeatsPerSecondAt(
                                      tracktion::TimePosition::fromSeconds(0));

    // Midi clip
    midiClipStart = track->edit.getTransport().getPosition();
    midiClipEnd = tracktion::TimePosition::fromSeconds(
        midiClipStart.inSeconds() +
        (numberOfNotes.get() * (4.0 / double(notesPerMeasure.get())) *
         secondsPerBeat));
    const tracktion::TimeRange midiClipTimeRange =
        tracktion::TimeRange(midiClipStart, midiClipEnd);
    midiClip = dynamic_cast<tracktion::MidiClip *>(track->insertNewClip(
        tracktion::TrackItem::Type::midi, "step", midiClipTimeRange, nullptr));

    generateMidiSequence();

    loopAroundClip(*midiClip);

    track->edit.getTransport().addListener(this);
}

StepSequencerViewModel::~StepSequencerViewModel() {
    stop();

    generateMidiSequence();

    // if the sequence is empty, delete the clip
    // and disable looping
    if (midiClip->getSequence().isEmpty()) {
        midiClip->removeFromParent();
        track->edit.getTransport().looping.setValue(false, nullptr);
    }

    state.removeListener(this);
    editState.removeListener(this);
    track->edit.state.getChildWithName(IDs::EDIT_VIEW_STATE)
        .removeListener(this);
    track->edit.getTransport().removeListener(this);
}

int StepSequencerViewModel::getNumChannels() {
    return app_models::StepChannel::maxNumberOfChannels;
}

int StepSequencerViewModel::getNumNotesPerChannel() {
    return app_models::StepChannel::getMaxNumberOfNotes(notesPerMeasure.get());
}

bool StepSequencerViewModel::hasNoteAt(int channel, int noteIndex) {
    return stepSequence.getChannel(channel)->getNote(noteIndex);
}

void StepSequencerViewModel::toggleNoteNumberAtSelectedIndex(int noteNumber) {
    // FIXME: it crashes if octave is higher or lower than normal
    int channel = noteNumberToChannel(noteNumber);
    bool active = hasNoteAt(channel, selectedNoteIndex.get());
    stepSequence.getChannel(channel)->setNote(selectedNoteIndex.get(), !active);

    if (!track->edit.getTransport().isPlaying()) {
        incrementSelectedNoteIndex();
    } else {
        // When looping also add or remove note from the midi sequence to hear
        // it
        if (!active) {
            addNoteToSequence(channel, selectedNoteIndex.get());
        } else {
            removeNoteFromSequence(channel, selectedNoteIndex.get());
        }
    }
}

int StepSequencerViewModel::noteNumberToChannel(int noteNumber) {
    // Note numbers range from 5 to 124
    // Subtract the min note number from the note so that its zero based
    int zeroBasedNoteNumber = noteNumber - MIN_NOTE_NUMBER;
    return zeroBasedNoteNumber - (NOTES_PER_OCTAVE * getZeroBasedOctave());
}

int StepSequencerViewModel::getSelectedNoteIndex() {
    return selectedNoteIndex.get();
}

int StepSequencerViewModel::getNumberOfNotes() { return numberOfNotes.get(); }

void StepSequencerViewModel::incrementSelectedNoteIndex() {
    if (!track->edit.getTransport().isPlaying())
        selectedNoteIndex.setValue(selectedNoteIndex.get() + 1, nullptr);
}

void StepSequencerViewModel::decrementSelectedNoteIndex() {
    if (!track->edit.getTransport().isPlaying())
        selectedNoteIndex.setValue(selectedNoteIndex.get() - 1, nullptr);
}

void StepSequencerViewModel::incrementNumberOfNotes() {
    if (!track->edit.getTransport().isPlaying())
        numberOfNotes.setValue(numberOfNotes.get() + 1, nullptr);
}

void StepSequencerViewModel::decrementNumberOfNotes() {
    if (!track->edit.getTransport().isPlaying())
        numberOfNotes.setValue(numberOfNotes.get() - 1, nullptr);
}

int StepSequencerViewModel::getNotesPerMeasure() {
    return notesPerMeasure.get();
}

void StepSequencerViewModel::incrementNotesPerMeasure() {
    if (!track->edit.getTransport().isPlaying()) {
        int currentIndex =
            notesPerMeasureOptions.indexOf(notesPerMeasure.get());

        if (currentIndex != -1) {
            int newIndex;
            if (currentIndex == notesPerMeasureOptions.size() - 1)
                newIndex = 0;
            else
                newIndex = currentIndex + 1;

            notesPerMeasure.setValue(notesPerMeasureOptions[newIndex], nullptr);
            // compute the new number of notes for the measure division
            float prevNbMeasures =
                numberOfNotes.get() / notesPerMeasureOptions[currentIndex];
            int newNbNotes =
                (int)std::round(notesPerMeasure.get() * prevNbMeasures);
            numberOfNotes.setValue(newNbNotes, nullptr);
        }
    }
}

void StepSequencerViewModel::decrementNotesPerMeasure() {
    if (!track->edit.getTransport().isPlaying()) {
        int currentIndex =
            notesPerMeasureOptions.indexOf(notesPerMeasure.get());

        if (currentIndex != -1) {
            int newIndex;
            if (currentIndex == 0)
                newIndex = notesPerMeasureOptions.size() - 1;
            else
                newIndex = currentIndex - 1;

            notesPerMeasure.setValue(notesPerMeasureOptions[newIndex], nullptr);
            // compute the new number of notes for the measure division
            float prevNbMeasures =
                numberOfNotes.get() / notesPerMeasureOptions[currentIndex];
            int newNbNotes =
                (int)std::round(notesPerMeasure.get() * prevNbMeasures);
            numberOfNotes.setValue(newNbNotes, nullptr);
        }
    }
}

void StepSequencerViewModel::clearNotesAtSelectedIndex() {
    for (int i = 0; i < app_models::StepChannel::maxNumberOfChannels; i++) {
        stepSequence.getChannel(i)->setNote(selectedNoteIndex, false);
        removeNoteFromSequence(i, selectedNoteIndex);
    }
}

void StepSequencerViewModel::play() {
    // In the future maybe only the midi clip currently generated by the
    // sequencer should be played not the whole edit. Might be challenging
    // though. https://forum.juce.com/t/how-to-render-a-midiclip-any-hints/31004
    // https://forum.juce.com/t/createeditforpreviewingclip-how-is-it-used/32757/11
    if (!track->edit.getTransport().isPlaying()) {
        generateMidiSequence();
        track->edit.clickTrackEnabled.setValue(false, nullptr);
        track->setSolo(true);
        track->edit.getTransport().play(false);
    }
}

void StepSequencerViewModel::stop() {
    auto &transport = track->edit.getTransport();
    if (transport.isPlaying()) {
        track->edit.clickTrackEnabled.setValue(true, nullptr);
        track->setSolo(false);
        transport.stop(false, false);
    } else {
        // if we try to stop while currently not playing
        // return transport to beginning
        transport.setPosition(
            tracktion::TimePosition::fromSeconds(midiClipStart.inSeconds()));
    }
}

void StepSequencerViewModel::handleAsyncUpdate() {
    if (compareAndReset(shouldUpdatePattern)) {
        listeners.call([this](Listener &l) { l.patternChanged(); });
    }

    if (compareAndReset(shouldUpdateSelectedNoteIndex))
        listeners.call([this](Listener &l) {
            l.selectedNoteIndexChanged(selectedNoteIndex.get());
        });

    if (compareAndReset(shouldUpdateNumberOfNotes)) {
        listeners.call([this](Listener &l) {
            l.numberOfNotesChanged(numberOfNotes.get());
        });

        if (selectedNoteIndex.get() >= numberOfNotes) {
            selectedNoteIndex.setValue(numberOfNotes.get() - 1, nullptr);
        }
    }

    if (compareAndReset(shouldUpdateNotesPerMeasure))
        listeners.call([this](Listener &l) {
            l.notesPerMeasureChanged(notesPerMeasure.get());
        });
}

void StepSequencerViewModel::valueTreePropertyChanged(
    juce::ValueTree &treeWhosePropertyHasChanged,
    const juce::Identifier &property) {
    if (treeWhosePropertyHasChanged.hasType(app_models::IDs::STEP_CHANNEL))
        if (property == app_models::IDs::stepPattern)
            markAndUpdate(shouldUpdatePattern);

    if (treeWhosePropertyHasChanged == state) {
        if (property == IDs::selectedNoteIndex)
            markAndUpdate(shouldUpdateSelectedNoteIndex);

        if (property == IDs::numberOfNotes) {
            double secondsPerBeat =
                1.0 / track->edit.tempoSequence.getBeatsPerSecondAt(
                          tracktion::TimePosition::fromSeconds(0.0));
            midiClipEnd = tracktion::TimePosition::fromSeconds(
                midiClipStart.inSeconds() +
                (numberOfNotes.get() * (4.0 / double(notesPerMeasure.get())) *
                 secondsPerBeat));
            midiClip->setEnd(midiClipEnd, true);
            loopAroundClip(*midiClip);

            markAndUpdate(shouldUpdateNumberOfNotes);
        }

        if (property == IDs::notesPerMeasure) {
            double secondsPerBeat =
                1.0 / track->edit.tempoSequence.getBeatsPerSecondAt(
                          tracktion::TimePosition::fromSeconds(0.0));
            midiClipEnd = tracktion::TimePosition::fromSeconds(
                midiClipStart.inSeconds() +
                (numberOfNotes.get() * (4.0 / double(notesPerMeasure.get())) *
                 secondsPerBeat));
            midiClip->setEnd(midiClipEnd, true);
            loopAroundClip(*midiClip);

            markAndUpdate(shouldUpdateNotesPerMeasure);
        }
    }

    if (treeWhosePropertyHasChanged.hasType(IDs::EDIT_VIEW_STATE)) {
        if (property == IDs::currentOctave) {
            // Might need to detect changes in here at some point, for now do
            // nothing
        }
    }
}

void StepSequencerViewModel::addListener(Listener *l) {
    listeners.add(l);
    l->patternChanged();
    l->selectedNoteIndexChanged(selectedNoteIndex.get());
    l->numberOfNotesChanged(numberOfNotes.get());
    l->notesPerMeasureChanged(notesPerMeasure.get());
}

void StepSequencerViewModel::removeListener(Listener *l) {
    listeners.remove(l);
}

void StepSequencerViewModel::generateMidiSequence() {
    auto &sequence = midiClip->getSequence();
    sequence.clear(nullptr);

    for (int i = 0; i < getNumChannels(); i++)
        for (int j = 0; j < getNumNotesPerChannel(); j++)
            if (hasNoteAt(i, j)) {
                addNoteToSequence(i, j);
            }
}

void StepSequencerViewModel::addNoteToSequence(int channel, int noteIndex) {
    // Need to get the pitch based on the sequence position and
    // current octave remember that we need to add the min note
    // number to get things correct since the min note number
    // possible is not 0, its 5
    int pitch =
        channel + (NOTES_PER_OCTAVE * getZeroBasedOctave()) + MIN_NOTE_NUMBER;
    auto startBeat = tracktion::BeatPosition::fromBeats(
        double(noteIndex * 4.0) / double(notesPerMeasure.get()));
    auto duration =
        tracktion::BeatDuration::fromBeats(4.0 / double(notesPerMeasure.get()));

    auto &sequence = midiClip->getSequence();
    sequence.addNote(pitch, startBeat, duration, 127, 1, nullptr);
}

void StepSequencerViewModel::removeNoteFromSequence(int channel,
                                                    int noteIndex) {
    auto note = findNoteInSequence(channel, noteIndex);
    if (note.has_value()) {
        auto &sequence = midiClip->getSequence();
        sequence.removeNote(*note.value(), nullptr);
    }
}

std::optional<tracktion::MidiNote *>
StepSequencerViewModel::findNoteInSequence(int channel, int noteIndex) {
    auto &sequence = midiClip->getSequence();

    // compute the note we want to remove
    int pitch =
        channel + (NOTES_PER_OCTAVE * getZeroBasedOctave()) + MIN_NOTE_NUMBER;
    auto startBeat = tracktion::BeatPosition::fromBeats(
        double(noteIndex * 4.0) / double(notesPerMeasure.get()));
    auto duration =
        tracktion::BeatDuration::fromBeats(4.0 / double(notesPerMeasure.get()));

    // find the first node with matching pitch & startBeat
    for (auto note : sequence.getNotes()) {
        if (note->getStartBeat().inBeats() == startBeat.inBeats() &&
            note->getNoteNumber() == pitch) {
            return note;
        }
    }

    return std::nullopt;
}

void StepSequencerViewModel::setVideoPosition(
    tracktion::TimePosition timePosition, bool forceJump) {
    // find beat of current time relative to the start of the midi clip
    // round it down to nearest whole beat
    // then account for the notes per measure
    auto time = timePosition.inSeconds();
    auto beats = track->edit.tempoSequence.toBeats(
        tracktion::TimePosition::fromSeconds(time - midiClipStart.inSeconds()));
    double beatTime =
        floorToFraction(beats.inBeats(), double(notesPerMeasure.get()) / 4.0);
    int note = (beatTime * notesPerMeasure.get()) / 4.0;
    selectedNoteIndex.setValue(note, nullptr);
}

double StepSequencerViewModel::floorToFraction(double number,
                                               double denominator) {
    // https://stackoverflow.com/questions/14903379/rounding-to-nearest-fraction-half-quarter-etc
    double x = number * denominator;
    x = floor(x);
    x = x / denominator;
    return x;
}

int StepSequencerViewModel::getZeroBasedOctave() {
    int currentOctave =
        editState.getProperty(app_view_models::IDs::currentOctave);
    int zeroBasedOctave = currentOctave;
    if (MIN_OCTAVE < 0) {
        zeroBasedOctave = currentOctave + abs(MIN_OCTAVE);
    }

    return zeroBasedOctave;
}

} // namespace app_view_models
