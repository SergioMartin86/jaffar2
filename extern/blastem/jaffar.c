#include "jaffar.h"

uint16_t curFrameId = 0;
uint16_t prevFrameId = 0;

void jaffarInject(m68k_context *context)
{
 uint8_t framesPerGameFrame = m68k_read_byte(context, 0x00FF5005);
 *(((uint8_t*)&curFrameId)+1) = m68k_read_byte(context, 0x00FF19C8);
 *(((uint8_t*)&curFrameId)+0) = m68k_read_byte(context, 0x00FF19C9);

 if (curFrameId >= prevFrameId + framesPerGameFrame)
 {
  uint16_t kidPositionX = 0;
  *(((uint8_t*)&kidPositionX)+1) = m68k_read_byte(context, 0x00FF4C3E);
  *(((uint8_t*)&kidPositionX)+0) = m68k_read_byte(context, 0x00FF4C3F);

  size_t size = 0;
  uint8_t* state = soft_serialize(current_system, &size);

  printf("Frame %d - Kid Position: X %d - State Size %lu\n", curFrameId, kidPositionX, size);
  prevFrameId = curFrameId;
  co_switch(_jaffarThread);
 }
}

uint8_t *soft_serialize(system_header *sys, size_t *size_out)
{
 genesis_context *gen = (genesis_context *)sys;
  serialize_buffer state;
  init_serialize(&state);
  uint32_t address = read_word(4, (void **)gen->m68k->mem_pointers, &gen->m68k->options->gen, gen->m68k) << 16;
  address |= read_word(6, (void **)gen->m68k->mem_pointers, &gen->m68k->options->gen, gen->m68k);

  start_section(&state, SECTION_68000);
  m68k_serialize(gen->m68k, address, &state);
  end_section(&state);

  start_section(&state, SECTION_Z80);
  z80_serialize(gen->z80, &state);
  end_section(&state);

  start_section(&state, SECTION_VDP);
  vdp_serialize(gen->vdp, &state);
  end_section(&state);

  start_section(&state, SECTION_YM2612);
  ym_serialize(gen->ym, &state);
  end_section(&state);

  start_section(&state, SECTION_PSG);
  psg_serialize(gen->psg, &state);
  end_section(&state);

  start_section(&state, SECTION_GEN_BUS_ARBITER);
  save_int8(&state, gen->z80->reset);
  save_int8(&state, gen->z80->busreq);
  save_int16(&state, gen->z80_bank_reg);
  end_section(&state);

  start_section(&state, SECTION_SEGA_IO_1);
  io_serialize(gen->io.ports, &state);
  end_section(&state);

  start_section(&state, SECTION_SEGA_IO_2);
  io_serialize(gen->io.ports + 1, &state);
  end_section(&state);

  start_section(&state, SECTION_SEGA_IO_EXT);
  io_serialize(gen->io.ports + 2, &state);
  end_section(&state);

  start_section(&state, SECTION_MAIN_RAM);
  save_int8(&state, RAM_WORDS * 2 / 1024);
  save_buffer16(&state, gen->work_ram, RAM_WORDS);
  end_section(&state);

  start_section(&state, SECTION_SOUND_RAM);
  save_int8(&state, Z80_RAM_BYTES / 1024);
  save_buffer8(&state, gen->zram, Z80_RAM_BYTES);
  end_section(&state);

  if (gen->version_reg & 0xF) {
   //only save TMSS info if it's present
   //that will allow a &state saved on a model lacking TMSS
   //to be loaded on a model that has it
   start_section(&state, SECTION_TMSS);
   save_int8(&state, gen->tmss);
   save_buffer16(&state, gen->tmss_lock, 2);
   end_section(&state);
  }

  cart_serialize(&gen->header, &state);

  *size_out = state.size;
  return state.data;
}

