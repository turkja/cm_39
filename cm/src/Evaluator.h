
#ifndef CM_EVALUATOR_H
#define CM_EVALUATOR_H

#include "Libraries.h"

class EvaluatorThread;

class EvaluatorThread : public juce::Thread
{
public:
  
  EvaluatorThread() ;
  ~EvaluatorThread();

  bool init();
  void quit();
  void run();
  
  juce_DeclareSingleton (EvaluatorThread, true)
};

#endif
