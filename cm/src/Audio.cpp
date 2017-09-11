/*
  ==============================================================================

  Copyright 1999-2013 Rick Taube.  All rights reserved.

  Licensed under the "Attribution-NonCommercial-ShareAlike" Vizsage
  Public License, which says that non-commercial users may share and
  modify this code but must give credit and share improvements. For
  complete terms please read the text of the full license available at
  this link: http://vizsage.com/license/Vizsage-License-BY-NC-SA.html

  ==============================================================================
*/

#include "Enumerations.h"
#include "Preferences.h"
#include "Audio.h"
#include "Console.h"
#include "AudioFilePlayer.h"

/*=======================================================================*
  Global Audio Manager
  *=======================================================================*/
 
juce_ImplementSingleton(AudioManager)

AudioManager::AudioManager()
  : internalSynth(0)
{
  audioDeviceManager.initialise(2, 2, 0, true);
  createInternalSynth();
}

AudioManager::~AudioManager()
{
  //  audioDeviceManager.removeAudioCallback (&synthPlayer);
  //  synthPlayer.setProcessor(0);
  // Flush any midi input
  synthPlayer.getMidiMessageCollector().reset(1000);
  audioDeviceManager.removeAudioCallback(&synthPlayer);
  synthPlayer.setProcessor(0);
}

bool AudioManager::createInternalSynth()
{
#ifdef JUCE_MAC
  pluginFormatManager.addDefaultFormats();
  juce::OwnedArray<juce::PluginDescription> buf;
  juce::String err;
  juce::PluginDescription des;

  // Create the DLSMusicDevice (see: 'auval -a')
  des.fileOrIdentifier = "AudioUnit:Synths/aumu,dls ,appl";

  internalSynth = pluginFormatManager.createPluginInstance(des, synthGraph.getSampleRate(), synthGraph.getBlockSize(), err);
  if(!internalSynth)
  {
    //    juce::AlertWindow::showMessageBox(juce::AlertWindow::WarningIcon, "Couldn't create Internal Synth", err);
    std::cerr << "Couldn't create Internal Synth\n";
    return false;
  }

  // These have to be hooked up first or the AudioOutput graph node
  // wont allow connections.
  audioDeviceManager.addAudioCallback(&synthPlayer);
  synthPlayer.setProcessor(&synthGraph);

  // Create nodes for the synth's AudioProcessorGraph
  juce::AudioProcessorGraph::AudioGraphIOProcessor* midiInputNode = new juce::AudioProcessorGraph::AudioGraphIOProcessor(juce::AudioProcessorGraph::AudioGraphIOProcessor::midiInputNode);
  juce::AudioProcessorGraph::AudioGraphIOProcessor* audioOutputNode = new juce::AudioProcessorGraph::AudioGraphIOProcessor(juce::AudioProcessorGraph::AudioGraphIOProcessor::audioOutputNode);
  juce::AudioProcessorGraph::Node* N1 = synthGraph.addNode(midiInputNode);
  juce::AudioProcessorGraph::Node* N2 = synthGraph.addNode(internalSynth);
  juce::AudioProcessorGraph::Node* N3 = synthGraph.addNode(audioOutputNode);
  if(!N1 || !N2 || !N3)
  {
    // delete unadded (graph owns added)
    if(!N1) delete midiInputNode;
    if(!N2) delete internalSynth;
    if(!N3) delete audioOutputNode;
    internalSynth = 0;
    return false;
  }

  int MCI = juce::AudioProcessorGraph::midiChannelIndex;
  /*
    std::cerr << "Midi input node id=" << N1->nodeId << "\n";
    std::cerr << "Internal synth node id=" << N2->nodeId << "\n";
    std::cerr << "Audio output node id=" << N3->nodeId << "\n";
    std::cerr << "Internal synth output chans=" << internalSynth->getNumOutputChannels() << "\n";
    for (int i = 0; i < internalSynth->getNumOutputChannels(); i++)
    std::cerr << i << ":" << internalSynth->getOutputChannelName(i) << "\n";
    std::cerr << "Output node input chans=" << AudioOutputNode->getNumInputChannels() << "\n";
    for (int i = 0; i < AudioOutputNode->getNumInputChannels(); i++)
    std::cerr << i << ":" << AudioOutputNode->getInputChannelName(i) << "\n";
    std::cerr << "can connect Midi->Synth=" << synthGraph.canConnect(N1->nodeId, MCI, N2->nodeId, MCI) << "\n";
    std::cerr << "can connect Synth0->Audio0=" << synthGraph.canConnect(N2->nodeId, 0, N3->nodeId, 0) << "\n";
    std::cerr << "can connect Synth1->Audio1=" << synthGraph.canConnect(N2->nodeId, 1, N3->nodeId, 1) << "\n";
  */
  if(!synthGraph.canConnect(N1->nodeId, MCI, N2->nodeId, MCI) ||
     !synthGraph.canConnect(N2->nodeId, 0, N3->nodeId, 0) ||
     !synthGraph.canConnect(N2->nodeId, 1, N3->nodeId, 1))
  {
    // set to null but dont delete! (already owned by graph)
    internalSynth = 0;
    return false;
  }

  synthGraph.addConnection(N1->nodeId, MCI, N2->nodeId, MCI);
  synthGraph.addConnection(N2->nodeId, 0, N3->nodeId, 0);
  synthGraph.addConnection(N2->nodeId, 1, N3->nodeId, 1);
  //  std::cerr << "is connected=" << synthGraph.isConnected(N2->nodeId, N3->nodeId) << "\n";
  // End AudioProcessorGraph creation
#endif
  return true;
}

