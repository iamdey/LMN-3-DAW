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

    // set a default before introducing numberOfNotesConstrainer because it
    // relies on it
    notesPerMeasure.referTo(state, IDs::notesPerMeasure, nullptr,
                            MAX_NOTES_PER_MEASURE);

    std::function<int(int)> selectedNoteIndexConstrainer = [this](int param) {
        // selected index cannot be less than 0
        // it also cannot be greater than or equal to the number of notes
        // but it would be nice if the cursor wrapped around :)
        if (param < 0)
            return numberOfNotes.get() - 1;
        else if (param >= numberOfNotes.get())
            return 0;
        else
            return param;
    };

    selectedNoteIndex.setConstrainer(selectedNoteIndexConstrainer);
    selectedNoteIndex.referTo(state, IDs::selectedNoteIndex, nullptr, 0);

    std::function<int(int)> rangeIndexConstrainer = [this](int param) {
        // rangeStartIndex cannot be less than 0
        // it also cannot be greater than the maximum number of notes allowed
        if (param <= 0)
            return 0;
        else if (param >= app_models::StepChannel::getMaxNumberOfNotes(
                              notesPerMeasure.get()))
            return app_models::StepChannel::getMaxNumberOfNotes(
                notesPerMeasure.get());
        else
            return param;
    };

    rangeAnchorIndex.setConstrainer(rangeIndexConstrainer);
    rangeAnchorIndex.referTo(state, IDs::RANGE_ANCHOR_INDEX, nullptr, 0);

    rangeSelectionEnabled.referTo(state, IDs::RANGE_SELECTION_ENABLED, nullptr,
                                  false);

    midiClip = editCurrentMidiClip();

    if (midiClip == nullptr) {
        midiClip = insertMidiClip();
    }

    DBG("clip starts at " + std::to_string(midiClipStart.inSeconds()));
    DBG("clip ends at " + std::to_string(midiClipEnd.inSeconds()));

    generateStepSequenceFromMidi();

    // --------------------------
    // compute numberOfNotes for the saved clip
    const tracktion::TimeRange midiClipTimeRange =
        tracktion::TimeRange(midiClipStart, midiClipEnd);

    auto duration = midiClipTimeRange.getLength();
    double nbBeatsPerSecond = track->edit.tempoSequence.getBeatsPerSecondAt(
        tracktion::TimePosition::fromSeconds(0));

    int nbBeats = juce::roundToInt(duration.inSeconds() * nbBeatsPerSecond);

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
    // compute default total notes according to the duration in beats
    int defaultNbNotes = juce::roundToInt(nbBeats * notesPerMeasure.get() /
                                          NB_BEATS_PER_MEASURE);
    // a default cached value is already set at this point, may be there is an
    // other way to constrain a value
    numberOfNotes.referTo(state, IDs::numberOfNotes, nullptr);
    // so, force to set a new cached value.
    numberOfNotes.setValue(defaultNbNotes, nullptr);
    // --------------------------

    // resync midi clip sequence with patterns
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

    // Cleanup sequence before going out to prevent cached data to restore on
    // next instanciation
    stepSequence.clear();
}

tracktion::MidiClip *StepSequencerViewModel::editCurrentMidiClip() {
    if (auto trackItem = track->getNextTrackItemAt(
            track->edit.getTransport().getPosition())) {
        // only keep the clip if it starts before current pos
        if (track->edit.getTransport().getPosition() <
            trackItem->getPosition().getStart()) {
            // nothing to return
            return nullptr;
        }

        if (strcmp(trackItem->typeToString(trackItem->type), "midi") == 0) {
            if (auto clip = dynamic_cast<tracktion::MidiClip *>(trackItem)) {
                DBG("Edit Current clip");
                // In case trackItem is longer than the maximum duration of the
                // sequence, split the clip.
                if (trackItem->getLengthInBeats().inBeats() >
                    NB_BEATS_PER_MEASURE * MAX_MEASURES) {
                    if (auto clipTrack = clip->getClipTrack()) {
                        double secondsPerBeat =
                            1.0 / track->edit.tempoSequence.getBeatsPerSecondAt(
                                      tracktion::TimePosition::fromSeconds(0));

                        midiClipStart = clip->getPosition().getStart();
                        midiClipEnd = tracktion::TimePosition::fromSeconds(
                            midiClipStart.inSeconds() + NB_BEATS_PER_MEASURE *
                                                            MAX_MEASURES *
                                                            secondsPerBeat);

                        if (auto splittedClip =
                                clipTrack->splitClip(*clip, midiClipEnd)) {
                            DBG("return 1st part of the clip");
                            // splittedClip is the 2nd part of the clip.
                            // We need the 1st part.
                            return dynamic_cast<tracktion::MidiClip *>(clip);
                        }
                    }

                    // nothing to return, split has failed
                    return nullptr;
                }

                DBG("Return initial clip");
                midiClipStart = trackItem->getPosition().getStart();
                midiClipEnd = trackItem->getPosition().getEnd();
                return clip;
            }
        }
    }
    // nothing to return, no clip or not midi clip or unable to get the
    // trackItem as clip.
    return nullptr;
}

