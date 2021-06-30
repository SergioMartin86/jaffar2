#include "blastemInstance.h"
#include "utils.h"
#include <dlfcn.h>
#include <iostream>
#include <omp.h>

#define SMD_HEADER_SIZE 512
#define SMD_MAGIC1 0x03
#define SMD_MAGIC2 0xAA
#define SMD_MAGIC3 0xBB
#define SMD_BLOCK_SIZE 0x4000

void blastemInstance::initialize()
{
 // Parsing ROM path
 const char *romPath = std::getenv("JAFFAR2_ROM_PATH");
 if (romPath == NULL) EXIT_WITH_ERROR("[Jaffar] JAFFAR2_ROM_PATH environment variable not defined.\n");

// uint8_t header[10];
//
// FILE* f = fopen(romPath, "rb");
// if (f == NULL)
//  EXIT_WITH_ERROR("Error opening ROM %s\n", romPath);
//
// if (sizeof(header) != fread(header, 1, sizeof(header), f))
//  EXIT_WITH_ERROR("Error reading ROM %s\n", romPath);
//
// size_t filesize = 512 * 1024;
// size_t readsize = sizeof(header);
//
// char *buf = (char*) malloc(filesize);
// memcpy(buf, header, readsize);
//
// size_t read;
// do {
//  read = fread(buf + readsize, 1, filesize - readsize, f);
//  if (read > 0) {
//   readsize += read;
//   if (readsize == filesize) {
//    int one_more = fgetc(f);
//    if (one_more >= 0) {
//     filesize *= 2;
//     buf = (char*) realloc(buf, filesize);
//     buf[readsize++] = one_more;
//    } else {
//     read = 0;
//    }
//   }
//  }
// } while (read > 0);
//
// fclose(f);
//
//
// uint32_t opts = 0;
// uint8_t force_region = 0;
//
// cart.buffer = buf;
// cart.size = filesize;
// cart.dir = path_dirname(romPath);
// cart.name = basename_no_extension(romPath);
// cart.extension = path_extension(romPath);
// system_type stype = detect_system_type(&cart);
//
// //allocate new system context
// *current_system = alloc_config_system(stype, &cart, opts, force_region);
//
// if (*current_system == NULL) EXIT_WITH_ERROR("Failed to configure emulated machine for %s\n", romPath);
//
// // Running blastem
// (*current_system)->start_context(*current_system, "/home/martiser/jaffar2/examples/saves/lvl1.state");
//// render_video_loop();
}

blastemInstance::blastemInstance(const char* libraryFile, const bool multipleLibraries)
{
  if (multipleLibraries)
   _dllHandle = dlmopen (LM_ID_NEWLM, libraryFile, RTLD_NOW | RTLD_LOCAL | RTLD_NODELETE);
  else
   _dllHandle = dlopen (libraryFile, RTLD_NOW);

  if (!_dllHandle)
    EXIT_WITH_ERROR("Could not load %s. Check that this library's path is included in the LD_LIBRARY_PATH environment variable. Try also reducing the number of openMP threads.\n", libraryFile);

  // Functions

  alloc_config_system = (alloc_config_system_t) dlsym(_dllHandle, "alloc_config_system");
  render_video_loop = (render_video_loop_t) dlsym(_dllHandle, "render_video_loop");
  detect_system_type = (detect_system_type_t) dlsym(_dllHandle, "detect_system_type");
  main = (main_t) dlsym(_dllHandle, "main");

  // Variables
  current_system = (system_header**) dlsym(_dllHandle, "current_system");
}

blastemInstance::~blastemInstance()
{
  dlclose(_dllHandle);
}
