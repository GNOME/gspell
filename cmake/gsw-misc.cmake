# SPDX-FileCopyrightText: 2022-2023 - Sébastien Wilmet
# SPDX-License-Identifier: LGPL-3.0-or-later

# Useful for printing a configuration summary.
function (GswMiscGetYesOrNo condition result)
	if (${condition})
		set (${result} yes PARENT_SCOPE)
	else ()
		set (${result} no PARENT_SCOPE)
	endif ()
endfunction ()

function (GswMiscGetAbsolutePaths files_list output_list)
	foreach (file ${files_list})
		set (absolute_file "${CMAKE_CURRENT_SOURCE_DIR}/${file}")
		list (APPEND absolute_files_list "${absolute_file}")
	endforeach ()

	set (${output_list} "${absolute_files_list}" PARENT_SCOPE)
endfunction ()
