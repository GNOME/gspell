unit_tests = [
  'test-checker',
  'test-entry',
  'test-icu',
  'test-inline-checker-text-buffer',
  'test-text-iter',
  'test-utils',
]

unit_tests_install_dir = join_paths(
  get_option('prefix'),
  get_option('libexecdir'),
  'installed-tests',
  '@0@-@1@'.format(meson.project_name(), GSPELL_API_VERSION)
)

foreach test_name : unit_tests
  test_exe = executable(
    test_name,
    test_name + '.c',
    dependencies: GSPELL_STATIC_DEP,
    install: get_option('install_tests'),
    install_dir: unit_tests_install_dir,
  )
  test(test_name, test_exe)

  if get_option('install_tests')
    unit_test_config_data = configuration_data()
    unit_test_config_data.set('TEST_NAME', test_name)
    unit_test_config_data.set('UNIT_TESTS_INSTALL_DIR', unit_tests_install_dir)

    configure_file(
      input: 'gspell.test.in',
      output: '@0@.test'.format(test_name),
      configuration: unit_test_config_data,
      install: true,
      install_dir: join_paths(
        get_option('datadir'),
        'installed-tests',
        '@0@-@1@'.format(meson.project_name(), GSPELL_API_VERSION)
      ),
    )
  endif
endforeach
