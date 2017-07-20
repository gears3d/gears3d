#!/usr/bin/env python3

# `header` applies to this and generated files
header = '''
/* Jordan Justen : gears3d is public domain */
'''.lstrip()

import os
import subprocess
import sys
import tempfile

shaders = [
    {
        'stage': 'vert',
        'src':
'''#version 430

layout(std140, set = 0, binding = 0) uniform block {
    mat4 model;
    mat4 view;
    mat4 projection;
    float gear_angle;
    float tooth_angle;
};

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 rel_norm;

layout(location = 0) out vec3 norm;
layout(location = 1) out vec3 light_dir;

const vec3 light_pos = vec3(5.0, 5.0, 10.0);

void main()
{
    float ang = tooth_angle * gl_InstanceIndex + gear_angle;
    mat2 rotz = mat2(vec2(cos(ang), sin(ang)),
                     vec2(-sin(ang), cos(ang)));

    vec3 pos = vec3(rotz * vertex.xy, vertex.z);
    vec4 m_pos = model * vec4(pos, 1.0);
    m_pos = vec4(m_pos.xyz / m_pos.w, 1.0);
    gl_Position = projection * view * m_pos;

    light_dir = normalize(light_pos - m_pos.xyz);

    vec3 n_pos = vertex + rel_norm;
    n_pos = vec3(rotz * n_pos.xy, n_pos.z);
    vec4 m_norm = model * vec4(n_pos, 1.0);
    norm = normalize((m_norm.xyz / m_norm.w) - m_pos.xyz);
}
'''
    },
    {
        'stage': 'frag',
        'src':
'''#version 420

layout(push_constant) uniform pconst {
    vec4 gear_color;
};

layout(location = 0) in vec3 norm;
layout(location = 1) in vec3 light_dir;

layout(location = 0) out vec4 f_color;

void main()
{
    float light_ref = clamp(0.0+dot(norm, light_dir), -0.0, 1.0);
    float light = clamp(0.2+light_ref, 0.1, 1.0);
    f_color = vec4(light * gear_color.xyz, 1.0);
}
'''
    },
]

def compile_to_spirv(shader):
    #print(shader['stage'], len(shader['src']))
    cmd = [ 'glslangValidator', '-V', '-S', shader['stage'] ]
    in_file = tempfile.NamedTemporaryFile()
    in_file.write(shader['src'].encode('utf-8'))
    in_file.flush()
    out_file = tempfile.NamedTemporaryFile()
    cmd += [ '-o', out_file.name, in_file.name ]
    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE)
    (stdout, stderr) = proc.communicate()
    assert proc.returncode == 0
    in_file.close()
    out_file.seek(0)
    shader['spirv'] = out_file.read()

def compile_shaders(shaders):
    for shader in shaders:
        compile_to_spirv(shader)

def shader_to_h(f, shader):
    print(str.format('static unsigned char vk_{}_spirv[] = {{',
                     shader['stage']), file=f)
    offset = 0
    for by in shader['spirv']:
        if offset % 12 == 0:
            d = ' '
        d += ' 0x{:02x}'.format(by)
        offset += 1
        if offset < len(shader['spirv']):
            d += ','
        if offset % 12 == 0 or offset == len(shader['spirv']):
            print(d, file=f)
    print('};', file=f)
    print(str.format('static unsigned int vk_{}_spirv_len = {};',
                     shader['stage'], len(shader['spirv'])), file=f)
    print(file=f)

def write_vk_spirv_h(shaders):
    f = open('vk-spirv.h', 'w')
    f.write(header)
    print('#ifndef vk_spirv_included', file=f)
    print('#define vk_spirv_included', file=f)
    print(file=f)
    for shader in shaders:
        shader_to_h(f, shader)
    print('#endif', file=f)
    f.close()

compile_shaders(shaders)
write_vk_spirv_h(shaders)
