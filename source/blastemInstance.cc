#include "blastemInstance.h"
#include "utils.h"
#include "libco.h"
#include <dlfcn.h>
#include <iostream>
#include "metrohash64.h"
#include <omp.h>


blastemInstance::blastemInstance(const char* libraryFile, const bool multipleLibraries)
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
  reloadState = (reloadState_t) dlsym(_dllHandle, "reloadState");
  _stateData = (uint8_t**) dlsym(_dllHandle, "_stateData");
  _stateSize = (size_t*) dlsym(_dllHandle, "_stateSize");
  _stateWorkRamOffset = (size_t*) dlsym(_dllHandle, "_stateWorkRamOffset");
  _nextMove = (move_t*) dlsym(_dllHandle, "_nextMove");

}

void blastemInstance::initialize(char* romFile, char* saveFile)
{
 int argc = 4;
 char* argv[4];

 char exec[] = "jaffar";
 char flag[] = "-s";
 argv[0] = exec;
 argv[1] = romFile;
 argv[2] = flag;
 argv[3] = saveFile;
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
 memcpyBigEndian32(&_state.checkpointPointer, &(*_stateData)[*_stateWorkRamOffset + 0x4FF0]);
 memcpyBigEndian8(&_state.slowfallFramesLeft, &(*_stateData)[*_stateWorkRamOffset + 0x4AA1]);

 memcpyBigEndian8(&_state.kidFrame,           &(*_stateData)[*_stateWorkRamOffset + 0x4C45]);
 memcpyBigEndian8(&_state.kidCurrentHP,       &(*_stateData)[*_stateWorkRamOffset + 0x4C4F]);
 memcpyBigEndian8(&_state.kidMaxHP,           &(*_stateData)[*_stateWorkRamOffset + 0x4C50]);
 memcpyBigEndian8(&_state.kidRoom,            &(*_stateData)[*_stateWorkRamOffset + 0x4C4B]);
 memcpyBigEndian8(&_state.kidHasSword,        &(*_stateData)[*_stateWorkRamOffset + 0x4C4D]);
 memcpyBigEndian8(&_state.kidDirection,       &(*_stateData)[*_stateWorkRamOffset + 0x4C3D]);
 memcpyBigEndian16(&_state.kidPositionX,      &(*_stateData)[*_stateWorkRamOffset + 0x4C3E]);
 memcpyBigEndian16(&_state.kidPositionY,      &(*_stateData)[*_stateWorkRamOffset + 0x4C40]);

 memcpyBigEndian8(&_state.guardFrame,         &(*_stateData)[*_stateWorkRamOffset + 0x4AFB]);
 memcpyBigEndian8(&_state.guardCurrentHP,     &(*_stateData)[*_stateWorkRamOffset + 0x4B05]);
 memcpyBigEndian8(&_state.guardMaxHP,         &(*_stateData)[*_stateWorkRamOffset + 0x4B06]);
 memcpyBigEndian8(&_state.guardRoom,          &(*_stateData)[*_stateWorkRamOffset + 0x4B01]);
 memcpyBigEndian8(&_state.guardDirection,     &(*_stateData)[*_stateWorkRamOffset + 0x4AF3]);
 memcpyBigEndian16(&_state.guardPositionX,    &(*_stateData)[*_stateWorkRamOffset + 0x4AF4]);
 memcpyBigEndian16(&_state.guardPositionY,    &(*_stateData)[*_stateWorkRamOffset + 0x4AF6]);
}

