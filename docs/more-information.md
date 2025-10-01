gspell - more information
=========================

About versions
--------------

gspell follows the even/odd minor version scheme.

For example the `1.7.x` versions are unstable (development versions), and the
`1.8.x` versions are stable.

About tarballs
--------------

Since October 2025, only a git tag is created for releasing gspell, and tarballs
are no longer uploaded to `download.gnome.org`.

[Old tarballs](https://download.gnome.org/sources/gspell/) are still available.

Dependencies
------------

- GLib
- GTK 3
- [Enchant](https://rrthomas.github.io/enchant/)
- [ICU](https://icu.unicode.org/)

Notes for packagers
-------------------

At least on GNU/Linux, it is better to install hunspell. aspell can be
considered deprecated and doesn't work well with gspell. If both hunspell and
aspell are installed, Enchant prefers hunspell by default.

See also:
- https://fedoraproject.org/wiki/Releases/FeatureDictionary
- https://wiki.ubuntu.com/ConsolidateSpellingLibs

Development resources
---------------------

- [Discourse](https://discourse.gnome.org/tag/gspell) (Platform category, gspell tag)
