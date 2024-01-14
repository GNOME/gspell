set (public_headers
	gspell.h
	gspell-checker.h
	gspell-checker-dialog.h
	gspell-entry.h
	gspell-entry-buffer.h
	gspell-language.h
	gspell-language-chooser.h
	gspell-language-chooser-button.h
	gspell-language-chooser-dialog.h
	gspell-navigator.h
	gspell-navigator-text-view.h
	gspell-text-buffer.h
	gspell-text-view.h)

set (public_c_files
	gspell-checker.c
	gspell-checker-dialog.c
	gspell-entry.c
	gspell-entry-buffer.c
	gspell-language.c
	gspell-language-chooser.c
	gspell-language-chooser-button.c
	gspell-language-chooser-dialog.c
	gspell-navigator.c
	gspell-navigator-text-view.c
	gspell-text-buffer.c
	gspell-text-view.c)

set (private_headers
	gconstructor.h
	gspellregion.h
	gspell-checker-private.h
	gspell-context-menu.h
	gspell-current-word-policy.h
	gspell-entry-private.h
	gspell-entry-utils.h
	gspell-icu.h
	gspell-inline-checker-text-buffer.h
	gspell-text-iter.h
	gspell-utils.h)

set (private_c_files
	gspellregion.c
	gspell-context-menu.c
	gspell-current-word-policy.c
	gspell-entry-utils.c
	gspell-icu.c
	gspell-init.c
	gspell-inline-checker-text-buffer.c
	gspell-text-iter.c
	gspell-utils.c)

install (FILES ${public_headers}
	DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}-${gsw_lib_api_version}/${PROJECT_NAME}")

# mkenums

GswMiscGetAbsolutePaths ("${public_headers}" absolute_paths_to_public_headers)

# Why "glib-mkenums/": to have a dedicated dir, to separate from other concerns.
# Why "[...]/${PROJECT_NAME}/": to support the #include of the public header.
set (output_dir "${PROJECT_BINARY_DIR}/glib-mkenums/${PROJECT_NAME}")

GswMiscGetAbsolutePaths ("gspell-enum-types.h.in" absolute_path_to_template)
GswMkenumsAddCommand ("${absolute_path_to_template}" "${output_dir}" "gspell-enum-types.h"
	"${absolute_paths_to_public_headers}")
install (FILES "${output_dir}/gspell-enum-types.h"
	DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}-${gsw_lib_api_version}/${PROJECT_NAME}")
list (APPEND built_public_headers "${output_dir}/gspell-enum-types.h")

GswMiscGetAbsolutePaths ("gspell-enum-types.c.in" absolute_path_to_template)
GswMkenumsAddCommand ("${absolute_path_to_template}" "${output_dir}" "gspell-enum-types.c"
	"${absolute_paths_to_public_headers}")
list (APPEND built_public_c_files "${output_dir}/gspell-enum-types.c")

# Resources

# Why "gresources/": to have a dedicated dir, to separate from other concerns.
set (output_dir "${PROJECT_BINARY_DIR}/gresources")
GswMiscGetAbsolutePaths ("resources" xml_dir)
GswResourcesAddCommand ("${xml_dir}" "gspell.gresource.xml"
	"${output_dir}" "gspell-resources.c")
list (APPEND built_public_c_files "${output_dir}/gspell-resources.c")

# The library

set (library_name "${PROJECT_NAME}-${gsw_lib_api_version}")
add_library ("${library_name}" SHARED
	${public_headers}
	${public_c_files}
	${private_headers}
	${private_c_files}
	${built_public_headers}
	${built_public_c_files})

target_compile_definitions ("${library_name}"
	PRIVATE "-DGSPELL_COMPILATION")

GswPkgConfigApply ("${library_name}" DEPS)
GswCFlagsApply ("${library_name}")

# For #include <gspell/gspell-something.h>
target_include_directories ("${library_name}" PRIVATE "${PROJECT_SOURCE_DIR}")

# For #include "config.h"
target_compile_definitions ("${library_name}"
	PRIVATE "-DHAVE_CONFIG_H")
target_include_directories ("${library_name}" PRIVATE "${PROJECT_BINARY_DIR}/config-h")

target_compile_definitions ("${library_name}"
	PRIVATE "-DG_LOG_DOMAIN=\"${PROJECT_NAME}\"")

install (TARGETS "${library_name}"
	DESTINATION "${CMAKE_INSTALL_LIBDIR}")
