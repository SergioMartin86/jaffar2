#include "jaffarInject.h"
#include "vdp.h"

uint8_t _framesPerGameFrame;
uint16_t _curFrameId = 0;
uint16_t _gameFrameId = 0;
size_t _stateSize = 0;
uint8_t* _stateData;
size_t _stateWorkRamOffset = 0;
move_t _nextMove;
m68k_context* _context;
uint16_t lastExitFrame = 9999;

void jaffarInject(m68k_context *context)
{
 _context = context;
 updateFrameInfo();
 if (lastExitFrame != _curFrameId)
 if ((_curFrameId == _gameFrameId + 3) || _gameFrameId == 0)
 {
  lastExitFrame = _curFrameId;
  request_exit(current_system);
 }
}

void updateFrameInfo()
{
 _framesPerGameFrame = m68k_read_byte(_context, 0x00FF5005);
 *(((uint8_t*)&_gameFrameId)+1) = m68k_read_byte(_context, 0x00FF5002);
 *(((uint8_t*)&_gameFrameId)+0) = m68k_read_byte(_context, 0x00FF5003);
 *(((uint8_t*)&_curFrameId)+1) = m68k_read_byte(_context, 0x00FF19C8);
 *(((uint8_t*)&_curFrameId)+0) = m68k_read_byte(_context, 0x00FF19C9);
}

void reloadState()
{
 genesis_context *gen = current_system;
 deserialize_buffer state;
 init_deserialize(&state, _stateData, _stateSize);
 genesis_deserialize(&state, gen);
 updateFrameInfo();
 lastExitFrame = _curFrameId;
}

void finalize()
{
 system_request_exit(current_system, 1);
 SDL_Quit();
}

void redraw()
{
 genesis_context *gen = current_system;
 vdp_context * v_context = gen->vdp;
 vdp_run_to_vblank(v_context);
 vdp_print_sprite_table(v_context);
}
