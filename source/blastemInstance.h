#pragma once

#include <string>

typedef void (*start_t)(int, char**);
typedef void (*resume_t)(void);

struct gameStateStruct
{
 uint16_t currentFrame;
 uint8_t framesPerStep;
 uint8_t currentLevel;
 uint8_t drawnRoom;
 uint16_t minutesLeft;
 uint16_t twelthSecondsLeft;
 uint8_t slowfallFramesLeft;

 // This is a pointer to the genesis bus to a position that gets modified when you reach the checkpoint
 // It's zero before you reach a checkpoint, but changes when you hit the first one.
 // When hitting a second checkpoint, the pointer doesn't change but instead the position in memory that it is pointing to
 // A system needs to be established to keep track of these changes to know what checkpoint it refers to
 uint32_t checkpointPointer;

 uint8_t kidFrame;
 uint8_t kidCurrentHP;
 uint8_t kidMaxHP;
 uint8_t kidRoom;
 uint8_t kidDirection;
 uint16_t kidPositionX;
 uint16_t kidPositionY;
 uint8_t kidHasSword;

 uint8_t guardFrame;
 uint8_t guardCurrentHP;
 uint8_t guardMaxHP;
 uint8_t guardRoom;
 uint8_t guardDirection;
 uint16_t guardPositionX;
 uint16_t guardPositionY;
};

class blastemInstance
{
  public:
  blastemInstance(int argc, char** argv, const char* libraryFile, const bool multipleLibraries);
  void playFrame();
  void updateState();
  void printState();
  ~blastemInstance();

  // blastem Functions
  start_t start;
  resume_t resume;

  // State
  gameStateStruct _state;
  uint8_t** _stateData;
  size_t* _stateSize;
  size_t* _stateWorkRamOffset;

  private:

  void memcpyBigEndian8(uint8_t* dst, uint8_t* src) { ((uint8_t*)dst)[0] = src[0]; }
  void memcpyBigEndian16(uint16_t* dst, uint8_t* src) { ((uint8_t*)dst)[0] = src[1]; ((uint8_t*)dst)[1] = src[0]; }
  void memcpyBigEndian32(uint32_t* dst, uint8_t* src) { ((uint8_t*)dst)[0] = src[3]; ((uint8_t*)dst)[1] = src[2]; ((uint8_t*)dst)[2] = src[1]; ((uint8_t*)dst)[3] = src[0]; }

  void *_dllHandle;
};
