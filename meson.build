# Jordan Justen : gears3d is public domain

project('gears3d', 'c')

deps_str = 'epoxy SDL2'
deps = []
foreach d : deps_str.split()
    deps += [ dependency(d) ]
endforeach

c_compiler = meson.get_compiler('c')

dep_libs_str = 'dl m'
foreach d : dep_libs_str.split()
    deps += [ c_compiler.find_library(d) ]
endforeach

srcs = 'main.c dlfcn_lib.c g_math.c'
srcs += ' gl21_compat.c gl32_core.c gles20.c gles30.c'
srcs += ' gl_shader.c options.c sdl_fw.c vert_buf.c'
srcs = srcs.split()

executable('gears3d', srcs, dependencies: deps)
