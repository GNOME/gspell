gspell.next
===========

What would a gspell.next project look like? (Apart from porting to the latest
major version of GTK). Note, there are currently no plans in gspell to work on
these items.

Expose more of the internals
----------------------------

To have an even more flexible toolkit.

Providing only a high-level API doesn't suit the needs of _all_ applications. So
by providing an API for some of the building blocks, it would ease the
implementation of a more customized spell-checker feature in a more complex
application.

But when more and more APIs are exposed, during the development phase, it's
important to be able to iterate on the API design, _and_ testing these APIs in
real-world applications. So basically it would require to bump the major/API
version more frequently, when improving the APIs. Like the Tepl library. Or
having a long period during which the API is unstable, and the apps need to
bundle the library.

Choice for the dependency used for `gspell_language_get_name()`
---------------------------------------------------------------

Its first implementation was based on the iso-codes package (by parsing the XML
files). Then it was replaced by the ICU. And there is a branch to use the
MS-Windows API.

Provide the three implementations, and be able to choose which one to use at
configure/compile time.

To have a smaller package size for certain apps (if gspell is bundled), or for
Linux distros to have a smaller set of packages installed by default, etc.

BUT, of course this would have a drawback, because there can be different sets
of bugs depending on which build option is used.

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
