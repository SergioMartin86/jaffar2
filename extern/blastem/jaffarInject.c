#include "jaffarInject.h"
#include "vdp.h"
#include "libco.h"
#include <time.h>

uint8_t _framesPerGameFrame;
uint16_t _curFrameId = 0;
uint16_t _gameFrameId = 0;
size_t _stateSize = 0;
uint8_t* _stateData;
size_t _stateWorkRamOffset = 0;
move_t _nextMove;
m68k_context* _context;
uint16_t lastExitFrame = 9999;
size_t injectCalls = 0;
int _detectedError = 0;
time_t _startTime = 999999999999;

#define TIMEOUT_SECONDS 3
#define MAX_INJECTS 1000

void jaffarInject(m68k_context *context)
{
 _context = context;
 updateFrameInfo();
 injectCalls++;

 if (injectCalls > MAX_INJECTS) printf("Warning: too many inject calls: %lu\n", injectCalls);

 if (lastExitFrame != _curFrameId)
 if ((_curFrameId == _gameFrameId + 3) || _gameFrameId == 0 || injectCalls > MAX_INJECTS)
 {
  lastExitFrame = _curFrameId;
  request_exit(current_system);
  injectCalls = 0;
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

 gen->reset_requested = 0;
 gen->m68k->should_return = 0;
 z80_assert_reset(gen->z80, gen->m68k->current_cycle);
 z80_clear_busreq(gen->z80, gen->m68k->current_cycle);
 ym_reset(gen->ym);
 //Is there any sort of VDP reset?
// m68k_reset(gen->m68k);

 deserialize_buffer state;
 init_deserialize(&state, _stateData, _stateSize);
 genesis_deserialize(&state, gen);
 updateFrameInfo();
 lastExitFrame = _curFrameId;

 if (!headless)
 {
  vdp_context * v_context = gen->vdp;
  vdp_run_to_vblank(v_context);
  vdp_print_sprite_table(v_context);
 }

// gen->header.delayed_load_slot = 0;
// resume_68k(gen->m68k);
}

void finalize()
{
 system_request_exit(current_system, 1);
 current_system->free_context(current_system);
 SDL_Quit();
}

void redraw()
{
 genesis_context *gen = current_system;
 vdp_context * v_context = gen->vdp;
 vdp_run_to_vblank(v_context);
 vdp_print_sprite_table(v_context);
}

void restartGenesis()
{
 genesis_context *gen = current_system;

 if (fast_vdp)
 {
  gen->reset_requested = 0;
  gen->header.force_release = 1;
  handle_reset_requests(gen);
 }
 request_exit(current_system);
 resume_68k(gen->m68k);
}

cothread_t _jaffarThread;
cothread_t _blastemThread;

extern int blastemMain(int argc, char** argv);
extern void init_system_with_media(const char *path, system_type force_stype);
void handleError(const char* errorMessage)
{
 printf("Error detected: %s\n", errorMessage);
 fflush(stdout);
 _gameFrameId = 0;
 lastExitFrame = 9999;
 _detectedError = 1;
 co_switch(_jaffarThread);
}

void routineWrapper()
{
 resume_genesis(current_system);
 co_switch(_jaffarThread);
}

void blastem_start(int argc, char** argv, int isHeadlessMode, int isFastVdp)
{
 headless = isHeadlessMode;
 fast_vdp = isFastVdp;
 blastemMain(argc, argv);
}

void blastem_resume()
{
 _startTime = time(NULL);
 _jaffarThread = co_active();
 _blastemThread = co_create(1 << 24, routineWrapper);
 co_switch(_blastemThread);
 co_delete(_blastemThread);
}

void checkTimeout()
{
 if (!headless) return;
 genesis_context *gen = current_system;
 time_t curTime = time(NULL);
 if (curTime >= _startTime + TIMEOUT_SECONDS)
  handleError("Timeout found");
}
