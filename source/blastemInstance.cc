#include "blastemInstance.h"
#include "utils.h"
#include <dlfcn.h>
#include <iostream>
#include <omp.h>

#define SMD_HEADER_SIZE 512
#define SMD_MAGIC1 0x03
#define SMD_MAGIC2 0xAA
#define SMD_MAGIC3 0xBB
#define SMD_BLOCK_SIZE 0x4000

void blastemInstance::initialize()
{
 // Parsing ROM path
 const char *romPath = std::getenv("JAFFAR2_ROM_PATH");
 if (romPath == NULL) EXIT_WITH_ERROR("[Jaffar] JAFFAR2_ROM_PATH environment variable not defined.\n");

}

blastemInstance::blastemInstance(const char* libraryFile, const bool multipleLibraries)
{
  if (multipleLibraries)
   _dllHandle = dlmopen (LM_ID_NEWLM, libraryFile, RTLD_NOW | RTLD_LOCAL | RTLD_NODELETE);
  else
   _dllHandle = dlopen (libraryFile, RTLD_NOW);

  if (!_dllHandle)
    EXIT_WITH_ERROR("Could not load %s. Check that this library's path is included in the LD_LIBRARY_PATH environment variable. Try also reducing the number of openMP threads.\n", libraryFile);

  // Functions

  alloc_config_system = (alloc_config_system_t) dlsym(_dllHandle, "alloc_config_system");
  render_video_loop = (render_video_loop_t) dlsym(_dllHandle, "render_video_loop");
  detect_system_type = (detect_system_type_t) dlsym(_dllHandle, "detect_system_type");
  main = (main_t) dlsym(_dllHandle, "main");

  // Variables
  current_system = (system_header**) dlsym(_dllHandle, "current_system");
  _jaffarThread = (cothread_t*) dlsym(_dllHandle, "_jaffarThread");
  _blastemThread = (cothread_t*) dlsym(_dllHandle, "_blastemThread");
}

blastemInstance::~blastemInstance()
{
  dlclose(_dllHandle);
}
