#pragma once

#include <string>
#include "libco.h"

typedef int (*main_t)(int argc, char ** argv);

class blastemInstance
{
  public:
  blastemInstance(const char* libraryFile, const bool multipleLibraries);
  ~blastemInstance();
  void initialize();

  // blastem Functions
  main_t main;

  // blastem variables
  cothread_t* _jaffarThread;
  cothread_t* _blastemThread;

  private:

  void *_dllHandle;
};
