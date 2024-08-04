subdir('xml')

# TODO: generate and use XML entities instead.
gtkdoc_config_data = configuration_data()
gtkdoc_config_data.set('GSPELL_API_VERSION', GSPELL_API_VERSION)
configure_file(
  input: 'intro.xml.in',
  output: 'intro.xml',
  configuration: gtkdoc_config_data,
)
configure_file(
  input: 'gspell-docs.xml.in',
  output: 'gspell-docs.xml',
  configuration: gtkdoc_config_data,
)

gtkdoc_module_name = '@0@-@1@.0'.format(meson.project_name(), GSPELL_API_VERSION)

html_dir = get_option('prefix') / GNOME.gtkdoc_html_dir(gtkdoc_module_name)

GNOME.gtkdoc(
  gtkdoc_module_name,
  main_xml: meson.current_build_dir() / '@0@-docs.xml'.format(meson.project_name()),
  src_dir: include_directories('../../gspell/'),
  dependencies: GSPELL_LIB_DEP,
  scan_args: ['--rebuild-types'],
  gobject_typesfile: '@0@.types'.format(gtkdoc_module_name),
  fixxref_args: [
    '--html-dir=@0@'.format(html_dir),
  ],
  content_files: [
    meson.current_build_dir() / 'intro.xml',
  ],
  ignore_headers: GSPELL_PRIVATE_HEADERS,
  install: true,
)
