interactive_tests = [
  # executable name, sources
  ['test-entry', 'test-entry.c'],
  ['test-text-view', 'test-text-view.c'],
  ['test-text-view-basic', 'test-text-view-basic.c'],
]

foreach test : interactive_tests
  executable(
    test[0], test[1],
    dependencies: GSPELL_LIB_DEP,
  )
endforeach
