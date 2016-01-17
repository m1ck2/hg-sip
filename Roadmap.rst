SIP v5 Roadmap
==============

The next major release of SIP will be v5 and is currently being planned.  While
we make no committments to release dates, we expect that the development will
be done during 2015.

The major focus of v5 will be to:

- eliminate inconsistencies in the syntax of specification files

- fill in some gaps in the C/C++ support

- restructure, refactor and rewrite the code as appropriate to ensure that it
  is easy to test, maintain and enhance over the long term.

There is no plan to introduce any significant new functionality.

Any feedback on the roadmap is very welcome.


Roadmap
-------

Here we list specific changes that are planned.  Note that no changes are
planned for the ``sip`` extension module.

- Support will be added for fixed sized arrays of any type.

- Support for optionally detecting overflows when converting from Python
  integers to C/C++ types will be investigated.

- Error messages will be improved and will always include a reference to the
  originating file and line number.

- The XML export file will be officially supported and its format documented.
  Its contents will be enhanced to fully describe the Python API that is
  generated including aspects of the C/C++ API that influence it (e.g.
  ownership, whether a method can be reimplemented etc.).

- Support for the generation of QScintilla API files will be removed.  A
  utility to create these files from the XML export file will be added to
  QScintilla.

- The code generator's ``-I`` command line option will support Windows style
  path names.

- The code generator may be extended using plugins.

- All PyQt specific support will be removed and implemented in appropriate
  plugins that will be distributed as part of PyQt.

- The design of the code generator will allow for the implementation of plugins
  to support generating bindings for languages other than Python.

- The code generator will be reimplemented using Python v3.  It will be able to
  be used as a standalone application or a package.
