# SPDX-FileCopyrightText: 2023 - Sébastien Wilmet
# SPDX-License-Identifier: LGPL-3.0-or-later

function (GswCFlagsInit)
	if (NOT DEFINED gsw_c_flags_supported)
		try_compile (gsw_c_flags_supported
			PROJECT "gsw-c-flags"
			SOURCE_DIR "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/gsw-c-flags")
	endif ()
endfunction ()

function (GswCFlagsApply target_name)
	if ("${gsw_c_flags_supported}")
		include ("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/gsw-c-flags/gsw-c-flags-get-list.cmake")
		GswCFlagsGetList (additional_c_flags)
		target_compile_options ("${target_name}" PRIVATE ${additional_c_flags})
	endif ()
endfunction ()

function (GswCFlagsPrintMessage)
	message ("gsw-c-flags: ${gsw_c_flags_supported}")
endfunction ()
