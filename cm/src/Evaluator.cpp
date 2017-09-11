
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

void EvaluatorThread::signal()
{
  ready.signal();
}

// Normally all this work should be done by the Scheme itself..

void EvaluatorThread::run()
{
  std::cerr << "Welcome to Grace shell.\n" << std::flush;
  std::cerr << "> " << std::flush;
  std::string s;
  std::string s_exp;
  while (true) {
    size_t o = 0, c = 0;
    //if (input) std::cerr << "> " << std::flush;

    // Read line
    while (true) {
      // Clear EOF
      if (!std::getline(std::cin, s)) {
	std::cin.clear();
	break;
      }
      // Consider only characters leading up to ";"
      std::size_t cf = s.find_first_of(";");
      if (cf != std::string::npos)
	s.resize(cf);

      s_exp.append(s);
      
      // Count for for opening "("
      o += std::count(s.begin(), s.end(), '(');
      // Count for closing ")"
      c += std::count(s.begin(), s.end(), ')');
      
      // XXX: more intelligent parsing, check for unbalanced parenthesis etc. (?)
      if (c >= o) {
	o = 0; c = 0;
	break;
      }
    }
    // Skip leading whitespaces
    std::size_t cf = s_exp.find_first_not_of(" ");
    if (cf != std::string::npos)
      s_exp = s_exp.substr(cf);

    //std::cout << "S-EXPRESSION: " << s_exp << " (" << s_exp.length() << ")\n";
    if (s_exp.length()) {
      // Reset the event before evaluation
      ready.reset();
      SchemeThread::getInstance()->eval("(begin " + s_exp + ")", true);
      s_exp = "";

      // Wait for evaluation to finish
      ready.wait();
      std::cerr << "> " << std::flush;
    }
  }
}

void EvaluatorThread::quit()
{
  ;
}
