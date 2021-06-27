#pragma once

#include <string>

class blastemInstance
{
  public:
  blastemInstance(const char* libraryFile, const bool multipleLibraries);
  ~blastemInstance();

  private:
  void *_dllHandle;
};