tracktion::MidiClip *StepSequencerViewModel::insertMidiClip() {
    DBG("Create new clip");
    double secondsPerBeat = 1.0 / track->edit.tempoSequence.getBeatsPerSecondAt(
                                      tracktion::TimePosition::fromSeconds(0));
    //   note: actually there is a BeatDuration class, I'm not sure if it can be
    //   used here
    auto beatDuration = tracktion::TimeDuration::fromSeconds(secondsPerBeat);

    // compute the available place before next clip
    auto nextClip =
        track->getNextTrackItemAt(track->edit.getTransport().getPosition());

    midiClipStart = track->edit.getTransport().getPosition();

    // hardcoded 4 measures cf. `StepChannel::maxNumberOfMeasures`
    auto maxEnd = tracktion::TimePosition::fromSeconds(
        midiClipStart.inSeconds() + NB_BEATS_PER_MEASURE * secondsPerBeat * 4);

    if (nextClip && nextClip->getPosition().getStart() <= maxEnd) {
        midiClipEnd = nextClip->getPosition().getStart();
    } else {
        // no next clip or empty track until max length of the clip
        midiClipEnd = maxEnd;
    }

    const tracktion::TimeRange midiClipTimeRange =
        tracktion::TimeRange(midiClipStart, midiClipEnd);

    return dynamic_cast<tracktion::MidiClip *>(track->insertNewClip(
        tracktion::TrackItem::Type::midi, "step", midiClipTimeRange, nullptr));
}

int StepSequencerViewModel::getNumChannels() {
    return app_models::StepChannel::maxNumberOfChannels;
}

int StepSequencerViewModel::getNumNotesPerChannel() {
    return app_models::StepChannel::getMaxNumberOfNotes(notesPerMeasure.get());
}

int StepSequencerViewModel::hasNoteAt(int channel, int noteIndex) {
    return stepSequence.getChannel(channel)->getNote(noteIndex);
}

// TODO: refactor with toggleNote(index, channel)

void StepSequencerViewModel::toggleNoteNumberAtSelectedIndex(int noteNumber) {
    // FIXME: it crashes if octave is higher or lower than normal
    int channel = noteNumberToChannel(noteNumber);
    // TODO: rename into `intensity`
    int velocity = hasNoteAt(channel, selectedNoteIndex.get());
    bool active = velocity > 0;
    bool isPlaying = track->edit.getTransport().isPlaying();

    // get current note velocity then change for the next velocity:
    // 3: 30% > 5: 60% > 7: 100% > 0: 0% …
    // The note's “intensity” is encoded on 3 bits (between 0 & 7)
    int nextVelo;
    if (isPlaying && active) {
        nextVelo = 0;
    } else if (isPlaying && !active) {
        // when playing, the note is at max velocity
        nextVelo = 7;
    } else if (velocity < 3) {
        nextVelo = 3;
    } else if (velocity < 5) {
        nextVelo = 5;
    } else if (velocity < 7) {
        nextVelo = 7;
    } else {
        nextVelo = 0;
    }

    stepSequence.getChannel(channel)->setNote(selectedNoteIndex.get(),
                                              nextVelo);

    if (isPlaying) {
        // When looping also add or remove note from the midi sequence to hear
        if (nextVelo == 0) {
            removeNoteFromSequence(channel, selectedNoteIndex.get());
        } else {
            addNoteToSequence(channel, selectedNoteIndex.get(), nextVelo);
        }
    }
}

int StepSequencerViewModel::computeNoteIntensity(tracktion::MidiNote *note) {
    // velocity is encoded as an int between 0-127 (TO CONFIRM)
    // intensity is encoded as an int between 0-7
    // coef is 0,05511811 = 7 * 1/127
    long intensity = std::lround(note->getVelocity() * 0.05511811);

    DBG("Note real intensity " + std::to_string(intensity));

    // 3: 30% | 5: 60% | 7: 100% | 0: 0% …
    // The note's “intensity” is encoded on 3 bits (between 0 & 7)
    int stepIntensity = 0;
    if (intensity >= 2 && intensity < 4) {
        stepIntensity = 3;
    } else if (intensity >= 4 && intensity < 6) {
        stepIntensity = 5;
    } else if (intensity >= 6) {
        stepIntensity = 7;
    }

    return stepIntensity;
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
    if (!track->edit.getTransport().isPlaying()) {
        selectedNoteIndex.setValue(selectedNoteIndex.get() + 1, nullptr);
    }
}

