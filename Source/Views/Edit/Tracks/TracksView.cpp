#include "TracksView.h"
#include "AppLookAndFeel.h"
#include "TrackPluginsListView.h"
#include "TrackModifiersListView.h"
#include <app_navigation/app_navigation.h>
#include "AvailableSequencersListView.h"

TracksView::TracksView(tracktion_engine::Edit& e, app_services::MidiCommandManager& mcm)
    : edit(e),
      midiCommandManager(mcm),
      camera(7),
      viewModel(e, camera),
      listModel(std::make_unique<TracksListBoxModel>(viewModel.listViewModel, camera)),
      singleTrackView(std::make_unique<TrackView>(dynamic_cast<tracktion_engine::AudioTrack*>(viewModel.listViewModel.getSelectedItem()), camera))
{
    edit.ensureNumberOfAudioTracks(8);

    multiTrackListBox.setModel(listModel.get());
    multiTrackListBox.getViewport()->setScrollBarsShown(false, false);
    multiTrackListBox.setColour(juce::ListBox::backgroundColourId, juce::Colour(0x00282828));
    multiTrackListBox.setAlwaysOnTop(true);
    addAndMakeVisible(multiTrackListBox);

    addAndMakeVisible(singleTrackView.get());

    addAndMakeVisible(informationPanel);



    playheadComponent.setAlwaysOnTop(true);
    addAndMakeVisible(playheadComponent);

    loopMarkerComponent.setAlwaysOnTop(true);
    addAndMakeVisible(loopMarkerComponent);
    loopMarkerComponent.setVisible(false);

    midiCommandManager.addListener(this);
    // since this is the initial view we will manually set it to be the focused component
    midiCommandManager.setFocusedComponent(this);

    viewModel.addListener(this);
    viewModel.listViewModel.addListener(this);
    viewModel.listViewModel.itemListState.addListener(this);

    juce::Timer::callAfterDelay(1, [this](){singleTrackListBox.scrollToEnsureRowIsOnscreen(viewModel.listViewModel.itemListState.getSelectedItemIndex());});

    startTimerHz(60);

}

TracksView::~TracksView()
{

    midiCommandManager.removeListener(this);
    viewModel.removeListener(this);
    viewModel.listViewModel.removeListener(this);
    viewModel.listViewModel.itemListState.removeListener(this);

}

