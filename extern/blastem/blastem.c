/*
 Copyright 2013-2016 Michael Pavone
 This file is part of BlastEm.
 BlastEm is free software distributed under the terms of the GNU General Public License version 3 or greater. See COPYING for full license text.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "system.h"
#include "68kinst.h"
#include "m68k_core.h"
#ifdef NEW_CORE
#include "z80.h"
#else
#include "z80_to_x86.h"
#endif
#include "mem.h"
#include "vdp.h"
#include "render.h"
#include "genesis.h"
#include "gst.h"
#include "util.h"
#include "romdb.h"
#include "arena.h"
#include "config.h"
#include "bindings.h"

extern m68k_context* _context;

#define BLASTEM_VERSION "0.6.3-pre"

#ifdef __ANDROID__
#define FULLSCREEN_DEFAULT 1
#else
#define FULLSCREEN_DEFAULT 0
#endif

cothread_t _jaffarThread;
cothread_t _blastemThread;

int headless = 0;
int exit_after = 0;
int z80_enabled = 1;
int frame_limit = 0;
uint8_t use_native_states = 1;

tern_node * config;

#define SMD_HEADER_SIZE 512
#define SMD_MAGIC1 0x03
#define SMD_MAGIC2 0xAA
#define SMD_MAGIC3 0xBB
#define SMD_BLOCK_SIZE 0x4000

#define ROMFILE FILE*
#define romopen fopen
#define romread fread
#define romseek fseek
#define romgetc fgetc
#define romclose fclose

uint16_t *process_smd_block(uint16_t *dst, uint8_t *src, size_t bytes)
{
	for (uint8_t *low = src, *high = (src+bytes/2), *end = src+bytes; high < end; high++, low++) {
		*(dst++) = *low << 8 | *high;
	}
	return dst;
}

int load_smd_rom(ROMFILE f, void **buffer)
{
	uint8_t block[SMD_BLOCK_SIZE];
	romseek(f, SMD_HEADER_SIZE, SEEK_SET);

	size_t filesize = 512 * 1024;
	size_t readsize = 0;
	uint16_t *dst, *buf;
	dst = buf = malloc(filesize);
	

	size_t read;
	do {
		if ((readsize + SMD_BLOCK_SIZE > filesize)) {
			filesize *= 2;
			buf = realloc(buf, filesize);
			dst = buf + readsize/sizeof(uint16_t);
		}
		read = romread(block, 1, SMD_BLOCK_SIZE, f);
		if (read > 0) {
			dst = process_smd_block(dst, block, read);
			readsize += read;
		}
	} while(read > 0);
	romclose(f);
	
	*buffer = buf;
	
	return readsize;
}

uint8_t is_smd_format(const char *filename, uint8_t *header)
{
	if (header[1] == SMD_MAGIC1 && header[8] == SMD_MAGIC2 && header[9] == SMD_MAGIC3) {
		int i;
		for (i = 3; i < 8; i++) {
			if (header[i] != 0) {
				return 0;
			}
		}
		if (i == 8) {
			if (header[2]) {
				fatal_error("%s is a split SMD ROM which is not currently supported", filename);
			}
			return 1;
		}
	}
	return 0;
}

uint32_t load_rom(const char * filename, void **dst, system_type *stype)
{
	uint8_t header[10];
	char *ext = path_extension(filename);
	free(ext);
	ROMFILE f = romopen(filename, "rb");
	if (!f) {
		return 0;
	}
	if (sizeof(header) != romread(header, 1, sizeof(header), f)) {
		fatal_error("Error reading from %s\n", filename);
	}
	
	if (is_smd_format(filename, header)) {
		if (stype) {
			*stype = SYSTEM_GENESIS;
		}
		return load_smd_rom(f, dst);
	}
	
	size_t filesize = 512 * 1024;
	size_t readsize = sizeof(header);
		
	char *buf = malloc(filesize);
	memcpy(buf, header, readsize);
	
	size_t read;
	do {
		read = romread(buf + readsize, 1, filesize - readsize, f);
		if (read > 0) {
			readsize += read;
			if (readsize == filesize) {
				int one_more = romgetc(f);
				if (one_more >= 0) {
					filesize *= 2;
					buf = realloc(buf, filesize);
					buf[readsize++] = one_more;
				} else {
					read = 0;
				}
			}
		}
	} while (read > 0);
	
	*dst = buf;
	
	romclose(f);
	return readsize;
}



int break_on_sync = 0;
char *save_state_path;





char * save_filename;
system_header *current_system;
system_header *game_system;
void persist_save()
{
	if (!game_system) {
		return;
	}
	game_system->persist_save(game_system);
}

char *title;
void update_title(char *rom_name)
{
	if (title) {
		free(title);
		title = NULL;
	}
	title = alloc_concat(rom_name, " - BlastEm");
	render_update_caption(title);
}

static char *get_save_dir(system_media *media)
{
	char *savedir_template = tern_find_path(config, "ui\0save_path\0", TVAL_PTR).ptrval;
	if (!savedir_template) {
		savedir_template = "$USERDATA/blastem/$ROMNAME";
	}
	tern_node *vars = tern_insert_ptr(NULL, "ROMNAME", media->name);
	vars = tern_insert_ptr(vars, "ROMDIR", media->dir);
	vars = tern_insert_ptr(vars, "HOME", get_home_dir());
	vars = tern_insert_ptr(vars, "EXEDIR", get_exe_dir());
	vars = tern_insert_ptr(vars, "USERDATA", (char *)get_userdata_dir());
	char *save_dir = replace_vars(savedir_template, vars, 1);
	tern_free(vars);
	if (!ensure_dir_exists(save_dir)) {
		warning("Failed to create save directory %s\n", save_dir);
	}
	return save_dir;
}

const char *get_save_fname(uint8_t save_type)
{
	switch(save_type)
	{
	case SAVE_I2C: return "save.eeprom";
	case SAVE_NOR: return "save.nor";
	case SAVE_HBPT: return "save.hbpt";
	default: return "save.sram";
	}
}

void setup_saves(system_media *media, system_header *context)
{
	static uint8_t persist_save_registered;
	rom_info *info = &context->info;
	char *save_dir = get_save_dir(info->is_save_lock_on ? media->chain : media);
	char const *parts[] = {save_dir, PATH_SEP, get_save_fname(info->save_type)};
	free(save_filename);
	save_filename = alloc_concat_m(3, parts);
	if (info->is_save_lock_on) {
		//initial save dir was calculated based on lock-on cartridge because that's where the save device is
		//save directory used for save states should still be located in the normal place
		free(save_dir);
		parts[0] = save_dir = get_save_dir(media);
	}
	if (use_native_states || context->type != SYSTEM_GENESIS) {
		parts[2] = "quicksave.state";
	} else {
		parts[2] = "quicksave.gst";
	}
	free(save_state_path);
	save_state_path = alloc_concat_m(3, parts);
	context->save_dir = save_dir;
	if (info->save_type != SAVE_NONE) {
		context->load_save(context);
		if (!persist_save_registered) {
			atexit(persist_save);
			persist_save_registered = 1;
		}
	}
}

void apply_updated_config(void)
{
	render_config_updated();
	if (current_system && current_system->config_updated) {
		current_system->config_updated(current_system);
	}
}

static void on_drag_drop(const char *filename)
{
	if (current_system) {
		if (current_system->next_rom) {
			free(current_system->next_rom);
		}
		current_system->next_rom = strdup(filename);
		system_request_exit(current_system, 1);
	} else {
		init_system_with_media(filename, SYSTEM_UNKNOWN);
	}
}

static system_media cart, lock_on;
const system_media *current_media(void)
{
	return &cart;
}

void reload_media(void)
{
	if (!current_system) {
		return;
	}
	if (current_system->next_rom) {
		free(current_system->next_rom);
	}
	char const *parts[] = {
		cart.dir, PATH_SEP, cart.name, ".", cart.extension
	};
	char const **start = parts[0] ? parts : parts + 2;
	int num_parts = parts[0] ? 5 : 3;
	if (!parts[4]) {
		num_parts--;
	}
	current_system->next_rom = alloc_concat_m(num_parts, start);
	system_request_exit(current_system, 1);
}

void lockon_media(char *lock_on_path)
{
	reload_media();
	cart.chain = &lock_on;
	free(lock_on.dir);
	free(lock_on.name);
	free(lock_on.extension);
	lock_on.dir = path_dirname(lock_on_path);
	lock_on.name = basename_no_extension(lock_on_path);
	lock_on.extension = path_extension(lock_on_path);
	lock_on.size = load_rom(lock_on_path, &lock_on.buffer, NULL);
}

static uint32_t opts = 0;
static uint8_t force_region = 0;
void init_system_with_media(const char *path, system_type force_stype)
{
	if (game_system) {
		game_system->persist_save(game_system);
		//swap to game context arena and mark all allocated pages in it free
		mark_all_free();
		game_system->free_context(game_system);
	} else if(current_system) {
		//start a new arena and save old one in suspended system context
		current_system->arena = start_new_arena();
	}
	system_type stype = SYSTEM_UNKNOWN;
	if (!(cart.size = load_rom(path, &cart.buffer, &stype))) {
		fatal_error("Failed to open %s for reading\n", path);
	}
	free(cart.dir);
	free(cart.name);
	free(cart.extension);
	cart.dir = path_dirname(path);
	cart.name = basename_no_extension(path);
	cart.extension = path_extension(path);
	if (force_stype != SYSTEM_UNKNOWN) {
		stype = force_stype;
	}
	if (stype == SYSTEM_UNKNOWN) {
		stype = detect_system_type(&cart);
	}
	if (stype == SYSTEM_UNKNOWN) {
		fatal_error("Failed to detect system type for %s\n", path);
	}
	//allocate new system context
	game_system = alloc_config_system(stype, &cart, opts, force_region);
	if (!game_system) {
		fatal_error("Failed to configure emulated machine for %s\n", path);
	}
	setup_saves(&cart, game_system);
	update_title(game_system->info.name);
}

char *parse_addr_port(char *arg)
{
	while (*arg && *arg != ':') {
		++arg;
	}
	if (!*arg) {
		return NULL;
	}
	char *end;
	int port = strtol(arg + 1, &end, 10);
	if (port && !*end) {
		*arg = 0;
		return arg + 1;
	}
	return NULL;
}

int main(int argc, char ** argv)
{
	set_exe_str(argv[0]);
	config = load_config();
	int width = -1;
	int height = -1;
	int debug = 0;
	int loaded = 0;
	system_type stype = SYSTEM_UNKNOWN, force_stype = SYSTEM_UNKNOWN;
	char * romfname = NULL;
	char * statefile = argv[3];
	char *reader_addr = NULL, *reader_port = NULL;
	uint8_t start_in_debugger = 0;
	uint8_t fullscreen = FULLSCREEN_DEFAULT, use_gl = 1;
	uint8_t debug_target = 0;
	char *port;

	for (int i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			switch(argv[i][1]) {
			case 'b':
				i++;
				if (i >= argc) {
					fatal_error("-b must be followed by a frame count\n");
				}
				headless = 1;
				exit_after = atoi(argv[i]);
				break;
			case 'd':
				start_in_debugger = 1;
				break;
			case 'e':
				i++;
				if (i >= argc) {
					fatal_error("-e must be followed by a file name\n");
				}
				port = parse_addr_port(argv[i]);
				break;
			case 'f':
				fullscreen = !fullscreen;
				break;
			case 'g':
				use_gl = 0;
				break;
			case 'l':
				opts |= OPT_ADDRESS_LOG;
				break;
			case 'v':
				info_message("blastem %s\n", BLASTEM_VERSION);
				return 0;
				break;
			case 'n':
				z80_enabled = 0;
				break;
			case 'r':
				i++;
				if (i >= argc) {
					fatal_error("-r must be followed by region (J, U or E)\n");
				}
				force_region = translate_region_char(toupper(argv[i][0]));
				if (!force_region) {
					fatal_error("'%c' is not a valid region character for the -r option\n", argv[i][0]);
				}
				break;
			case 'm':
				i++;
				if (i >= argc) {
					fatal_error("-r must be followed by a machine type (sms, gen or jag)\n");
				}
				if (!strcmp("sms", argv[i])) {
					stype = force_stype = SYSTEM_SMS;
				} else if (!strcmp("gen", argv[i])) {
					stype = force_stype = SYSTEM_GENESIS;
				} else if (!strcmp("jag", argv[i])) {
					stype = force_stype = SYSTEM_JAGUAR;
				} else {
					fatal_error("Unrecognized machine type %s\n", argv[i]);
				}
				break;
			case 's':
				i++;
				if (i >= argc) {
					fatal_error("-s must be followed by a savestate filename\n");
				}
				statefile = argv[i];
				break;
			case 'y':
				opts |= YM_OPT_WAVE_LOG;
				break;
			case 'o': {
				i++;
				if (i >= argc) {
					fatal_error("-o must be followed by a lock on cartridge filename\n");
				}
				lock_on.size = load_rom(argv[i], &lock_on.buffer, NULL);
				if (!lock_on.size) {
					fatal_error("Failed to load lock on cartridge %s\n", argv[i]);
				}
				lock_on.name = basename_no_extension(argv[i]);
				lock_on.extension = path_extension(argv[i]);
				cart.chain = &lock_on;
				break;
			}
			case 'h':
				info_message(
					"Usage: blastem [OPTIONS] ROMFILE [WIDTH] [HEIGHT]\n"
					"Options:\n"
					"	-h          Print this help text\n"
					"	-r (J|U|E)  Force region to Japan, US or Europe respectively\n"
					"	-m MACHINE  Force emulated machine type to MACHINE. Valid values are:\n"
					"                   sms - Sega Master System/Mark III\n"
					"                   gen - Sega Genesis/Megadrive\n"
					"                   jag - Atari Jaguar\n"
					"	-f          Toggles fullscreen mode\n"
					"	-g          Disable OpenGL rendering\n"
					"	-s FILE     Load a GST format savestate from FILE\n"
					"	-o FILE     Load FILE as a lock-on cartridge\n"
					"	-d          Enter debugger on startup\n"
					"	-n          Disable Z80\n"
					"	-v          Display version number and exit\n"
					"	-l          Log 68K code addresses (useful for assemblers)\n"
					"	-y          Log individual YM-2612 channels to WAVE files\n"
					"   -e FILE     Write hardware event log to FILE\n"
				);
				return 0;
			default:
				fatal_error("Unrecognized switch %s\n", argv[i]);
			}
		} else if (!loaded) {
			reader_port = parse_addr_port(argv[i]);
			if (reader_port) {
				reader_addr = argv[i];
			} else {
				if (!(cart.size = load_rom(argv[i], &cart.buffer, stype == SYSTEM_UNKNOWN ? &stype : NULL))) {
					fatal_error("Failed to open %s for reading\n", argv[i]);
				}
				cart.dir = path_dirname(argv[i]);
				cart.name = basename_no_extension(argv[i]);
				cart.extension = path_extension(argv[i]);
			}
			romfname = argv[i];
			loaded = 1;
		} else if (width < 0) {
			width = atoi(argv[i]);
		} else if (height < 0) {
			height = atoi(argv[i]);
		}
	}
	
	int def_width = 0, def_height = 0;
	char *config_width = tern_find_path(config, "video\0width\0", TVAL_PTR).ptrval;
	if (config_width) {
		def_width = atoi(config_width);
	}
	if (!def_width) {
		def_width = 640;
	}
	char *config_height = tern_find_path(config, "video\0height\0", TVAL_PTR).ptrval;
	if (config_height) {
		def_height = atoi(config_height);
	}
	if (!def_height) {
		def_height = -1;
	}
	width = width < 1 ? def_width : width;
	height = height < 1 ? def_height : height;

	char *config_fullscreen = tern_find_path(config, "video\0fullscreen\0", TVAL_PTR).ptrval;
	if (config_fullscreen && !strcmp("on", config_fullscreen)) {
		fullscreen = !fullscreen;
	}
	if (!headless) {
		if (reader_addr) {
			render_set_external_sync(1);
		}
		render_init(width, height, "BlastEm", fullscreen);
		render_set_drag_drop_handler(on_drag_drop);
	}
	set_bindings();
	
	uint8_t use_nuklear = 0;
	char *state_format = tern_find_path(config, "ui\0state_format\0", TVAL_PTR).ptrval;
	if (state_format && !strcmp(state_format, "gst")) {
		use_native_states = 0;
	} else if (state_format && strcmp(state_format, "native")) {
		warning("%s is not a valid value for the ui.state_format setting. Valid values are gst and native\n", state_format);
	}

	if (loaded && !reader_addr) {
		if (stype == SYSTEM_UNKNOWN) {
			stype = detect_system_type(&cart);
		}
		if (stype == SYSTEM_UNKNOWN) {
			fatal_error("Failed to detect system type for %s\n", romfname);
		}
		
		current_system = alloc_config_system(stype, &cart, 0, force_region );
		if (!current_system) {
			fatal_error("Failed to configure emulated machine for %s\n", romfname);
		}
	
		setup_saves(&cart, current_system);
		update_title(current_system->info.name);
			game_system = current_system;
	}

	_context = (m68k_context*)current_system;
	current_system->start_context(current_system, statefile);
	return 0;
}

int __argc;
char** __argv;

void blastemWrapper()
{
  main(__argc, __argv);
  fatal_error("Should not reach this point!\n");
}

void start(int argc, char** argv, int isHeadlessMode)
{
 headless = isHeadlessMode;
 __argc = argc;
 __argv = (char**) malloc (sizeof(char*) * argc);
 for (size_t i = 0; i < argc; i++)
 {
  __argv[i] = (char*) malloc(strlen(argv[i]));
  strcpy(__argv[i], argv[i]);
 }
 _blastemThread = co_create(1 << 24, blastemWrapper);
 _jaffarThread = co_active();
 co_switch(_blastemThread);
}

void resume()
{
 co_switch(_blastemThread);
}

