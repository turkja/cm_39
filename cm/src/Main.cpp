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

#include "Libraries.h"
#include "Enumerations.h"
#include "CmSupport.h"
#include "Console.h"
#include "Scheme.h"
#include "Midi.h"
#include "Main.h"
#include "Audio.h"
#include "Syntax.h"
#include "CodeEditor.h"
#include "PlotWindow.h"
#include "Skin.h"
#include "Preferences.h"
#include "OpenSoundControl.h"
#ifdef WITH_FOMUS
#include "Fomus.h"
#endif

#include "Evaluator.h"

extern Console* globalConsole;

const juce::String Grace::getApplicationName(void)
{
  return "Grace";
}

const juce::String Grace::getApplicationVersion(void)
{
  return SysInfo::getVersionString(SysInfo::CM_VERSION);
}

bool Grace::moreThanOneInstanceAllowed(void)
{
  return SysInfo::isMac();
}

void Grace::anotherInstanceStarted(const juce::String& commandLine)
{
  juce::StringArray droppedfiles;
  // tokenize input with whitespace inside quoted file names preserved
  droppedfiles.addTokens(commandLine, true);
  int size = droppedfiles.size();
  if (size > 0)
  {
    // strip explicit quotes from file names
    for (int i = 0; i < size; i++)
      droppedfiles.set(i, droppedfiles[i].unquoted());
    if (Console::getInstance()->isInterestedInFileDrag(droppedfiles))
      Console::getInstance()->filesDropped(droppedfiles, 0, 0);
  }
}

void Grace::initialise(const juce::String& commandLine)
{
  std::cerr << "Starting Grace...\n";
  std::cerr << "Creating Windows Skin..." << std::flush;
  juce::LookAndFeel::setDefaultLookAndFeel(lookAndFeel = new Skin());

  // Console must exist before any components attempt to write to it!
  std::cerr << "Creating Console..." << std::flush;
  Console::globalInstance = new Console;
  std::cerr << "OK!\n";

  // Load preference file.
  std::cerr << "Creating Preferences..." << std::flush;
  Preferences::getInstance();
  std::cerr << "OK!\n";
  applicationSupportDirectory = Preferences::getInstance()->getProps().getFile().getParentDirectory();
  std::cerr << "applicationSupportDirectory=" << applicationSupportDirectory.getFullPathName() << "\n";

  // Initialize Liblo
  std::cerr << "Creating OpenSoundControl..." << std::flush;
  OpenSoundControl::getInstance();
  std::cerr << "OK!\n";


#ifdef WITH_FOMUS  
  // Initialize fomus
  std::cerr << "Creating Fomus..." << std::flush;
  initfomus();
  std::cerr << "OK!\n";
#endif  

#ifdef WITH_SDIF
  std::cerr << "Creating SDIF..." << std::flush;
  SdifGenInit("");
  std::cerr << "OK!\n";
#endif

  // Scheme
  std::cerr << "Creating Scheme..." << std::flush;
  SchemeThread* scm = SchemeThread::getInstance();
  scm->setPriority(10);
  scm->startThread();
  std::cerr << "OK!\n";

  // Audio Manager
  std::cerr << "Creating AudioManager..." << std::flush;
  AudioManager::getInstance();
  std::cerr << "OK!\n";

  // MidiOut 
  std::cerr << "Creating MidiOutPort..." << std::flush;
  MidiOutPort* mid = MidiOutPort::getInstance();
  mid->setPriority(9);
  mid->startThread();
  std::cerr << "OK!\n";

  // FIXME: COMMAND MANAGER DEPENDS ON PREFS CONSOLE AUDIO AND MIDI
  // create application command manager

  std::cerr << "Creating MenuBarModel..." << std::flush;
  menuBarModel = new GraceMenuBarModel();
  std::cerr << "OK!\n";
  // If we are on the mac then add our App menu bar 
  // create the console window (console may have output in it already)
  std::cerr << "Creating ConsoleWindow..." << std::flush;
  ConsoleWindow* win = new ConsoleWindow();
  std::cerr << "OK!\n";
  //win->getConsole()->toFront(true);

#ifdef JUCE_MAC
  juce::MenuBarModel::setMacMainMenu(menuBarModel);
#endif

  std::cerr << "Registering CommandManager Commands..." << std::flush;
  commandManager.registerAllCommandsForTarget(this);
  menuBarModel->setApplicationCommandManagerToWatch(&commandManager);
  std::cerr << "OK!\n";


  juce::String str = Preferences::getInstance()->getStringProp("LispInitFile");
  if (str != juce::String::empty)
  {
    std::cerr << "Loading LispInitFile..." << std::flush;
    scm->load(juce::File(str),false);
    std::cerr << "OK!\n";
  }


  // Create a thread that just waits for Scheme statements..
  
  EvaluatorThread* evt = EvaluatorThread::getInstance();
  evt->setPriority(10);
  evt->startThread();

}

