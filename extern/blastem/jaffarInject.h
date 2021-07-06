#include "serialize.h"
#include "io.h"
#include "blastem.h"
#include "render.h"
#include "util.h"
#include "bindings.h"
#include "m68k_core.h"
#include "genesis.h"

extern uint16_t curFrameId;
extern uint16_t prevFrameId;
extern size_t _stateWorkRamOffset;

void jaffarInject(m68k_context *context);
