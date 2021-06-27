#include "blastemInstance.h"
#include "utils.h"
#include <dlfcn.h>
#include <iostream>
#include <omp.h>

blastemInstance::blastemInstance(const char* libraryFile, const bool multipleLibraries)
{
  if (multipleLibraries)
   _dllHandle = dlmopen (LM_ID_NEWLM, libraryFile, RTLD_NOW | RTLD_LOCAL | RTLD_NODELETE);
  else
   _dllHandle = dlopen (libraryFile, RTLD_NOW);

  if (!_dllHandle)
    EXIT_WITH_ERROR("Could not load %s. Check that this library's path is included in the LD_LIBRARY_PATH environment variable. Try also reducing the number of openMP threads.\n", libraryFile);
}

blastemInstance::~blastemInstance()
{
  dlclose(_dllHandle);
}
