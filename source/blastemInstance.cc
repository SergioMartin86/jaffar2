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
  _finalize = (finalize_t) dlsym(_dllHandle, "finalize");
  _redraw = (redraw_t) dlsym(_dllHandle, "redraw");
  reloadState = (reloadState_t) dlsym(_dllHandle, "reloadState");
  _stateData = (uint8_t**) dlsym(_dllHandle, "_stateData");
  _stateSize = (size_t*) dlsym(_dllHandle, "_stateSize");
  _stateWorkRamOffset = (size_t*) dlsym(_dllHandle, "_stateWorkRamOffset");
  _nextMove = (move_t*) dlsym(_dllHandle, "_nextMove");
}

void blastemInstance::initialize(char* romFile, char* saveFile, const bool headlessMode)
{
 int argc = 4;
 char* argv[4];

 char exec[] = "jaffar";
 char flag[] = "-s";
 argv[0] = exec;
 argv[1] = romFile;
 argv[2] = flag;
 argv[3] = saveFile;
 start(argc, argv, headlessMode);
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
 printf("[Jaffar2]  + Current Level: %2d\n", _state.currentLevel);
 printf("[Jaffar2]  + Current Frame: %d\n", _state.currentFrame);
// printf("[Jaffar]  + IGT: %2lu:%02lu.%03lu\n", getElapsedMins(), getElapsedSecs(), getElapsedMilisecs());
 printf("[Jaffar2]  + [Kid]   Room: %d, Pos.x: %3d, Pos.y: %3d, Frame: %3d, Direction: %s, HP: %d/%d\n", _state.kidRoom, _state.kidPositionX, _state.kidPositionY, _state.kidFrame, _state.kidDirection == 255 ? "L" : "R", _state.kidCurrentHP, _state.kidMaxHP);
 printf("[Jaffar2]  + [Guard] Room: %d, Pos.x: %3d, Pos.y: %3d, Frame: %3d, Direction: %s, HP: %d/%d\n", _state.guardRoom, _state.guardPositionX, _state.guardPositionY, _state.guardFrame, _state.guardDirection == 255 ? "L" : "R", _state.guardCurrentHP, _state.guardMaxHP);
// printf("[Jaffar]  + Exit Room Timer: %d\n", *exit_room_timer);
// printf("[Jaffar]  + Exit Door Open: %s\n", isLevelExitDoorOpen() ? "Yes" : "No");
// printf("[Jaffar]  + Reached Checkpoint: 0x%X\n", _state.checkpointPointer);
// printf("[Jaffar]  + Feather Fall: %d\n", _state.slowfallFramesLeft);
// printf("[Jaffar]  + RNG State: 0x%08X (Last Loose Tile Sound Id: %d)\n", *random_seed, *last_loose_sound);


// printf("General Info:\n");
// printf(" + Savestate Size: %lu\n", *_stateSize);
// printf(" + Hash: 0x%lX\n", computeHash());
//
// printf("Game State:\n");
// printf(" + Current Frame: %d\n", _state.currentFrame);
// printf(" + Video Frames Per Game Frame: %d\n", _state.framesPerStep);
// printf(" + Current Level: %d\n", _state.currentLevel);
// printf(" + Drawn Room: %d\n", _state.drawnRoom);
// printf(" + Checkpoint Pointer: 0x%X\n", _state.checkpointPointer);
//// printf(" + Minutes Left: %d\n", _state.minutesLeft - 1);
//// printf(" + Seconds Left: %d\n", _state.twelthSecondsLeft / 12 );
// printf(" + Slowfall Frames Left: %d\n", _state.slowfallFramesLeft);
//
// printf("Kid State:\n");
// printf(" + Frame: %d\n", _state.kidFrame);
// printf(" + HP: %d/%d\n", _state.kidCurrentHP, _state.kidMaxHP);
// printf(" + Room: %d\n", _state.kidRoom);
// printf(" + Has Sword: %s\n", _state.kidHasSword == 255 ? "No" : "Yes");
// printf(" + Direction: %s\n", _state.kidDirection == 255 ? "Left" : "Right");
// printf(" + Position X: %d\n", _state.kidPositionX);
// printf(" + Position Y: %d\n", _state.kidPositionY);
//
// printf("Guard State:\n");
// printf(" + Frame: %d\n", _state.guardFrame);
// printf(" + HP: %d/%d\n", _state.guardCurrentHP, _state.guardMaxHP);
// printf(" + Room: %d\n", _state.guardRoom);
// printf(" + Direction: %s\n", _state.guardDirection == 255 ? "Left" : "Right");
// printf(" + Position X: %d\n", _state.guardPositionX);
// printf(" + Position Y: %d\n", _state.guardPositionY);
}


