# tests

catch2_dep = dependency(
    'catch2-with-main', 
    fallback: ['catch2', 'catch2_with_main_dep']
)

test_launch_exe = executable(
    'test_launch',
    'test_launch.cpp',
    link_with: libdpa,
    dependencies: catch2_dep,
    include_directories: inc,
    install: false
)
