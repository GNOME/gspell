gspell - a spell-checking library for GTK applications
======================================================

This is version 1.9.1 of gspell.

gspell provides a flexible API to add spell-checking to a GTK application.

The gspell library is free software and is released under the terms of the GNU
LGPL version 2.1 or later. See the file 'COPYING' for more information.

The [official web page of gspell](https://wiki.gnome.org/Projects/gspell).

About versions
--------------

gspell follows the GNOME release schedule and uses the same
[versioning scheme](https://developer.gnome.org/programming-guidelines/stable/versioning.html.en).

For example the 1.7.x versions are unstable, and the 1.8.x versions are stable.

Dependencies
------------

* GLib
* GTK
* Enchant
* [ICU](http://site.icu-project.org/)

Notes for packagers
-------------------

At least on GNU/Linux, it is better to install hunspell. aspell can be
considered deprecated and doesn't work well with gspell. If both hunspell and
aspell are installed, Enchant prefers hunspell by default.

See also:
- https://fedoraproject.org/wiki/Releases/FeatureDictionary
- https://wiki.ubuntu.com/ConsolidateSpellingLibs

Installation
------------

Simple install procedure from a tarball:

```
$ ./configure
$ make
[ Become root if necessary ]
$ make install
```

See the file 'INSTALL' for more detailed information.

From the Git repository, the 'configure' script and the 'INSTALL' file are not
yet generated, so you need to run 'autogen.sh' instead, which takes the same
arguments as 'configure'.

To build the latest version of gspell plus some of its dependencies from Git,
[Jhbuild](https://wiki.gnome.org/Projects/Jhbuild) can be used.

Contributions
-------------

Contributions are no longer accepted for this project. See the `HACKING` file
for more information.
