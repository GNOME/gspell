executable(
  'gspell-app' + GSPELL_API_VERSION,
  'gspell-app.c',
  dependencies: GSPELL_LIB_DEP,
  install: true,
  c_args: [
    '-DHAVE_CONFIG_H', # TODO: remove once Autotools removed.
  ],
)
