# tools

include_cli = include_directories('include')

exe = executable(
  'dpa', 
  'cli.c',
  include_directories : [inc, include_cli],
  link_with : libdpa,
  install : false,
  install_dir : 'bin'
)
