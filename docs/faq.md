gspell FAQ
==========

Differences between gspell and GtkSpell
---------------------------------------

- gspell has more features.
- gspell is unit tested.
- For the inline GtkTextView spell-checker, GtkSpell checks the whole
  GtkTextBuffer synchronously, which can freeze the UI if the buffer contains a
  lot of text. On the other hand, gspell checks the buffer asynchronously. A
  copy of the GtkSourceRegion utility (from the GtkSourceView project) is used
  to keep track of the regions already spell-checked. This also permits to
  check only the visible regions of the GtkTextBuffer (only the text currently
  displayed in a GtkTextView).
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
- Be able to iterate on the API more freely, and have something working (and
  used by several apps) in less time. Now gspell-1 has a stable API, but
  nothing prevents me from bumping to gspell-2 to break the API, making the new
  major version parallel-installable. For the time being, GIO is stuck at
  version 2, which means that breaking the API is not possible there.
- Be the maintainer. If I wanted to integrate spell-checking in GIO and GTK,
  the project would have taken much more time. With a separate library, it was
  done in a little more than 6 months, working on and off on it (and
  part-time). I didn't want to be blocked on my work because of a lack of
  reviews.
- Having the code self-contained. Having all the spell-checking-related code in
  one repository. The biggest `*.c` file currently contains 1400 lines. Mixing
  all the GtkTextView support of gspell into GtkTextView itself would make the
  code less clear and thus harder to maintain, in my opinion. The first chapter
  of the book “Large-Scale C++ Volume I: Process and Architecture” (by
  John Lakos) suggests to me that it was a good idea to create gspell as a
  separate library, and that it would actually be a bad idea to integrate
  spell-checking into GTK. A 14k lines of code module (the current gspell's
  size) is already a good size to reason about the correctness of the code (to
  focus our attention on features related to spell-checking only).
