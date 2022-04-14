gspell.next
===========

What would a gspell.next project look like? (Apart from porting to the latest
major version of GTK). Note, I don't plan to work on it. Insert here elves,
wisdom, rainbows, unicorns, …

Expose more of the internals
----------------------------

To have an even more flexible toolkit. Like Tepl.

But it would require to bump the major/API version more frequently, when the
implementation changes.

Choice for the dependency used for `gspell_language_get_name()`
---------------------------------------------------------------

Its first implementation was based on the iso-codes package (by parsing the XML
files). Then it was replaced by the ICU. And there is a branch to use the
MS-Windows API.

Provide the three implementations, and be able to choose which one to use at
configure/compile time.

To have a smaller package size for certain apps (if gspell is bundled), or for
Linux distros to have a smaller set of packages installed by default, etc.

Interfaces
----------

"Program against interfaces", the Design Patterns book teaches us.

For example for GspellLanguage and GspellChecker (the core classes), have
interfaces in addition to classes. To be able to use something else than
Enchant.

Async I/O only
--------------

Dedicate a separate thread for calling Enchant APIs? The Enchant APIs are
synchronous currently, if I recall correctly. So provide an async API for
GspellLanguage and GspellChecker? If it degrades too much the performances,
words need to be checked in batches.
