project('wbg-iotd', 'c',
        version: '1.3.0',
        license: 'MIT',
        meson_version: '>=0.58.0',
        default_options: [
            'c_std=c18',
            'warning_level=1',
            'werror=true',
            'b_ndebug=if-release'])

is_debug_build = get_option('buildtype').startswith('debug')

add_project_arguments(
  ['-D_POSIX_C_SOURCE=200809L', '-D_GNU_SOURCE'] +
  (is_debug_build ? ['-D_DEBUG'] : []),
  language: 'c',
)

cjson = dependency('libcjson')
curl = dependency('libcurl')
pixman = dependency('pixman-1')
jpg = dependency('libjpeg')

add_project_arguments('-DWBG_HAVE_JPG=1', language:'c')

wayland_protocols = dependency('wayland-protocols')
wayland_client = dependency('wayland-client')
tllist = dependency('tllist', version: '>=1.0.1', fallback: 'tllist')

wayland_protocols_datadir = wayland_protocols.get_variable('pkgdatadir')

wscanner = dependency('wayland-scanner', native: true)
wscanner_prog = find_program(
  wscanner.get_variable('wayland_scanner'), native: true)

wl_proto_headers = []
wl_proto_src = []
foreach prot : [
    'external/wlr-layer-shell-unstable-v1.xml',
    wayland_protocols_datadir + '/stable/xdg-shell/xdg-shell.xml']


  wl_proto_headers += custom_target(
    prot.underscorify() + '-client-header',
    output: '@BASENAME@.h',
    input: prot,
    command: [wscanner_prog, 'client-header', '@INPUT@', '@OUTPUT@'])

  wl_proto_src += custom_target(
    prot.underscorify() + '-private-code',
    output: '@BASENAME@.c',
    input: prot,
    command: [wscanner_prog, 'private-code', '@INPUT@', '@OUTPUT@'])
endforeach

image_format_sources = ['jpg.c', 'jpg.h']

executable(
    'wbg-iotd',
    'main.c',
    'log.c', 'log.h',
    'shm.c', 'shm.h',
    'stride.h',
    'wbg-features.h',
    'iotd.c', 'iotd.h',
    'file.c', 'file.h',
    image_format_sources,
    wl_proto_src + wl_proto_headers, 
    dependencies: [curl, cjson, pixman, jpg, wayland_client, tllist],
    install: true)
