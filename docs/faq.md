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
- Different API. gspell has more classes, while all GtkSpell public functions
  have the `gtk_spell_checker` prefix even though not all those functions
  belong to the GtkSpellChecker GObject class. So, in my opinion, gspell has a
  better API.

Why a separate library?
-----------------------

The non-GUI parts of gspell could be implemented in GIO, with an extension
point to not hard-depend on Enchant. And the GUI parts could be implemented in
GTK.

I (Sébastien Wilmet) created gspell as a separate library for several reasons:
- Be able to iterate on the API more freely. Now gspell-1 has a stable API, but
  nothing prevents me from bumping to gspell-2 to break the API, making the new
  major version parallel-installable. For the time being, GIO is stuck at
  version 2, which means that breaking the API is not possible there.
- If I wanted to integrate spell-checking in GIO and GTK, the project would have
  taken more time.
- Having all the spell-checking-related code in one repository. The biggest
  `*.c` file currently contains 1400 lines. Mixing all the GtkTextView support
  of gspell into GtkTextView itself would make the code less clear and thus
  harder to maintain, in my opinion. A 14k lines of code module (the current
  gspell's size) is already a good size to reason about the correctness of the
  code (to focus our attention on features related to spell-checking only). On
  that matter, see also the first chapter of “Large-Scale C++ Volume I: Process
  and Architecture” (John Lakos).
