gspell - more information
=========================

About versions
--------------

gspell follows the even/odd minor version scheme.

For example the `1.7.x` versions are unstable (development versions), and the
`1.8.x` versions are stable.

Dependencies
------------

- GLib
- GTK 3
- Enchant
- [ICU](http://site.icu-project.org/)

If building from Git:
- autoconf-archive (version 2021.02.19 or 2022.02.11, both work)

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

Development resources
---------------------

- [Discourse](https://discourse.gnome.org/) (Platform category, gspell tag)
- [Tarballs](https://download.gnome.org/sources/gspell/)

Development and maintenance
---------------------------

The project is in low-maintenance state.
