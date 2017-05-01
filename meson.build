# Jordan Justen : gears3d is public domain

project('gears3d', 'c')

c_compiler = meson.get_compiler('c')

build_dep_libs_str = 'X11 xcb'
foreach d : build_dep_libs_str.split()
    c_compiler.find_library(d)
endforeach

deps = []
dep_libs_str = 'dl m'
foreach d : dep_libs_str.split()
    deps += [ c_compiler.find_library(d) ]
endforeach

srcs = 'main.c dlfcn_lib.c g_math.c'
srcs += ' glfn.c gl21_compat.c gl32_core.c gles20.c gles30.c'
srcs += ' gl_shader.c glx.c options.c sim.c vert_buf.c vk10.c'
srcs += ' winsys_x11r6.c'
srcs = srcs.split()

executable('gears3d', srcs, dependencies: deps)
