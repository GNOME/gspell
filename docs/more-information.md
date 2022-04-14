gspell - more information
=========================

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

To build the latest version of gspell plus some of its dependencies,
[Jhbuild](https://wiki.gnome.org/Projects/Jhbuild) can be used.

Development resources
---------------------

- [Discourse](https://discourse.gnome.org/) (Platform category, gspell tag)
- [Tarballs](https://download.gnome.org/sources/gspell/)

Contributions
-------------

Contributions are no longer accepted for this project. See the `HACKING` file
for more information.