bool AudioManager::sendMessageToPluginGraph(const juce::MidiMessage &message)
{
  // stop a juce assertion about time stamp 0 with midi
  // collectors. 
  if (message.getTimeStamp() == 0.0)
  {
    juce::MidiMessage fixed = juce::MidiMessage(message, juce::Time::getMillisecondCounterHiRes() * .001);
    synthPlayer.getMidiMessageCollector().addMessageToQueue(fixed);
  }
  else
    synthPlayer.getMidiMessageCollector().addMessageToQueue(message);
  return true;
}

void AudioManager::openSynthSettings()
{
  //  static juce::AudioProcessorEditor* e = 0;
  if (internalSynth)
  {
    if (juce::AudioProcessorEditor* comp = internalSynth->createEditorIfNeeded())
    {
      juce::DialogWindow::LaunchOptions dw;
      dw.useNativeTitleBar = true;
      dw.resizable = false;
      dw.dialogTitle = "DLSMusicDevice Settings";
      dw.dialogBackgroundColour = ColorThemeIDs::getWindowBackgroundColor();
      dw.content.set(comp, false);
      dw.runModal();
    }
  }
}

void AudioManager::openAudioSettings()
{
  // Open an AudioDeviceSelectorComponent but without midi selections
  // (these are handled by the Midi Manger)
  juce::AudioDeviceSelectorComponent* comp
    = new juce::AudioDeviceSelectorComponent(audioDeviceManager, 0, 256, 0, 256, false, false, true, false);
  comp->setSize(500, 270);
  juce::DialogWindow::LaunchOptions dw;
  dw.useNativeTitleBar = true;
  dw.resizable = false;
  dw.dialogTitle = "Audio Settings";
  dw.dialogBackgroundColour = ColorThemeIDs::getWindowBackgroundColor();
  dw.content.setOwned(comp);
  dw.runModal();
}

// DEBUGGING
void AudioManager::listAllPlugins()
{
  juce::KnownPluginList plugins;
  juce::AudioPluginFormatManager* formats = &pluginFormatManager;
  
  std::cerr << "Audio Plugin Formats:\n";
  for(int i = 0; i < formats->getNumFormats(); i++)
  {
    juce::AudioPluginFormat* f = formats->getFormat(i);
    std::cerr << f->getName() << " paths:\n";
    std::cerr << f->getDefaultLocationsToSearch().toString();
    juce::PluginDirectoryScanner scanner
      (plugins, *f, f->getDefaultLocationsToSearch(), true, juce::File::nonexistent);
    bool Searching = true;
    while(Searching)
    {
      /*Exclude the Sibelius GM module which has an annoying message box that
        pops up, thereby stalling the application. Further plug-ins can be
        excluded from the scan by separating their names with a pipe.*/
      juce::String ExcludedPlugins = "General MIDI Module|";
      juce::String NextPlugin;
      juce::String scannedPluginName;
      NextPlugin << scanner.getNextPluginFileThatWillBeScanned();
      if(ExcludedPlugins.contains(NextPlugin))
        Searching = scanner.skipNextFile();
      else
        Searching = scanner.scanNextFile(true, scannedPluginName);
    }
  }
  
  for(int i = 0; i < plugins.getNumTypes(); i++)
  {
    juce::PluginDescription* desc = plugins.getType(i);
    std::cerr << i << ": " << desc->name << " " << desc->fileOrIdentifier << "\n";
  }
}

void AudioManager::stopAudioPlayback()
{
  // TODO: cycle through audiofileplayer windows and stop them playing
}
