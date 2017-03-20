# Gears. Draw them. In 3D.
#
# Jordan Justen : gears3d is public domain

project('gears3d', 'c')

deps_str = 'epoxy GL SDL2 x11'
deps = []
foreach d : deps_str.split()
    deps += [ dependency(d) ]
endforeach

c_compiler = meson.get_compiler('c')

dep_libs_str = 'm'
foreach d : dep_libs_str.split()
    deps += [ c_compiler.find_library(d) ]
endforeach

srcs = 'main.c g_math.c'
srcs += ' gl21_compat.c gl32_core.c gles30.c'
srcs += ' gl_shader.c options.c'
srcs = srcs.split()

executable('gears3d', srcs, dependencies: deps)
