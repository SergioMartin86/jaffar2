#include "serialize.h"
#include "io.h"
#include "blastem.h"
#include "render.h"
#include "util.h"
#include "bindings.h"
#include "m68k_core.h"
#include "genesis.h"

#define MAX_MOVE_SIZE 4
typedef char move_t[MAX_MOVE_SIZE];

extern uint16_t curFrameId;
extern uint16_t prevFrameId;
extern size_t _stateWorkRamOffset;
extern move_t _nextMove;

void jaffarInject(m68k_context *context);