void Grace::systemRequestedQuit()
{
  int flag = -1; 
  int unsaved = getNumUnsavedWindows();
  if (unsaved > 0)
  {
    juce::String m ("You have ");
    m << unsaved << " document";
    if (unsaved > 1) m << "s";
    m << " with unsaved changes. Do you want to review these changes before quitting?";
    flag = juce::AlertWindow::showYesNoCancelBox(juce::AlertWindow::WarningIcon,
                                                 "Quit Grace", m, "Discard & Quit",
                                                 "Review Changes...", "Cancel");
  }
  //  std::cout << "query return flag=" << flag << "\n";
  // Flag -1: no changed windows, proceed to quit
  // Flag 0: Cancel
  // Flag 1: Discard changes (proceed to quit)
  // Flag 2: Review Changes (and possibly quit)
  if (flag == -1 || flag == 1 || (flag == 2 && queryUnsavedWindows()))
    quit();
}

int Grace::getNumUnsavedWindows()
{
  // return the number of windows that need saving
  int n = 0;
  for (int i = 0; i < juce::TopLevelWindow::getNumTopLevelWindows(); i++)
  {
    juce::TopLevelWindow* w = juce::TopLevelWindow::getTopLevelWindow(i);
    if (CodeEditorWindow* e = dynamic_cast<CodeEditorWindow*>(w))
    {
      if (e->hasUnsavedChanges())
        n++;
    }
    else if (PlotWindow* p = dynamic_cast<PlotWindow*>(w))
    {
      if (p->hasUnsavedChanges())
        n++;
    }
  }
  return n;
}

bool Grace::queryUnsavedWindows()
{
  for (int i = 0; i < juce::TopLevelWindow::getNumTopLevelWindows(); i++)
  {
    juce::TopLevelWindow* w = juce::TopLevelWindow::getTopLevelWindow(i);
    if (!(WindowTypes::isWindowType(w, WindowTypes::CodeEditor) ||
          (WindowTypes::isWindowType(w, WindowTypes::PlotWindow))))
      continue;
    juce::String m = "Save changes you made to document ";
    m << "\"" << w->getName() << "\"?";
    int flag = juce::AlertWindow::showYesNoCancelBox(juce::AlertWindow::QuestionIcon,
                                                     juce::String::empty,
                                                     m, "Don't Save", "Save", "Cancel");
    //    std::cout << "query return flag=" << flag << "\n";
    if (flag == 0) return false;
    if (flag == 1) continue; 
    if (WindowTypes::isWindowType(w, WindowTypes::CodeEditor))
    {
      w->toFront(false);
      CodeEditorWindow* c=(CodeEditorWindow*)w;
      c->saveFile(false);
    }
    else if (WindowTypes::isWindowType(w, WindowTypes::PlotWindow))
    {
    }
  }
  return true;
}