void TracksView::paint(juce::Graphics& g)
{

    g.fillAll(getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.setColour(juce::Colours::white);

}

void TracksView::resized()
{

    informationPanel.setBounds(0, 0, getWidth(), getHeight() / 4);


    singleTrackView->setBounds(0, informationPanel.getHeight(), getWidth(), getHeight() - informationPanel.getHeight());

    multiTrackListBox.setBounds(0, informationPanel.getHeight(), getWidth(), getHeight() - informationPanel.getHeight());
    multiTrackListBox.setRowHeight(getHeight() / 6);


}


void TracksView::encoder1Increased()
{

    if (isShowing())
        if (midiCommandManager.getFocusedComponent() == this)
            viewModel.listViewModel.itemListState.setSelectedItemIndex(viewModel.listViewModel.itemListState.getSelectedItemIndex() + 1);

}

void TracksView::encoder1Decreased()
{

    if (isShowing())
        if (midiCommandManager.getFocusedComponent() == this)
            viewModel.listViewModel.itemListState.setSelectedItemIndex(viewModel.listViewModel.itemListState.getSelectedItemIndex() - 1);

}

void TracksView::encoder1ButtonReleased()
{

    if (isShowing())
    {
        if (midiCommandManager.getFocusedComponent() == this)
        {

            viewModel.setTracksViewType(app_view_models::TracksListViewModel::TracksViewType::SINGLE_TRACK);
            juce::Timer::callAfterDelay(1, [this](){sendLookAndFeelChange();});

        }

    }


}

void TracksView::encoder2Increased()
{

    if (isShowing())
    {

        if (midiCommandManager.getFocusedComponent() == this)
        {

            if (midiCommandManager.isShiftDown)
                viewModel.nudgeLoopInForward();
            else
                viewModel.nudgeLoopOutForward();

        }

    }

}
void TracksView::encoder2Decreased()
{

    if (isShowing())
    {

        if (midiCommandManager.getFocusedComponent() == this)
        {

            if (midiCommandManager.isShiftDown)
                viewModel.nudgeLoopInBackward();
            else
                viewModel.nudgeLoopOutBackward();

        }

    }

}

void TracksView::encoder3Increased()
{

    if (isShowing())
    {

        if (midiCommandManager.getFocusedComponent() == this)
        {

            if (midiCommandManager.isShiftDown)
                viewModel.nudgeTransportForwardToNearestBeat();
            else
                viewModel.nudgeTransportForward();

        }

    }

}

void TracksView::encoder3Decreased()
{

    if (isShowing())
    {

        if (midiCommandManager.getFocusedComponent() == this)
        {

            if (midiCommandManager.isShiftDown)
                viewModel.nudgeTransportBackwardToNearestBeat();
            else
                viewModel.nudgeTransportBackward();

        }

    }

}

void TracksView::cutButtonReleased()
{

    if (isShowing())
        if (midiCommandManager.getFocusedComponent() == this)
            viewModel.cutSelectedTracksClipAtPlayHead();

}

void TracksView::pasteButtonReleased()
{

    if (isShowing())
        if (midiCommandManager.getFocusedComponent() == this)
            viewModel.pasteClipboardContentToTrackAtPlayhead();

}


void TracksView::splitButtonReleased()
{

    if (isShowing())
        if (midiCommandManager.getFocusedComponent() == this)
            viewModel.splitSelectedTracksClipAtPlayHead();


}

void TracksView::plusButtonReleased()
{

    if (isShowing())
        if (midiCommandManager.getFocusedComponent() == this)
            viewModel.addTrack();

}

void TracksView::minusButtonReleased()
{

    if (isShowing())
        if (midiCommandManager.getFocusedComponent() == this)
            viewModel.deleteSelectedTrack();

}

void TracksView::sequencersButtonReleased()
{

    if (isShowing())
    {

        if (midiCommandManager.getFocusedComponent() == this)
        {

            if (auto track = dynamic_cast<tracktion_engine::AudioTrack*>(viewModel.listViewModel.getSelectedItem()))
            {

                if (auto stackNavigationController = findParentComponentOfClass<app_navigation::StackNavigationController>())
                {

                    stackNavigationController->push(new AvailableSequencersListView(*track, midiCommandManager));
                    midiCommandManager.setFocusedComponent(stackNavigationController->getTopComponent());

                }

            }

        }

    }

}

void TracksView::pluginsButtonReleased()
{
    if (isShowing())
    {

        if (midiCommandManager.getFocusedComponent() == this)
        {

            if (auto track = dynamic_cast<tracktion_engine::AudioTrack*>(viewModel.listViewModel.getSelectedItem()))
            {

                if (auto stackNavigationController = findParentComponentOfClass<app_navigation::StackNavigationController>())
                {

                    stackNavigationController->push(new TrackPluginsListView(*track, midiCommandManager));
                    midiCommandManager.setFocusedComponent(stackNavigationController->getTopComponent());

                }

            }

        }

    }

}

void TracksView::modifiersButtonReleased()
{

    if (isShowing())
    {

        if (midiCommandManager.getFocusedComponent() == this)
        {

            if (auto track = dynamic_cast<tracktion_engine::AudioTrack*>(viewModel.listViewModel.getSelectedItem()))
            {

                if (auto stackNavigationController = findParentComponentOfClass<app_navigation::StackNavigationController>())
                {

                    stackNavigationController->push(new TrackModifiersListView(*track, midiCommandManager));
                    midiCommandManager.setFocusedComponent(stackNavigationController->getTopComponent());

                }

            }

        }

    }

}

void TracksView::loopingChanged(bool looping)
{

    informationPanel.setIsLooping(looping);
    loopMarkerComponent.setVisible(looping);
    resized();

}
void TracksView::loopInButtonReleased()
{

    viewModel.setLoopIn();

}

void TracksView::loopOutButtonReleased()
{

    viewModel.setLoopOut();

}

void TracksView::loopButtonReleased()
{

    viewModel.toggleLooping();

}

void TracksView::recordButtonReleased()
{

    if (isShowing())
        if (midiCommandManager.getFocusedComponent() == this)
            viewModel.startRecording();

}

void TracksView::playButtonReleased()
{

    if (isShowing())
        if (midiCommandManager.getFocusedComponent() == this)
            viewModel.startPlaying();

}

void TracksView::stopButtonReleased()
{

    if (isShowing())
        if (midiCommandManager.getFocusedComponent() == this)
            viewModel.stopRecordingOrPlaying();

}

void TracksView::tracksButtonReleased()
{
    if (isShowing())
    {

        if (midiCommandManager.getFocusedComponent() == this)
        {

            multiTrackListBox.updateContent();
            viewModel.setTracksViewType(app_view_models::TracksListViewModel::TracksViewType::MULTI_TRACK);
            juce::Timer::callAfterDelay(1, [this](){sendLookAndFeelChange();});

        }

    }

}

void TracksView::selectedIndexChanged(int newIndex)
{

    multiTrackListBox.updateContent();
    multiTrackListBox.selectRow(viewModel.listViewModel.itemListState.getSelectedItemIndex());

    if (auto track = dynamic_cast<tracktion_engine::AudioTrack*>(viewModel.listViewModel.getSelectedItem()))
    {
        singleTrackView.reset();
        singleTrackView = std::make_unique<TrackView>(*track, camera);
        singleTrackView->setSelected(true);
        singleTrackView->setBounds(0, informationPanel.getHeight(), getWidth(), getHeight() - informationPanel.getHeight());
        singleTrackView->setAlwaysOnTop(true);
        addChildComponent(singleTrackView.get());
        playheadComponent.setAlwaysOnTop(true);
        addAndMakeVisible(playheadComponent);
        playheadComponent.toFront(false);
        loopMarkerComponent.toFront(false);
    }

    if (viewModel.getTracksViewType() == app_view_models::TracksListViewModel::TracksViewType::SINGLE_TRACK)
    {
        singleTrackView->setVisible(true);
        multiTrackListBox.setVisible(false);
    } else
    {
        singleTrackView->setVisible(false);
        multiTrackListBox.setVisible(true);
    }


    // When deleting tracks in quick succession sometimes the selectedItem is null
    if (viewModel.listViewModel.getSelectedItem() != nullptr)
    {
        informationPanel.setTrackNumber(viewModel.listViewModel.getSelectedItem()->getName().trimCharactersAtStart("Track "));
    }

    sendLookAndFeelChange();
    repaint();
    resized();

}

void TracksView::isRecordingChanged(bool isRecording)
{

    informationPanel.setIsRecording(isRecording);

}

void TracksView::isPlayingChanged(bool isPlaying)
{

    informationPanel.setIsPlaying(isPlaying);

}

void TracksView::itemsChanged()
{

    multiTrackListBox.updateContent();
    multiTrackListBox.scrollToEnsureRowIsOnscreen(viewModel.listViewModel.itemListState.getSelectedItemIndex());

    if (auto track = dynamic_cast<tracktion_engine::AudioTrack*>(viewModel.listViewModel.getSelectedItem()))
    {
        singleTrackView.reset();
        singleTrackView = std::make_unique<TrackView>(*track, camera);
        singleTrackView->setSelected(true);
        singleTrackView->setBounds(0, informationPanel.getHeight(), getWidth(), getHeight() - informationPanel.getHeight());
        singleTrackView->setAlwaysOnTop(true);
        addChildComponent(singleTrackView.get());
        playheadComponent.setAlwaysOnTop(true);
        addAndMakeVisible(playheadComponent);
        playheadComponent.toFront(false);
        loopMarkerComponent.toFront(false);


    }


    sendLookAndFeelChange();
    resized();
    repaint();

}

void TracksView::tracksViewTypeChanged(app_view_models::TracksListViewModel::TracksViewType type)
{

    switch (type)
    {

        case app_view_models::TracksListViewModel::TracksViewType::SINGLE_TRACK:

            singleTrackView->setVisible(true);
            multiTrackListBox.setVisible(false);
            break;

        case app_view_models::TracksListViewModel::TracksViewType::MULTI_TRACK:

            multiTrackListBox.setVisible(true);
            singleTrackView->setVisible(false);
            break;

        default:

            multiTrackListBox.setVisible(true);
            singleTrackView->setVisible(false);
            break;
    }

    sendLookAndFeelChange();
    resized();
    repaint();

}

void TracksView::buildBeats()
{

    beats.clear();

    double pxPerSec = getWidth() / camera.getScope();
    double secondsPerBeat = (1.0 / edit.tempoSequence.getBeatsPerSecondAt(0.0));
    // give a few extra beats per screen
    int beatsPerScreen = (camera.getScope() / secondsPerBeat) + 2;
    double pxPerBeat = secondsPerBeat * pxPerSec;

    double leftEdgeBeat = edit.tempoSequence.timeToBeats(camera.getCenter() - (camera.getScope() / 2.0));
    double beatOffset = ceil(leftEdgeBeat) - leftEdgeBeat;
    double timeOffset = secondsPerBeat * beatOffset;

    double pxOffset = timeOffset * pxPerSec;

    int leftEdgeBeatNumber = ceil(leftEdgeBeat);

    for (int i = 0; i < beatsPerScreen; i++)
    {

        double beatX = (i * pxPerBeat) + pxOffset;
        int beatNumber = leftEdgeBeatNumber + i;

        beats.add(new juce::DrawableRectangle());
        beats.getLast()->setFill(juce::FillType(appLookAndFeel.textColour));
        beats.getLast()->setStrokeFill(juce::FillType(appLookAndFeel.textColour));
        juce::Point<float> topLeft(beatX - .5, informationPanel.getHeight());
        juce::Point<float> topRight(beatX + .5, informationPanel.getHeight());
        juce::Point<float> bottomLeft(beatX - .5, getHeight());
        juce::Parallelogram<float> bounds(topLeft, topRight, bottomLeft);
        beats.getLast()->setRectangle(bounds);

        if (beatNumber % 4 == 0)
        {

            juce::Point<float> topLeft(beatX - 1.5, informationPanel.getHeight());
            juce::Point<float> topRight(beatX + 1.5, informationPanel.getHeight());
            juce::Point<float> bottomLeft(beatX - 1.5, getHeight());
            juce::Parallelogram<float> bounds(topLeft, topRight, bottomLeft);
            beats.getLast()->setRectangle(bounds);

        }


        addAndMakeVisible(beats.getLast());


    }

}

void TracksView::timerCallback()
{

    informationPanel.setTimecode(edit.getTimecodeFormat().getString(edit.tempoSequence, edit.getTransport().getCurrentPosition(), false));
    playheadComponent.setBounds(camera.timeToX(edit.getTransport().getCurrentPosition(),
                                getWidth()),
                                informationPanel.getHeight(), 2, getHeight() - informationPanel.getHeight());

    double loop1X = camera.timeToX(edit.getTransport().loopPoint1, getWidth());
    double loop2X = camera.timeToX(edit.getTransport().loopPoint2, getWidth());
    double loopEndpointRadius = 8;
    loopMarkerComponent.setBounds(loop1X - loopEndpointRadius , informationPanel.getHeight() - loopEndpointRadius, loop2X - loop1X + 2*loopEndpointRadius, 2*loopEndpointRadius);

    buildBeats();
    repaint();

}




