#include "jaffarInject.h"

uint16_t curFrameId = 0;
uint16_t prevFrameId = 0;
size_t _stateSize = 0;
uint8_t* _stateData;
size_t _stateWorkRamOffset = 0;

void jaffarInject(m68k_context *context)
{
 uint8_t framesPerGameFrame = m68k_read_byte(context, 0x00FF5005);

 *(((uint8_t*)&curFrameId)+1) = m68k_read_byte(context, 0x00FF19C8);
 *(((uint8_t*)&curFrameId)+0) = m68k_read_byte(context, 0x00FF19C9);
 if (curFrameId >= prevFrameId + framesPerGameFrame)
 {
  if (_stateSize > 0) free(_stateData);
  _stateData = soft_serialize(current_system, &_stateSize);

  prevFrameId = curFrameId;
  co_switch(_jaffarThread);
  soft_deserialize(current_system, _stateData, _stateSize);
 }
}


