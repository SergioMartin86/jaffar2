#pragma once

#include <string>
#include "system.h"

typedef system_header* (*alloc_config_system_t)(system_type stype, system_media *media, uint32_t opts, uint8_t force_region);
typedef void (*render_video_loop_t)(void);
typedef system_type (*detect_system_type_t)(system_media *media);
typedef int (*main_t)(int argc, char ** argv);

class blastemInstance
{
  public:
  blastemInstance(const char* libraryFile, const bool multipleLibraries);
  ~blastemInstance();
  void initialize();

  // blastem Functions
  alloc_config_system_t alloc_config_system;
  render_video_loop_t render_video_loop;
  detect_system_type_t detect_system_type;
  main_t main;

  // blastem variables
  system_header **current_system;
  system_media cart;

  private:

  uint16_t* process_smd_block(uint16_t *dst, uint8_t *src, size_t bytes);
  uint8_t is_smd_format(const char *filename, uint8_t *header);
  int load_smd_rom(FILE* f, void **buffer);
  void *_dllHandle;
};
