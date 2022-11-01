gspell FAQ
==========

Differences between gspell and GtkSpell
---------------------------------------

- gspell has more features and unit tests.

- For the inline GtkTextView spell-checker, GtkSpell checks the whole
  GtkTextBuffer at once (in one main loop iteration), which can freeze the UI if
  the buffer contains a lot of text. On the other hand, gspell checks the buffer
  chunk by chunk (in several main loop iterations). A copy of the
  GtkSourceRegion utility (from the GtkSourceView project) is used to keep track
  of the regions already spell-checked. That way, gspell also checks only the
  visible regions of the GtkTextBuffer (only the text currently displayed in a
  GtkTextView).

- Different API. gspell has more classes.

Why a separate library?
-----------------------

The non-GUI parts of gspell could be implemented in GIO, with an extension
point to not hard-depend on Enchant. And the GUI parts could be implemented in
GTK.

gspell has been created as a separate library for several reasons:

- To be able to iterate on the API more freely. gspell has now a stable API, but
  it's still possible to create new major versions that are installable in
  parallel and that have a different API. On the other hand, GIO is currently
  stuck at version 2, the only possibility there is to deprecate an old API and
  create a new one.

- A project to integrate spell-checking into GIO and GTK would have taken more
  time.

- Having all the spell-checking-related code in one repository. The gspell
  module currently contains 14k lines of code, with the biggest `*.c` file
  containing 1400 lines. It's already a good size. In general, a small module is
  easier to work with and to reason about the correctness of the code. On that
  matter, see also the first chapter of “Large-Scale C++ Volume I: Process and
  Architecture” (John Lakos).
