# SPDX-FileCopyrightText: 2024 - Sébastien Wilmet
# SPDX-License-Identifier: LGPL-3.0-or-later

# Depends on gsw-pkg-config.cmake, GswPkgConfigInit() must be called first.

function (GswResourcesInit)
	if (DEFINED gsw_compile_resources_program_path)
		return ()
	endif ()

	pkg_get_variable (program_path gio-2.0 glib_compile_resources)

	if ("${program_path}" STREQUAL "")
		message (FATAL_ERROR "glib-compile-resources program not found.")
	endif ()

	set (gsw_compile_resources_program_path "${program_path}" PARENT_SCOPE)
endfunction ()

function (GswResourcesAddCommand xml_dir xml_filename output_dir output_filename)
	set (xml_file "${xml_dir}/${xml_filename}")
	set (output "${output_dir}/${output_filename}")

	# The stamp file is never created, which cause the command to be run
	# every time. Easier to implement than using
	# `glib-compile-resources --generate-dependencies`.
	add_custom_command (OUTPUT "${output}" "${output_dir}/gsw-resources-stamp"
		COMMAND ${CMAKE_COMMAND}
		ARGS -E make_directory "${output_dir}"
		COMMAND "${gsw_compile_resources_program_path}"
		ARGS --target "${output}" --sourcedir "${xml_dir}" --generate-source "${xml_file}"
		COMMENT "Generating ${output_filename}")
endfunction ()
