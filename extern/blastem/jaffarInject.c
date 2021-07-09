#include "jaffarInject.h"

uint8_t _framesPerGameFrame;
uint16_t _curFrameId = 0;
uint16_t _prevFrameId = 0;
size_t _stateSize = 0;
uint8_t* _stateData;
size_t _stateWorkRamOffset = 0;
move_t _nextMove;
m68k_context* _context;

void jaffarInject(m68k_context *context)
{
 _context = context;
 updateFrameInfo();
 if (_curFrameId >= _prevFrameId + _framesPerGameFrame)
 {
  if (_stateSize > 0) free(_stateData);
  _stateData = soft_serialize(current_system, &_stateSize);

  _prevFrameId = _curFrameId;
  co_switch(_jaffarThread);
  printf("Next Move: %s\n", _nextMove);
 }
}

void updateFrameInfo()
{
 _framesPerGameFrame = m68k_read_byte(_context, 0x00FF5005);
 *(((uint8_t*)&_curFrameId)+1) = m68k_read_byte(_context, 0x00FF19C8);
 *(((uint8_t*)&_curFrameId)+0) = m68k_read_byte(_context, 0x00FF19C9);
}

void reloadState()
{
 soft_deserialize(current_system, _stateData, _stateSize);
 updateFrameInfo();
 _prevFrameId = 0;
}
