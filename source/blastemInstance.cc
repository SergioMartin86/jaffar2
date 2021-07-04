#include "blastemInstance.h"
#include "utils.h"
#include "libco.h"
#include <dlfcn.h>
#include <iostream>
#include <omp.h>


blastemInstance::blastemInstance(int argc, char** argv, const char* libraryFile, const bool multipleLibraries)
{
  if (multipleLibraries)
   _dllHandle = dlmopen (LM_ID_NEWLM, libraryFile, RTLD_NOW | RTLD_LOCAL | RTLD_NODELETE);
  else
   _dllHandle = dlopen (libraryFile, RTLD_NOW);

  if (!_dllHandle)
    EXIT_WITH_ERROR("Could not load %s. Check that this library's path is included in the LD_LIBRARY_PATH environment variable. Try also reducing the number of openMP threads.\n", libraryFile);

  // Variables
  start = (start_t) dlsym(_dllHandle, "start");
  resume = (resume_t) dlsym(_dllHandle, "resume");

  start(argc, argv);
}


blastemInstance::~blastemInstance()
{
  dlclose(_dllHandle);
}
