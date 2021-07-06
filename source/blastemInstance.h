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

 // if (property == "Next Level") return sdlPop->next_level;
 // if (property == "Drawn Room") return sdlPop->drawn_room;
 // if (property == "Exit Door Open") return &sdlPop->isExitDoorOpen;
 // if (property == "Checkpoint Reached") return sdlPop->checkpoint;
 // if (property == "Is Feather Fall") return sdlPop->is_feather_fall;
 // if (property == "Current Step") return &_currentStep;

 uint8_t kidFrame;
 uint8_t kidCurrentHP;
 uint8_t kidMaxHP;
 uint16_t kidRoom;
 uint16_t kidPositionX;
 uint16_t kidPositionY;

 // if (property == "Kid Direction") return &sdlPop->Kid->direction;
 // if (property == "Kid Current Column") return &sdlPop->Kid->curr_col;
 // if (property == "Kid Current Row") return &sdlPop->Kid->curr_row;
 // if (property == "Kid Action") return &sdlPop->Kid->action;
 // if (property == "Kid Fall Velocity X") return &sdlPop->Kid->fall_x;
 // if (property == "Kid Fall Velocity Y") return &sdlPop->Kid->fall_y;
 // if (property == "Kid Repeat") return &sdlPop->Kid->repeat;
 // if (property == "Kid Character Id") return &sdlPop->Kid->charid;
 // if (property == "Kid Has Sword") return &sdlPop->Kid->sword;
 // if (property == "Kid Is Alive") return &sdlPop->Kid->alive;
 // if (property == "Kid Current Sequence") return &sdlPop->Kid->curr_seq;

 uint8_t guardFrame;
 uint8_t guardCurrentHP;
 uint8_t guardMaxHP;
 uint16_t guardRoom;
 uint16_t guardPositionX;
 uint16_t guardPositionY;

// if (property == "Guard Direction") return &sdlPop->Guard->direction;
// if (property == "Guard Current Column") return &sdlPop->Guard->curr_col;
// if (property == "Guard Current Row") return &sdlPop->Guard->curr_row;
// if (property == "Guard Action") return &sdlPop->Guard->action;
// if (property == "Guard Fall Velocity X") return &sdlPop->Guard->fall_x;
// if (property == "Guard Fall Velocity Y") return &sdlPop->Guard->fall_y;
// if (property == "Guard Repeat") return &sdlPop->Guard->repeat;
// if (property == "Guard Character Id") return &sdlPop->Guard->charid;
// if (property == "Guard Has Sword") return &sdlPop->Guard->sword;
// if (property == "Guard Is Alive") return &sdlPop->Guard->alive;
// if (property == "Guard Current Sequence") return &sdlPop->Guard->curr_seq;

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
