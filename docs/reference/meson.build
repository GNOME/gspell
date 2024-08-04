subdir('xml')

gtkdoc_module_name = '@0@-@1@.0'.format(meson.project_name(), GSPELL_API_VERSION)

FS.copyfile(
  '@0@-sections.txt'.format(meson.project_name()),
  '@0@-sections.txt'.format(gtkdoc_module_name)
)

html_dir = get_option('prefix') / GNOME.gtkdoc_html_dir(gtkdoc_module_name)

GNOME.gtkdoc(
  gtkdoc_module_name,
  main_xml: '@0@-docs.xml'.format(meson.project_name()),
  src_dir: include_directories('../../gspell/'),
  dependencies: GSPELL_LIB_DEP,
  scan_args: ['--rebuild-types'],
  gobject_typesfile: '@0@.types'.format(gtkdoc_module_name),
  fixxref_args: [
    '--html-dir=@0@'.format(html_dir),
  ],
  content_files: [
    'intro.xml',
  ],
  ignore_headers: GSPELL_PRIVATE_HEADERS,
  install: true,
)
