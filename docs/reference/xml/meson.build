gtkdocentities_conf_data = configuration_data()
gtkdocentities_conf_data.set('PACKAGE_VERSION', meson.project_version())
gtkdocentities_conf_data.set('PACKAGE_API_VERSION', GSPELL_API_VERSION)

configure_file(
  input: 'gtkdocentities.ent.in',
  output: 'gtkdocentities.ent',
  configuration: gtkdocentities_conf_data,
)
