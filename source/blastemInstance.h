#pragma once

#include <string>
#include <vector>

#define _STATE_DATA_SIZE 141601
#define MAX_MOVE_SIZE 4
typedef char move_t[MAX_MOVE_SIZE];

extern const std::vector<std::string> _possibleMoves;

struct gameStateStruct
{
 uint32_t rngValue;
 uint16_t gameFrame;
 uint16_t videoFrame;
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

 uint8_t kidCurrentSequenceStage;
 uint8_t kidCurrentSequence;
 uint8_t kidLastSequence;
 uint8_t kidFrame;
 uint8_t kidCurrentHP;
 uint8_t kidMaxHP;
 uint8_t kidRoom;
 uint8_t kidDirection;
 uint16_t kidPositionX;
 uint16_t kidPositionY;
 uint8_t kidHasSword;

 uint8_t sandTile1;
 uint8_t sandTile2;
 uint8_t sandTile3;
 uint8_t sandTile4;
 uint8_t sandTile5;
 uint8_t caveEntrancePos;
 uint8_t lvl3ExitDoor;

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
  void initialize(char* romFile, char* saveFile, const bool headlessMode, const bool fastVdp);
  void finalize();
  int playFrame(const std::string& move);
  gameStateStruct getGameState(const uint8_t* state);
  void printState();
  void redraw();
  uint64_t computeHash();
  void loadState(const uint8_t* state);
  void saveState(uint8_t* state);
  std::vector<uint8_t> getPossibleMoveIds(const gameStateStruct& gameState);
  void setRNGValue(const uint32_t& rngValue);
  void reset();

  // State
  gameStateStruct _state;
  std::string _saveFile;
  uint8_t* _startStateData;

  private:

  void memcpyBigEndian8(uint8_t* dst, uint8_t* src) { ((uint8_t*)dst)[0] = src[0]; }
  void memcpyBigEndian16(uint16_t* dst, uint8_t* src) { ((uint8_t*)dst)[0] = src[1]; ((uint8_t*)dst)[1] = src[0]; }
  void memcpyBigEndian32(uint32_t* dst, uint8_t* src) { ((uint8_t*)dst)[0] = src[3]; ((uint8_t*)dst)[1] = src[2]; ((uint8_t*)dst)[2] = src[1]; ((uint8_t*)dst)[3] = src[0]; }
};