std::vector<uint8_t> blastemInstance::getPossibleMoveIds()
{
  // Move Ids =         0    1    2    3    4    5     6     7     8    9     10    11    12    13   14    15   16   17
  //_possibleMoves = { ".", "B", "A", "L", "R", "D", "LA", "LD", "RA", "RD", "BR", "BL", "BU", "BD", "C", "BU", "U", "S" };
  // A = Jump
  // B = Hold / Careful Step
  // C = Attack
  // S = Start
  //return {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

  if (_state.kidFrame == 1) return {0, 3, 4, 5}; // Running
  if (_state.kidFrame == 2) return {0, 3, 4, 5}; // Running
  if (_state.kidFrame == 3) return {0, 3, 4, 5}; // Running
  if (_state.kidFrame == 4) return {0, 3, 4, 5}; // Running
  if (_state.kidFrame == 5) return {0, 3, 4, 5}; // Running
  if (_state.kidFrame == 6) return {0, 3, 4, 5}; // Running
  if (_state.kidFrame == 7) return {0, 3, 4, 5, 6, 8}; // Running + Can Jump
  if (_state.kidFrame == 8) return {0, 3, 4, 5}; // Running
  if (_state.kidFrame == 9) return {0, 3, 4, 5}; // Running
  if (_state.kidFrame == 10) return {0, 3, 4, 5}; // Running
  if (_state.kidFrame == 11) return {0, 3, 4, 5, 6, 8}; // Running + Can Jump
  if (_state.kidFrame == 12) return {0, 3, 4, 5}; // Running
  if (_state.kidFrame == 13) return {0, 3, 4, 5}; // Running
  if (_state.kidFrame == 14) return {0, 3, 4, 5}; // Running
  if (_state.kidFrame == 15) return {0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 14}; // Standing
  if (_state.kidFrame == 16) return {0}; // Standing Jump
  if (_state.kidFrame == 17) return {0}; // Standing Jump
  if (_state.kidFrame == 18) return {0}; // Standing Jump
  if (_state.kidFrame == 19) return {0}; // Standing Jump
  if (_state.kidFrame == 20) return {0}; // Standing Jump
  if (_state.kidFrame == 21) return {0}; // Standing Jump
  if (_state.kidFrame == 22) return {0}; // Standing Jump
  if (_state.kidFrame == 23) return {0}; // Standing Jump
  if (_state.kidFrame == 24) return {0}; // Standing Jump
  if (_state.kidFrame == 25) return {0}; // Standing Jump
  if (_state.kidFrame == 26) return {0}; // Standing Jump
  if (_state.kidFrame == 27) return {0}; // Standing Jump
  if (_state.kidFrame == 28) return {0}; // Standing Jump
  if (_state.kidFrame == 29) return {0}; // Standing Jump
  if (_state.kidFrame == 30) return {0}; // Standing Jump
  if (_state.kidFrame == 31) return {0}; // Standing Jump
  if (_state.kidFrame == 32) return {0}; // Standing Jump
  if (_state.kidFrame == 33) return {0}; // Standing Jump
  if (_state.kidFrame == 34) return {0}; // Running Jump
  if (_state.kidFrame == 35) return {0}; // Running Jump
  if (_state.kidFrame == 36) return {0}; // Running Jump
  if (_state.kidFrame == 37) return {0}; // Running Jump
  if (_state.kidFrame == 38) return {0}; // Running Jump
  if (_state.kidFrame == 39) return {0}; // Running Jump
  if (_state.kidFrame == 40) return {0}; // Running Jump
  if (_state.kidFrame == 41) return {0}; // Running Jump
  if (_state.kidFrame == 42) return {0}; // Running Jump
  if (_state.kidFrame == 43) return {0}; // Post-Jump
  if (_state.kidFrame == 44) return {0}; // Post-Jump
  if (_state.kidFrame == 45) return {0}; // Turning
  if (_state.kidFrame == 46) return {0}; // Turning
  if (_state.kidFrame == 47) return {0}; // Turning
  if (_state.kidFrame == 48) return {0}; // Turning
  if (_state.kidFrame == 49) return {0}; // Stopping after run / Recovering
  if (_state.kidFrame == 50) return {0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 14}; // Stopping after run / Recovering (Can act now)
  if (_state.kidFrame == 51) return {0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 14}; // Stopping after run / Recovering (Can act now)
  if (_state.kidFrame == 52) return {0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 14}; // Stopping after run / Recovering (Can act now)
  if (_state.kidFrame == 53) return {0}; // Stopping after run
  if (_state.kidFrame == 54) return {0}; // Stopping after run
  if (_state.kidFrame == 55) return {0}; // Stopping after run
  if (_state.kidFrame == 56) return {0}; // Stopping after run
  if (_state.kidFrame == 57) return {0}; // Running Turn
  if (_state.kidFrame == 58) return {0}; // Running Turn
  if (_state.kidFrame == 59) return {0}; // Running Turn
  if (_state.kidFrame == 60) return {0}; // Running Turn
  if (_state.kidFrame == 61) return {0}; // Running Turn
  if (_state.kidFrame == 62) return {0}; // Running Turn
  if (_state.kidFrame == 63) return {0}; // Running Turn
  if (_state.kidFrame == 64) return {0}; // Running Turn
  if (_state.kidFrame == 65) return {0}; // Running Turn
  if (_state.kidFrame == 67) return {0}; // Upwards Jump / Climbing
  if (_state.kidFrame == 68) return {0}; // Upwards Jump / Climbing
  if (_state.kidFrame == 69) return {0}; // Upwards Jump / Climbing
  if (_state.kidFrame == 70) return {0}; // Upwards Jump / Climbing
  if (_state.kidFrame == 71) return {0}; // Upwards Jump / Climbing
  if (_state.kidFrame == 72) return {0}; // Upwards Jump / Climbing
  if (_state.kidFrame == 73) return {0}; // Upwards Jump / Climbing
  if (_state.kidFrame == 74) return {0}; // Upwards Jump / Climbing
  if (_state.kidFrame == 75) return {0}; // Upwards Jump / Climbing
  if (_state.kidFrame == 76) return {0}; // Upwards Jump / Climbing
  if (_state.kidFrame == 77) return {0}; // Upwards Jump / Climbing
  if (_state.kidFrame == 78) return {0}; // Upwards Jump / Climbing
  if (_state.kidFrame == 79) return {0, 1, 12}; // Upwards Jump / Climbing (Can Grab)
  if (_state.kidFrame == 80) return {0, 1, 12}; // Upwards Jump / Climbing (Can Grab)
  if (_state.kidFrame == 81) return {0}; // Let go after grab
  if (_state.kidFrame == 86) return {0}; // Hesitant Careful Step (before a ledge)
  if (_state.kidFrame == 91) return {0, 1, 12}; // Grabbing ledge, can go up
  if (_state.kidFrame == 92) return {0, 1, 12}; // Grabbing ledge, can go up
  if (_state.kidFrame == 93) return {0, 1, 12}; // Grabbing ledge, can go up
  if (_state.kidFrame == 94) return {0, 1, 12}; // Grabbing ledge, can go up
  if (_state.kidFrame == 102) return {0, 1}; // Falling
  if (_state.kidFrame == 103) return {0, 1}; // Falling
  if (_state.kidFrame == 104) return {0, 1}; // Falling
  if (_state.kidFrame == 105) return {0, 1}; // Falling
  if (_state.kidFrame == 106) return {0, 1}; // Falling
  if (_state.kidFrame == 107) return {0}; // Pre-Crouching
  if (_state.kidFrame == 108) return {0}; // Pre-Crouching
  if (_state.kidFrame == 109) return {0, 7, 9}; // Crouching
  if (_state.kidFrame == 110) return {0}; // Post-Crouching
  if (_state.kidFrame == 111) return {0}; // Post-Crouching
  if (_state.kidFrame == 111) return {0}; // Post-Crouching
  if (_state.kidFrame == 112) return {0}; // Post-Crouching
  if (_state.kidFrame == 113) return {0}; // Post-Crouching
  if (_state.kidFrame == 114) return {0}; // Post-Crouching
  if (_state.kidFrame == 115) return {0}; // Post-Crouching
  if (_state.kidFrame == 116) return {0}; // Post-Crouching
  if (_state.kidFrame == 117) return {0}; // Post-Crouching
  if (_state.kidFrame == 118) return {0}; // Post-Crouching
  if (_state.kidFrame == 119) return {0}; // Post-Crouching
  if (_state.kidFrame == 121) return {0}; // Careful Step
  if (_state.kidFrame == 122) return {0}; // Careful Step
  if (_state.kidFrame == 123) return {0}; // Careful Step
  if (_state.kidFrame == 124) return {0}; // Careful Step
  if (_state.kidFrame == 125) return {0}; // Careful Step
  if (_state.kidFrame == 126) return {0}; // Careful Step
  if (_state.kidFrame == 127) return {0}; // Careful Step
  if (_state.kidFrame == 128) return {0}; // Careful Step
  if (_state.kidFrame == 129) return {0}; // Careful Step
  if (_state.kidFrame == 130) return {0}; // Careful Step
  if (_state.kidFrame == 131) return {0}; // Careful Step
  if (_state.kidFrame == 132) return {0}; // Careful Step
  if (_state.kidFrame == 133) return {0}; // [Sword] Final Sheathing Sword
  if (_state.kidFrame == 134) return {0}; // [Sword] Final Sheathing Sword
  if (_state.kidFrame == 135) return {0}; // Climbing Up
  if (_state.kidFrame == 136) return {0}; // Climbing Down/Up
  if (_state.kidFrame == 137) return {0}; // Climbing Up
  if (_state.kidFrame == 138) return {0}; // Climbing Down/Up
  if (_state.kidFrame == 139) return {0}; // Climbing Up
  if (_state.kidFrame == 140) return {0}; // Climbing Down/Up
  if (_state.kidFrame == 141) return {0}; // Climbing Down/Up
  if (_state.kidFrame == 142) return {0}; // Climbing Down/Up
  if (_state.kidFrame == 143) return {0}; // Climbing Down/Up
  if (_state.kidFrame == 144) return {0}; // Climbing Down/Up
  if (_state.kidFrame == 145) return {0}; // Climbing Down/Up
  if (_state.kidFrame == 146) return {0}; // Climbing Up
  if (_state.kidFrame == 147) return {0}; // Climbing Up
  if (_state.kidFrame == 148) return {0}; // Climbing Down/Up
  if (_state.kidFrame == 149) return {0}; // Climbing Up
  if (_state.kidFrame == 150) return {0, 14}; // [Sword] Parrying 2 - Can Attack
  if (_state.kidFrame == 151) return {0, 2}; // [Sword] Attack
  if (_state.kidFrame == 152) return {0, 2}; // [Sword] Attack
  if (_state.kidFrame == 153) return {0, 2}; // [Sword] Attack
  if (_state.kidFrame == 154) return {0, 2}; // [Sword] Attack
  if (_state.kidFrame == 155) return {0, 2}; // [Sword] Attack
  if (_state.kidFrame == 156) return {0, 2}; // [Sword] After Attack / Recovering from Hit
  if (_state.kidFrame == 157) return {0, 3, 4, 5, 14, 16}; // [Sword] After Attack / Recovering from Hit
  if (_state.kidFrame == 158) return {0, 3, 4, 5, 14, 16}; // [Sword] En Guarde
  if (_state.kidFrame == 160) return {0}; // [Sword] Walk Backward 2
  if (_state.kidFrame == 161) return {0, 14, 16}; // [Sword] Attack While parrying
  if (_state.kidFrame == 162) return {0}; // [Sword] Attack While parrying
  if (_state.kidFrame == 163) return {0}; // [Sword] Walk Forward
  if (_state.kidFrame == 164) return {0}; // [Sword] Walk Forward
  if (_state.kidFrame == 165) return {0, 3, 4, 5, 14, 16}; // [Sword] Walk Forward
  if (_state.kidFrame == 169) return {0}; // [Sword] Parrying 1
  if (_state.kidFrame == 170) return {0, 3, 4, 5, 14, 16}; // [Sword] En Guarde
  if (_state.kidFrame == 171) return {0, 3, 4, 5, 14, 16}; // [Sword] En Guarde
  if (_state.kidFrame == 172) return {0}; // [Sword] Getting Hit
  if (_state.kidFrame == 173) return {0}; // [Sword] Getting Hit
  if (_state.kidFrame == 174) return {0, 3, 4, 5, 14, 16}; // [Sword] Getting Hit
  if (_state.kidFrame == 177) return {0}; // [Sword] Turning
  if (_state.kidFrame == 178) return {0}; // [Sword] Turning
  if (_state.kidFrame == 179) return {0}; // [Sword] Dying
  if (_state.kidFrame == 180) return {0}; // [Sword] Dying
  if (_state.kidFrame == 181) return {0}; // [Sword] Dying
  if (_state.kidFrame == 182) return {0}; // [Sword] Dying
  if (_state.kidFrame == 183) return {0}; // [Sword] Dying
  if (_state.kidFrame == 185) return {0}; // [Sword] Dying
  if (_state.kidFrame == 207) return {0}; // [Sword] Drawing Sword
  if (_state.kidFrame == 208) return {0}; // [Sword] Drawing Sword
  if (_state.kidFrame == 209) return {0}; // [Sword] Drawing Sword
  if (_state.kidFrame == 210) return {0}; // [Sword] Drawing Sword
  if (_state.kidFrame == 211) return {0}; // [Sword] Turning
  if (_state.kidFrame == 212) return {0}; // [Sword] Turning
  if (_state.kidFrame == 213) return {0}; // [Sword] Turning
  if (_state.kidFrame == 233) return {0}; // [Sword] Sheathing Sword
  if (_state.kidFrame == 234) return {0}; // [Sword] Sheathing Sword
  if (_state.kidFrame == 235) return {0}; // [Sword] Sheathing Sword
  if (_state.kidFrame == 236) return {0}; // [Sword] Sheathing Sword
  if (_state.kidFrame == 237) return {0}; // [Sword] Sheathing Sword
  if (_state.kidFrame == 238) return {0}; // [Sword] Sheathing Sword
  if (_state.kidFrame == 239) return {0}; // [Sword] Sheathing Sword
  if (_state.kidFrame == 240) return {0}; // [Sword] Sheathing Sword

  // Default, report unrecognized frame
  EXIT_WITH_ERROR("[Jaffar2] Warning: Frame %d not recognized.\n", _state.kidFrame);
  return {0};
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

void blastemInstance::redraw()
{
 _redraw();
}

void blastemInstance::finalize()
{
 _finalize();
 dlclose(_dllHandle);
}

