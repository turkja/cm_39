
#include "Libraries.h"
#include "Enumerations.h"
#include "Evaluator.h"
#include "Console.h"
#include "Midi.h"
#include "Csound.h"
#include "CmSupport.h"
#include "SndLib.h"
#include "Scheme.h"

#ifdef WITH_FOMUS
#include "Fomus.h"
#endif

#include "Preferences.h"

#include "Syntax.h"

juce_ImplementSingleton(EvaluatorThread)


EvaluatorThread::EvaluatorThread() : juce::Thread("Evaluator Thread")
{
  ;
}

EvaluatorThread::~EvaluatorThread()
{
  ;
}

bool EvaluatorThread::init()
{
  return true;
}

void EvaluatorThread::run()
{

  std::cout << "\n";
  std::string s;
  std::string s_exp;
  
  while (true) {
    size_t o = 0, c = 0;
    std::cout << "GRACE> ";

    // Read line
    while (true) {
      std::getline(std::cin, s);
      // Count for for opening "("
      o += std::count(s.begin(), s.end(), '(');
      // Count for closing ")"
      c += std::count(s.begin(), s.end(), ')');
      s_exp.append(s);
      // XXX: more intelligent parsing
      if (c >= o) {
	o = 0; c = 0;
	break;
      }
    }
    //std::cout << "INPUT: " << s << "\n";
    //std::cout << "S-EXPRESSION: " << s_exp << "\n";
    if (s_exp != "") {
      SchemeThread::getInstance()->eval("(begin " + s_exp + ")", true);
      s_exp = "";
    }
  }
}

void EvaluatorThread::quit()
{
  ;
}
