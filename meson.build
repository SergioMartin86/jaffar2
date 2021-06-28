project('jaffar2', 'c', 'cpp',
  version: '1.0.0',
  license: 'GPL-3.0-only',
  default_options : ['cpp_std=c++17', 'default_library=static', 'buildtype=debugoptimized', 'c_args=-DSDL_VIDEO_DRIVER_DUMMY'],
)

cc = meson.get_compiler('c')
cxx = meson.get_compiler('cpp')

inc = include_directories([
 'extern/blastem',
 'extern/metrohash64',
 'extern/argparse',
 'extern/json/single_include'
  ])

jaffar2Files = [
  'source/blastemInstance.cc',
  'source/utils.cc',
  'extern/metrohash64/metrohash64.cc'
]

blastemFiles = [
  'extern/blastem/blastem.c',
  'extern/blastem/system.c',
  'extern/blastem/genesis.c',
  'extern/blastem/debug.c',
  'extern/blastem/gdb_remote.c',
  'extern/blastem/vdp.c',
  'extern/blastem/ppm.c',
  'extern/blastem/controller_info.c',
  'extern/blastem/render_sdl.c',
  'extern/blastem/zlib/adler32.c',
  'extern/blastem/zlib/compress.c',
  'extern/blastem/zlib/crc32.c',
  'extern/blastem/zlib/deflate.c',
  'extern/blastem/zlib/gzclose.c',
  'extern/blastem/zlib/gzlib.c',
  'extern/blastem/zlib/gzread.c',
  'extern/blastem/zlib/gzwrite.c',
  'extern/blastem/zlib/infback.c',
  'extern/blastem/zlib/inffast.c',
  'extern/blastem/zlib/inflate.c',
  'extern/blastem/zlib/inftrees.c',
  'extern/blastem/zlib/trees.c',
  'extern/blastem/zlib/uncompr.c',
  'extern/blastem/zlib/zutil.c',
  'extern/blastem/png.c',
  'extern/blastem/io.c',
  'extern/blastem/romdb.c',
  'extern/blastem/hash.c',
  'extern/blastem/menu.c',
  'extern/blastem/xband.c',
  'extern/blastem/realtec.c',
  'extern/blastem/i2c.c',
  'extern/blastem/nor.c',
  'extern/blastem/sega_mapper.c',
  'extern/blastem/multi_game.c',
  'extern/blastem/megawifi.c',
  'extern/blastem/net.c',
  'extern/blastem/serialize.c',
  'extern/blastem/terminal.c',
  'extern/blastem/config.c',
  'extern/blastem/tern.c',
  'extern/blastem/util.c',
  'extern/blastem/paths.c',
  'extern/blastem/gst.c',
  'extern/blastem/68kinst.c',
  'extern/blastem/m68k_core.c',
  'extern/blastem/m68k_core_x86.c',
  'extern/blastem/gen.c',
  'extern/blastem/backend.c',
  'extern/blastem/mem.c',
  'extern/blastem/arena.c',
  'extern/blastem/gen_x86.c',
  'extern/blastem/backend_x86.c',
  'extern/blastem/ym2612.c',
  'extern/blastem/psg.c',
  'extern/blastem/wave.c',
  'extern/blastem/vgm.c',
  'extern/blastem/event_log.c',
  'extern/blastem/render_audio.c',
  'extern/blastem/saves.c',
  'extern/blastem/zip.c',
  'extern/blastem/bindings.c',
  'extern/blastem/jcart.c',
  'extern/blastem/gen_player.c',
  'extern/blastem/nuklear_ui/font.c',
  'extern/blastem/nuklear_ui/blastem_nuklear.c',
  'extern/blastem/nuklear_ui/sfnt.c',
  'extern/blastem/sms.c',
  'extern/blastem/z80inst.c',
  'extern/blastem/z80_to_x86.c',
]

default_library='static'
abs_base = subproject('abseil').get_variable('absl_base_dep')
abs_cont = subproject('abseil').get_variable('absl_container_dep')
default_library='shared'

sdl2_dep = dependency('sdl2', required: true)
sdl2_image_dep = dependency('sdl2_image', required: true)
glew_dep = dependency('glew', required: true) 
 
mpi_dep = dependency('mpi', language: 'cpp', required: false)  
if mpi_dep.found() == false
 mpi_dep = dependency('mpich', required: false)
endif

boost_dep = dependency('boost')

deps = [
  mpi_dep,
  boost_dep,
  sdl2_dep,
  sdl2_image_dep,
  dependency('openmp', required: true),
  abs_base,
  abs_cont
]


blastemLib = shared_library('blastemLib',
  blastemFiles,
  c_args: [   
  '-flto',
  '-I/usr/include/libdrm',
  '-std=gnu99',
  '-Wreturn-type',
  '-Werror=return-type',
  '-Werror=implicit-function-declaration',
  '-Wno-unused-value',
  '-Wpointer-arith',
  '-Werror=pointer-arith',
  '-DHAS_PROC',
  '-DHAVE_UNISTD_H',
  '-DX86_64',
  '-m64'
  ],
  dependencies : [ sdl2_dep, sdl2_image_dep, glew_dep ],
  link_args: [ '-lm' ]
  )
  
    
executable('jaffar2-test',
  'source/test.cc',
  jaffar2Files,
  dependencies: deps,
  include_directories: inc,
  link_with: [ ],
  link_args: [ '-ldl', '-lncurses' ],
  cpp_args: [ '-Wfatal-errors', '-fpermissive' ]
  )