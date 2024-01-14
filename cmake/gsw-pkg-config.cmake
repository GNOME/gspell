# SPDX-FileCopyrightText: 2022-2023 - Sébastien Wilmet
# SPDX-License-Identifier: LGPL-3.0-or-later

macro (GswPkgConfigInit)
	find_package (PkgConfig REQUIRED)
endmacro ()

function (GswPkgConfigApply target_name pkg_dep)
	target_include_directories ("${target_name}" PRIVATE ${${pkg_dep}_INCLUDE_DIRS})
	target_compile_options ("${target_name}" PRIVATE ${${pkg_dep}_CFLAGS_OTHER})
	target_link_libraries ("${target_name}" ${${pkg_dep}_LDFLAGS})
endfunction ()
