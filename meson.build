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

default_library='static'
abs_base = subproject('abseil').get_variable('absl_base_dep')
abs_cont = subproject('abseil').get_variable('absl_container_dep')
default_library='shared'

sdl2_dep = dependency('sdl2')
sdl2_image_dep = dependency('sdl2_image')

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
  
run_command('make', '-C', 'extern/blastem', 'clean')
run_command('make', '-C', 'extern/blastem', 'libblastem.so', '-j8')
#run_command('make', '-C', 'extern/blastem', 'libblastem.so', 'DEBUG=true', '-j8')

executable('jaffar2-test',
  'source/test.cc',
  jaffar2Files,
  dependencies: deps,
  include_directories: inc,
  link_with: [ ],
  link_args: [ '-ldl', '-lncurses' ],
  cpp_args: [ '-Wfatal-errors', '-fpermissive' ]
  )