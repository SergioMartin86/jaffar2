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

void jaffarInject(m68k_context *context);
uint8_t *soft_serialize(system_header *sys, size_t *size_out);
void soft_deserialize(system_header *sys, uint8_t *data, size_t size);
