unit_tests = [
  'test-checker',
  'test-entry',
  'test-icu',
  'test-inline-checker-text-buffer',
  'test-text-iter',
  'test-utils',
]

foreach test_name : unit_tests
  test_exe = executable(
    test_name,
    test_name + '.c',
    dependencies: GSPELL_STATIC_DEP,
  )
  test(test_name, test_exe)
endforeach