void StepSequencerViewModel::decrementSelectedNoteIndex() {
    if (!track->edit.getTransport().isPlaying()) {
        selectedNoteIndex.setValue(selectedNoteIndex.get() - 1, nullptr);
    }
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

            int nextValue = notesPerMeasureOptions[newIndex];

            notesPerMeasure.setValue(nextValue, nullptr);

            // compute the new number of notes for the measure division
            float prevNbMeasures =
                numberOfNotes.get() / notesPerMeasureOptions[currentIndex];
            int newNbNotes =
                (int)std::round(notesPerMeasure.get() * prevNbMeasures);
            numberOfNotes.setValue(newNbNotes, nullptr);

            // recompute pattern according to the current midiclip
            generateStepSequenceFromMidi();
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

            // recompute pattern according to the current midiclip
            // some notes can be lost during the process
            generateStepSequenceFromMidi();
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

void StepSequencerViewModel::toggleRangeSelection() {
    // Toggle the state (using state or a dedicated field)
    bool nowActive = !rangeSelectionEnabled.get();
    rangeSelectionEnabled.setValue(nowActive, nullptr);

    // Update range markers only if active
    if (!nowActive) {
        rangeAnchorIndex.resetToDefault();
    } else {
        rangeAnchorIndex.setValue(getSelectedNoteIndex(), nullptr);
    }
}

void StepSequencerViewModel::copySelection() {
    // copy all notes in the selected range to an internal buffer
    if (!rangeSelectionEnabled.get()) {
        return;
    }

    copiedNotes.clear();
    int from = getRangeStartIndex();
    int to = getRangeEndIndex();
    DBG("Copying selection from " + std::to_string(from) + " to " +
        std::to_string(to));
    for (int index = from; index <= to; index++) {
        for (int channel = 0; channel < getNumChannels(); channel++) {
            int velocity = hasNoteAt(channel, index);
            if (velocity > 0) {
                Note note;
                note.index = index - from;
                note.channel = channel;
                copiedNotes.push_back(note);
                DBG("Copied note, index " + std::to_string(note.index) +
                    " channel " + std::to_string(note.channel));
            }
        }
    }
}

void StepSequencerViewModel::pasteSelection() {
    // paste notes from our internal buffer at the current cursor pos
    if (rangeSelectionEnabled.get() || copiedNotes.size() == 0) {
        return;
    }

    DBG("Pasting selection");
    int from = getSelectedNoteIndex();
    for (int noteIndex = 0; noteIndex < copiedNotes.size(); noteIndex++) {
        int velocity = hasNoteAt(copiedNotes[noteIndex].channel,
                                 copiedNotes[noteIndex].index);
        addNoteToSequence(copiedNotes[noteIndex].channel,
                          from + copiedNotes[noteIndex].index, velocity);

        stepSequence.getChannel(copiedNotes[noteIndex].channel)
            ->setNote(from + copiedNotes[noteIndex].index, velocity);

        DBG("Pasted note, index " +
            std::to_string(from + copiedNotes[noteIndex].index) + " channel " +
            std::to_string(copiedNotes[noteIndex].channel) + " velocity " +
            std::to_string(velocity));
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

    if (compareAndReset(shouldUpdateRangeSelectionEnabled)) {
        listeners.call([this](Listener &l) {
            l.rangeSelectionEnabledChanged(rangeSelectionEnabled.get());
        });
    }
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

        if (property == IDs::RANGE_SELECTION_ENABLED) {
            markAndUpdate(shouldUpdateRangeSelectionEnabled);
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
    l->rangeSelectionEnabledChanged(rangeSelectionEnabled.get());
}

void StepSequencerViewModel::removeListener(Listener *l) {
    listeners.remove(l);
}

/**
 * Generate channel patterns from midiClip
 * This will also clean the current patterns if any.
 */
void StepSequencerViewModel::generateStepSequenceFromMidi() {
    auto &sequence = midiClip->getSequence();

    // Fill channels
    if (!sequence.isEmpty()) {
        DBG("Start fill patterns");
        // Clear channels before filling again
        stepSequence.clear();

        for (auto note : sequence.getNotes()) {
            auto pos = note->getBeatPosition();
            auto velocity = note->getVelocity();
            auto noteNumber = note->getNoteNumber();
            /* get the closest index of the beat position e.g. `beatsPos *
             * (16/4)` */
            long index = std::lround(pos.inBeats() * notesPerMeasure.get() /
                                     NB_BEATS_PER_MEASURE);
            jassert(index >= 0);
            int intensity = computeNoteIntensity(note);

            stepSequence.getChannel(noteNumberToChannel(noteNumber))
                ->setNote(index, intensity);
        }
    }
}

/**
 * Generate midiclip from channel patterns
 */
void StepSequencerViewModel::generateMidiSequence() {
    auto &sequence = midiClip->getSequence();
    sequence.clear(nullptr);

    for (int i = 0; i < getNumChannels(); i++) {
        for (int j = 0; j < getNumNotesPerChannel(); j++) {
            int velocity = hasNoteAt(i, j);
            if (velocity > 0) {
                addNoteToSequence(i, j, velocity);
            }
        }
    }
}

void StepSequencerViewModel::addNoteToSequence(int channel, int noteIndex,
                                               int dumbVelocity) {
    // dumbVelocity is an int between 0-7
    // velocity is an int between 0 and 127
    int velocity = dumbVelocity * 18;
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
    sequence.addNote(pitch, startBeat, duration, velocity, 1, nullptr);
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
    // auto duration =
    //     tracktion::BeatDuration::fromBeats(4.0 /
    //     double(notesPerMeasure.get()));

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
    tracktion::TimePosition timePosition, bool /*forceJump*/) {
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
