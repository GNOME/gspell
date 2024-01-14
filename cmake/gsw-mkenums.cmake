# SPDX-FileCopyrightText: 2023 - Sébastien Wilmet
# SPDX-License-Identifier: LGPL-3.0-or-later

# Depends on gsw-pkg-config.cmake, GswPkgConfigInit() must be called first.

function (GswMkenumsInit)
	if (DEFINED gsw_mkenums_program_path)
		return ()
	endif ()

	pkg_get_variable (program_path glib-2.0 glib_mkenums)

	if ("${program_path}" STREQUAL "")
		message (FATAL_ERROR "glib-mkenums program not found.")
	endif ()

	set (gsw_mkenums_program_path "${program_path}" PARENT_SCOPE)
endfunction ()

function (GswMkenumsAddCommand absolute_path_to_template output_dir output_filename
	absolute_paths_to_headers)
	set (output "${output_dir}/${output_filename}")

	add_custom_command (OUTPUT "${output}"
		COMMAND ${CMAKE_COMMAND}
		ARGS -E make_directory "${output_dir}"
		COMMAND "${gsw_mkenums_program_path}"
		ARGS --template "${absolute_path_to_template}" --output "${output}" ${absolute_paths_to_headers}
		DEPENDS "${gsw_mkenums_program_path}" "${absolute_path_to_template}" ${absolute_paths_to_headers}
		COMMENT "Generating ${output_filename}")
endfunction ()