uint64_t blastemInstance::computeHash()
{
  MetroHash64 hash;

  hash.Update(&_state.currentFrame, sizeof(uint16_t));
  hash.Update(&_state.framesPerStep, sizeof(uint8_t));
  hash.Update(&_state.currentLevel, sizeof(uint8_t));
  hash.Update(&_state.drawnRoom, sizeof(uint8_t));
  hash.Update(&_state.minutesLeft, sizeof(uint16_t));
  hash.Update(&_state.twelthSecondsLeft, sizeof(uint16_t));
  hash.Update(&_state.checkpointPointer, sizeof(uint32_t));
  hash.Update(&_state.slowfallFramesLeft, sizeof(uint8_t));

  hash.Update(&_state.kidFrame, sizeof(uint8_t));
  hash.Update(&_state.kidCurrentHP, sizeof(uint8_t));
  hash.Update(&_state.kidMaxHP, sizeof(uint8_t));
  hash.Update(&_state.kidRoom, sizeof(uint8_t));
  hash.Update(&_state.kidHasSword, sizeof(uint8_t));
  hash.Update(&_state.kidDirection, sizeof(uint8_t));
  hash.Update(&_state.kidPositionX, sizeof(uint16_t));
  hash.Update(&_state.kidPositionY, sizeof(uint16_t));

  hash.Update(&_state.guardFrame, sizeof(uint8_t));
  hash.Update(&_state.guardCurrentHP, sizeof(uint8_t));
  hash.Update(&_state.guardMaxHP, sizeof(uint8_t));
  hash.Update(&_state.guardRoom, sizeof(uint8_t));
  hash.Update(&_state.guardDirection, sizeof(uint8_t));
  hash.Update(&_state.guardPositionX, sizeof(uint16_t));
  hash.Update(&_state.guardPositionY, sizeof(uint16_t));

  uint64_t result;
  hash.Finalize(reinterpret_cast<uint8_t *>(&result));
  return result;
}

void blastemInstance::printState()
{
 printf("General Info:\n");
 printf(" + Savestate Size: %lu\n", *_stateSize);
 printf(" + Hash: 0x%lX\n", computeHash());

 printf("Game State:\n");
 printf(" + Current Frame: %d\n", _state.currentFrame);
 printf(" + Video Frames Per Game Frame: %d\n", _state.framesPerStep);
 printf(" + Current Level: %d\n", _state.currentLevel);
 printf(" + Drawn Room: %d\n", _state.drawnRoom);
 printf(" + Checkpoint Pointer: 0x%X\n", _state.checkpointPointer);
 printf(" + Minutes Left: %d\n", _state.minutesLeft - 1);
 printf(" + Seconds Left: %d\n", _state.twelthSecondsLeft / 12 );
 printf(" + Slowfall Frames Left: %d\n", _state.slowfallFramesLeft);

 printf("Kid State:\n");
 printf(" + Frame: %d\n", _state.kidFrame);
 printf(" + HP: %d/%d\n", _state.kidCurrentHP, _state.kidMaxHP);
 printf(" + Room: %d\n", _state.kidRoom);
 printf(" + Has Sword: %s\n", _state.kidHasSword == 255 ? "No" : "Yes");
 printf(" + Direction: %s\n", _state.kidDirection == 255 ? "Left" : "Right");
 printf(" + Position X: %d\n", _state.kidPositionX);
 printf(" + Position Y: %d\n", _state.kidPositionY);

 printf("Guard State:\n");
 printf(" + Frame: %d\n", _state.guardFrame);
 printf(" + HP: %d/%d\n", _state.guardCurrentHP, _state.guardMaxHP);
 printf(" + Room: %d\n", _state.guardRoom);
 printf(" + Direction: %s\n", _state.guardDirection == 255 ? "Left" : "Right");
 printf(" + Position X: %d\n", _state.guardPositionX);
 printf(" + Position Y: %d\n", _state.guardPositionY);
}

void blastemInstance::playFrame(const std::string& move)
{
 strcpy(*_nextMove, move.c_str());
 resume();
 updateState();
}

void blastemInstance::loadState(const uint8_t* state)
{
 memcpy(*_stateData, state, *_stateSize);
 reloadState();
}

void blastemInstance::saveState(uint8_t* state)
{
 memcpy(state, *_stateData, *_stateSize);
}

