subdir('xml')

gtkdoc_module_name = '@0@-@1@'.format(meson.project_name(), GSPELL_API_VERSION)

FS.copyfile(
  '@0@-sections.txt'.format(meson.project_name()),
  '@0@-sections.txt'.format(gtkdoc_module_name)
)

html_dir = get_option('prefix') / GNOME.gtkdoc_html_dir(gtkdoc_module_name)

glib_docpath = dependency('glib-2.0').get_variable(pkgconfig: 'prefix') / 'share/gtk-doc/html/glib'
gobject_docpath = dependency('gobject-2.0').get_variable(pkgconfig: 'prefix') / 'share/gtk-doc/html/gobject'
gio_docpath = dependency('gio-2.0').get_variable(pkgconfig: 'prefix') / 'share/gtk-doc/html/gio'
gtk_docpath = dependency('gtk+-3.0').get_variable(pkgconfig: 'prefix') / 'share/gtk-doc/html/gtk3'

GNOME.gtkdoc(
  gtkdoc_module_name,
  main_xml: '@0@-docs.xml'.format(meson.project_name()),
  src_dir: include_directories('../../gspell/'),
  dependencies: GSPELL_LIB_DEP,
  scan_args: ['--rebuild-types'],
  gobject_typesfile: '@0@.types'.format(gtkdoc_module_name),
  fixxref_args: [
    '--html-dir=@0@'.format(html_dir),
    '--extra-dir=@0@'.format(glib_docpath),
    '--extra-dir=@0@'.format(gobject_docpath),
    '--extra-dir=@0@'.format(gio_docpath),
    '--extra-dir=@0@'.format(gtk_docpath),
  ],
  content_files: [
    'intro.xml',
  ],
  ignore_headers: [
    GSPELL_PRIVATE_HEADERS,
    'gspell.h', # For ignoring GSPELL_H_INSIDE.
  ],
  install: true,
)
