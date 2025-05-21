gspell - dev notes
==================

Flatpak support
---------------

At the time of writing, gspell is not installed as part of the GNOME runtime, so
it needs to be bundled with the app.

hunspell and hunspell dictionaries are installed by the Flatpak freedesktop.org
runtime, they are not shared with the host OS. See the
[freedesktop-sdk](https://gitlab.com/freedesktop-sdk/freedesktop-sdk/)
repository (`git grep -in hunspell`).

Adding words to the personal dictionary works, but each Flatpak app has a
different personal dictionary. For example for Polari the personal dictionary
is stored in:

```
~/.var/app/org.gnome.Polari/config/enchant/
```

It works thanks to the `XDG_CONFIG_HOME` environment variable (Enchant calls the
`g_get_user_config_dir()` GLib function).

To share personal dictionaries between Flatpak apps, maybe a good solution is to
set the `ENCHANT_CONFIG_DIR` environment variable to a common path (if set, it
has a higher priority than `XDG_CONFIG_HOME`).

Using hunspell directly instead of Enchant?
-------------------------------------------

- https://rrthomas.github.io/enchant/
- https://fedoraproject.org/wiki/Releases/FeatureDictionary
- https://wiki.ubuntu.com/ConsolidateSpellingLibs

Various links
-------------

- GNOME: [Platform-wide Spell Checking](https://wiki.gnome.org/Initiatives/SpellChecking),
  predates the creation of gspell.
- https://zverok.github.io/blog/2021-05-06-how-to-spellcheck.html
