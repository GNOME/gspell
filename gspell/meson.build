subdir('resources')

gspell_public_headers = [
  'gspell.h',
  'gspell-checker.h',
  'gspell-checker-dialog.h',
  'gspell-entry.h',
  'gspell-entry-buffer.h',
  'gspell-language.h',
  'gspell-language-chooser.h',
  'gspell-language-chooser-button.h',
  'gspell-language-chooser-dialog.h',
  'gspell-navigator.h',
  'gspell-navigator-text-view.h',
  'gspell-text-buffer.h',
  'gspell-text-view.h',
]

gspell_public_c_files = [
  'gspell-checker.c',
  'gspell-checker-dialog.c',
  'gspell-entry.c',
  'gspell-entry-buffer.c',
  'gspell-language.c',
  'gspell-language-chooser.c',
  'gspell-language-chooser-button.c',
  'gspell-language-chooser-dialog.c',
  'gspell-navigator.c',
  'gspell-navigator-text-view.c',
  'gspell-text-buffer.c',
  'gspell-text-view.c',
]

GSPELL_PRIVATE_HEADERS = [
  'gconstructor.h',
  'gspellregion.h',
  'gspell-checker-private.h',
  'gspell-context-menu.h',
  'gspell-current-word-policy.h',
  'gspell-entry-private.h',
  'gspell-entry-utils.h',
  'gspell-icu.h',
  'gspell-inline-checker-text-buffer.h',
  'gspell-text-iter.h',
  'gspell-utils.h',
]

gspell_private_c_files = [
  'gspellregion.c',
  'gspell-context-menu.c',
  'gspell-current-word-policy.c',
  'gspell-entry-utils.c',
  'gspell-icu.c',
  'gspell-init.c',
  'gspell-inline-checker-text-buffer.c',
  'gspell-text-iter.c',
  'gspell-utils.c',
]

headers_install_dir = get_option('includedir') / '@0@-@1@/gspell/'.format(meson.project_name(), GSPELL_API_VERSION)
install_headers(
  gspell_public_headers,
  install_dir: headers_install_dir
)

gspell_enum_types = GNOME.mkenums_simple(
  'gspell-enum-types',
  decorator: 'G_MODULE_EXPORT',
  header_prefix: '#include <gmodule.h>',
  sources: gspell_public_headers,
  install_header: true,
  install_dir: headers_install_dir,
)

gspell_static_lib_c_args = ['-DGSPELL_COMPILATION']
gspell_static_lib_link_args = []

if host_machine.system() == 'darwin'
  GSPELL_PRIVATE_HEADERS += ['gspell-osx.h']
  gspell_private_c_files += ['gspell-osx.c']

  gspell_static_lib_c_args += ['-xobjective-c']
  gspell_static_lib_link_args += ['-framework', 'Cocoa']
endif

gspell_static_lib = static_library(
  'gspell-static',
  [gspell_public_c_files,
   gspell_private_c_files,
   gspell_enum_types,
   GSPELL_RESOURCES],
  pic: true, # gspell_static_lib is linked in a shared library.
  include_directories: ROOT_INCLUDE_DIR,
  dependencies: GSPELL_DEPS,
  c_args: gspell_static_lib_c_args,
  link_args: gspell_static_lib_link_args,
)

# For unit tests, to be able to test private functions.
GSPELL_STATIC_DEP = declare_dependency(
  include_directories: ROOT_INCLUDE_DIR,
  link_with: gspell_static_lib,
  sources: gspell_enum_types[1],
  dependencies: GSPELL_DEPS,
)

symbol_map = 'symbol.map'
gspell_lib_link_args = C_COMPILER.get_supported_link_arguments([
  '-Wl,--version-script,@0@/@1@'.format(meson.current_source_dir(), symbol_map),
])

gspell_lib = library(
  'gspell-@0@'.format(GSPELL_API_VERSION),
  dependencies: GSPELL_DEPS,
  link_args: gspell_lib_link_args,
  link_depends: symbol_map,
  # link_whole is not supported with MSVC, so we use extract_all_objects().
  objects: gspell_static_lib.extract_all_objects(recursive: false),
  soversion: 3,
  install: true,
)

GSPELL_LIB_DEP = declare_dependency(
  include_directories: ROOT_INCLUDE_DIR,
  link_with: gspell_lib,
  sources: gspell_enum_types[1],
  dependencies: GSPELL_DEPS,
)

PKG_CONFIG.generate(gspell_lib,
  filebase: '@0@-@1@'.format(meson.project_name(), GSPELL_API_VERSION),
  name: meson.project_name(),
  description: 'Spell-checking for GTK applications',
  subdirs: '@0@-@1@'.format(meson.project_name(), GSPELL_API_VERSION),
  libraries: GSPELL_PUBLIC_DEPS,
)

if get_option('gobject_introspection')
  gspell_gir = GNOME.generate_gir(
    gspell_lib,
    export_packages: '@0@-@1@'.format(meson.project_name(), GSPELL_API_VERSION),
    header: 'gspell/gspell.h',
    identifier_prefix: 'Gspell',
    include_directories: ROOT_INCLUDE_DIR,
    includes: ['Gtk-3.0'],
    install: true,
    namespace: 'Gspell',
    nsversion: GSPELL_API_VERSION,
    sources: [
      gspell_public_headers,
      gspell_public_c_files,
      gspell_enum_types,
    ],
    # Support for deps being built as subprojects:
    dependencies: GSPELL_DEPS,
  )

  if get_option('vapi')
    GNOME.generate_vapi(
      '@0@-@1@'.format(meson.project_name(), GSPELL_API_VERSION),
      sources: gspell_gir[0],
      install: true,
      packages: ['gtk+-3.0'],
    )
  endif
endif
