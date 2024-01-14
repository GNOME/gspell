# SPDX-FileCopyrightText: 2023 - Sébastien Wilmet
# SPDX-License-Identifier: LGPL-3.0-or-later

# Try to mimic the AX_COMPILER_FLAGS Autotools macro.
#
# For the rationale (having such a long list of flags instead of just relying on
# -Wall and -Wextra) see the GCC manpage for the -Wall option:
#
# """
# Note that some warning flags are not implied by -Wall. Some of them warn about
# constructions that users generally do not consider questionable, but which
# occasionally you might wish to check for; others warn about constructions that
# are necessary or hard to avoid in some cases, and there is no simple way to
# modify the code to suppress the warning. Some of them are enabled by -Wextra
# but many of them must be enabled individually.
# """
function (GswCFlagsGetList flags)
	set (${flags}
		"-Wall"
		"-Wextra"
		"-fno-strict-aliasing"
		"-Wundef"
		"-Wwrite-strings"
		"-Wpointer-arith"
		"-Wmissing-declarations"
		"-Wredundant-decls"
		"-Wno-unused-parameter"
		"-Wno-missing-field-initializers"
		"-Wformat=2"
		"-Wcast-align"
		"-Wformat-nonliteral"
		"-Wformat-security"
		"-Wsign-compare"
		"-Wstrict-aliasing"
		"-Wshadow"
		"-Winline"
		"-Wpacked"
		"-Wmissing-format-attribute"
		"-Wmissing-noreturn"
		"-Winit-self"
		"-Wredundant-decls"
		"-Wmissing-include-dirs"
		"-Wunused-but-set-variable"
		"-Warray-bounds"
		"-Wreturn-type"
		"-Wswitch-enum"
		"-Wswitch-default"
		"-Wduplicated-cond"
		"-Wduplicated-branches"
		"-Wlogical-op"
		"-Wrestrict"
		"-Wnull-dereference"
		"-Wdouble-promotion"
		"-Wnested-externs"
		"-Wmissing-prototypes"
		"-Wstrict-prototypes"
		"-Wdeclaration-after-statement"
		"-Wimplicit-function-declaration"
		"-Wold-style-definition"
		"-Wjump-misses-init"
		PARENT_SCOPE)
endfunction ()
