#pragma once

#include <string>

typedef void (*start_t)(int, char**);
typedef void (*resume_t)(void);

class blastemInstance
{
  public:
  blastemInstance(int argc, char** argv, const char* libraryFile, const bool multipleLibraries);
  ~blastemInstance();

  // blastem Functions
  start_t start;
  resume_t resume;

  private:

  void *_dllHandle;
};
