
#ifndef CM_EVALUATOR_H
#define CM_EVALUATOR_H

#include "Libraries.h"

class EvaluatorThread;

class EvaluatorThread : public juce::Thread
{
public:
  
  EvaluatorThread() ;
  ~EvaluatorThread();

  juce::WaitableEvent ready;
  
  bool init();
  void quit();
  void run();
  void signal();
  
  juce_DeclareSingleton (EvaluatorThread, true)
};

#endif
