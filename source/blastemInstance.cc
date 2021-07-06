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
  _stateData = (uint8_t**) dlsym(_dllHandle, "_stateData");
  _stateSize = (size_t*) dlsym(_dllHandle, "_stateSize");
  _stateWorkRamOffset = (size_t*) dlsym(_dllHandle, "_stateWorkRamOffset");

  start(argc, argv);
}


blastemInstance::~blastemInstance()
{
  dlclose(_dllHandle);
}

void blastemInstance::updateState()
{
 memcpyBigEndian16(&_state.currentFrame,      &(*_stateData)[*_stateWorkRamOffset + 0x19C8]);
 memcpyBigEndian8(&_state.framesPerStep,      &(*_stateData)[*_stateWorkRamOffset + 0x5005]);
 memcpyBigEndian8(&_state.currentLevel,       &(*_stateData)[*_stateWorkRamOffset + 0x4AA5]);
 memcpyBigEndian8(&_state.drawnRoom,          &(*_stateData)[*_stateWorkRamOffset + 0x4A36]);
 memcpyBigEndian16(&_state.minutesLeft,       &(*_stateData)[*_stateWorkRamOffset + 0x4C90]);
 memcpyBigEndian16(&_state.twelthSecondsLeft, &(*_stateData)[*_stateWorkRamOffset + 0x4FC8]);
}

void blastemInstance::printState()
{
 printf("-----------------------------------------------\n");
 printf("State Information:\n");
 printf("Current Frame: %d\n", _state.currentFrame);
 printf("Video Frames Per Game Frame: %d\n", _state.framesPerStep);
 printf("Current Level: %d\n", _state.currentLevel);
 printf("Drawn Room: %d\n", _state.drawnRoom);
 printf("Minutes Left: %d\n", _state.minutesLeft - 1);
 printf("Seconds Left: %d\n", _state.twelthSecondsLeft / 12 );
}

void blastemInstance::playFrame()
{
 resume();
 updateState();
 printState();
}
