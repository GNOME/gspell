# SPDX-FileCopyrightText: 2023 - Sébastien Wilmet
# SPDX-License-Identifier: LGPL-3.0-or-later

cmake_minimum_required (VERSION 3.27)
project (gsw-c-flags
	LANGUAGES C)

add_executable (test test.c)

include (gsw-c-flags-get-list.cmake)
GswCFlagsGetList (additional_c_flags)

target_compile_options (test PRIVATE ${additional_c_flags})