void Grace::shutdown()
{
  // Typing COMMAND-Q on mac brings us here WITHOUT closing any windows.
  std::cerr << "Quiting Grace...\n";
  activeWindow = 0;
  ConsoleWindow* cw = (ConsoleWindow *)Console::getInstance()->getParentComponent();
  // save console window's position and size as preference
  Preferences::getInstance()->setStringProp("ConsoleState", cw->getWindowStateAsString()); 	
  // delete all open windows except the console (which might still be
  // in use by scheme)
  {
    juce::Array<juce::TopLevelWindow*> windows;
    for (int i = 0; i < juce::TopLevelWindow::getNumTopLevelWindows(); i++)
    {
      juce::TopLevelWindow* t = juce::TopLevelWindow::getTopLevelWindow(i);
      if (t != cw) windows.add(t);
    }
    for (int i = 0; i < windows.size(); i++)
      delete windows[i];
  }
  Preferences::getInstance()->getProps().saveIfNeeded();
  // Stop all scheme processing but do NOT delete scheme thread (or
  // app crashes...)
  std::cerr << "Flushing Scheme...clearing midi hooks..." << std::flush;
  SchemeThread::getInstance()->clearMidiHook(-1);
  std::cerr << "clearing osc hooks..."  << std::flush;
  SchemeThread::getInstance()->clearOscHook("*");
  std::cerr << "clearing scheduler..."  << std::flush;
  SchemeThread::getInstance()->schemeNodes.clear(); // remove any running nodes from scheduler
  //  SchemeThread::getInstance()->stop(0, true);
  // Interrupt any scheme eval going on
  std::cerr << "interrupting scheme..."  << std::flush;
  SchemeThread::getInstance()->setSchemeInterrupt(true);
  std::cerr << "OK!\n";

  // Delete console window, this will delete Console
  std::cerr << "Deleting Console Window..."  << std::flush;
  delete cw;
  std::cerr << "OK!\n";

  // Flush midi output queue
  std::cerr << "Flushing MidiOut..." << std::flush;
  MidiOutPort::getInstance()->clear();
  std::cerr << "OK!\n";

  // delete audio manager before midi out because of internal synth
  std::cerr << "Deleting AudioManager..." << std::flush;
  AudioManager::deleteInstance();
  std::cerr << "OK!\n";

  std::cerr << "Deleting MidiOut..." << std::flush;
  MidiOutPort::deleteInstance();
  std::cerr << "OK!\n";

  std::cerr << "Deleting MidiIn..." << std::flush;
  MidiInPort::deleteInstance();
  std::cerr << "OK!\n";

  std::cerr << "Deleting OpenSoundControl..." << std::flush;
  OpenSoundControl::deleteInstance();
  std::cerr << "OK!\n";

#ifdef WITH_FOMUS
  std::cerr << "Deleting Fomus..." << std::flush;
  Fomus::deleteInstance();
  std::cerr << "OK!\n";
#endif

#ifdef WITH_SDIF
  std::cerr << "Deleting SDIF..." << std::flush;
  SdifGenKill();
  std::cerr << "OK!\n";
#endif

  std::cerr << "Deleting Preferences..." << std::flush;
  Preferences::deleteInstance();
  std::cerr << "OK!\n";

  std::cerr << "Deleting Menu Bar Model..." << std::flush;
#if JUCE_MAC  // ..and also the main bar if we're using that on a Mac...
  juce::MenuBarModel::setMacMainMenu (0);
#endif
  delete menuBarModel;
  std::cerr << "OK!\n";

  std::cerr << "Deleting LookAndFeel..." << std::flush;
  delete lookAndFeel;
  std::cerr << "OK!\n";

  std::cerr << "Bye!\n";
}

/** flush any musical processes currently running, interrupt any lisp
    eval taking place and silence any audio/midi output **/

void Grace::reset()
{
  std::cerr << "RESET!\n";
}

START_JUCE_APPLICATION(Grace)

