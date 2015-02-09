# The entry point for SIP.
#
# Copyright (c) 2015 Riverbank Computing Limited <info@riverbankcomputing.com>
#
# This file is part of SIP.
#
# This copy of SIP is licensed for use under the terms of the SIP License
# Agreement.  See the file LICENSE for more details.
#
# This copy of SIP may also used under the terms of the GNU General Public
# License v2 or v3 as published by the Free Software Foundation which can be
# found in the files LICENSE-GPL2 and LICENSE-GPL3 included in this package.
#
# SIP is supplied WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.


import argparse

from sip5 import SIP_VERSION, SIP_VERSION_STR
from sip5._sip import (parse, transform, generateCode, generateExtracts,
        generateAPI, generateXML)


def main():
    """ The entry point for the setuptools generated CLI wrapper. """

    # Parse the command line.
    parser = argparse.ArgumentParser(
            description="Generate Python extension modules for C/C++ "
                    "libraries.",
            fromfile_prefix_chars='@')

    parser.add_argument('-V', action='version', version=SIP_VERSION_STR)

    parser.add_argument('filename',
            help="the name of the specification file [default stdin]",
            metavar="FILE", nargs='?')

    parser.add_argument('-a', dest='apiFile',
            help="the name of the QScintilla API file [default not generated]",
            metavar="FILE")

    parser.add_argument('-B', dest='backstops', action='append',
            help="add <TAG> to the list of timeline backstops",
            metavar="TAG")

    parser.add_argument('-c', dest='codeDir',
            help="the name of the code directory [default not generated]",
            metavar="DIR")

    parser.add_argument('-e', dest='exceptions', action='store_true',
            default=False,
            help="enable support for exceptions [default disabled]")

    parser.add_argument('-g', dest='releaseGIL', action='store_true',
            default=False,
            help="always release and reacquire the GIL [default only when "
                    "specified]")

    parser.add_argument('-I', dest='includeDirList', action='append',
            help="add <DIR> to the list of directories to search when "
                    "including files",
            metavar="DIR")

    parser.add_argument('-j', dest='parts', type=int, default=0,
            help="split the generated code into <FILES> files [default 1 per "
                    "class]",
            metavar="FILES")

    parser.add_argument('-k', dest='kwArgs', action='store_const', const=1,
            default=0,
            help="support keyword arguments in functions and methods")

    parser.add_argument('-m', dest='xmlFile',
            help="the name of the XML export file [default not generated]",
            metavar="FILE")

    parser.add_argument('-o', dest='docs', action='store_true', default=False,
            help="enable the automatic generation of docstrings [default "
                    "disabled]")

    parser.add_argument('-p', dest='consModule',
            help="the name of the consolidated module that this is a "
                    "component of",
            metavar="MODULE")

    parser.add_argument('-P', dest='protHack', action='store_true',
            default=False,
            help="enable the protected/public hack [default disabled]")

    parser.add_argument('-r', dest='tracing', action='store_true',
            default=False,
            help="generate code with tracing enabled [default disabled]")

    parser.add_argument('-s', dest='srcSuffix',
            help="the suffix to use for C or C++ source files [default \".c\" "
                    "or \".cpp\"]",
            metavar="SUFFIX")

    parser.add_argument('-t', dest='versions', action='append',
            help="add <TAG> to the list of versions/platforms to generate "
                    "code for",
            metavar="TAG")

    parser.add_argument('-T', dest='timestamp', action='store_false',
            default=True,
            help="disable the timestamp in the header of generated files "
                    "[default enabled]")

    parser.add_argument('-w', dest='warnings', action='store_true',
            default=False,
            help="enable warning messages [default disabled]")

    parser.add_argument('-x', dest='xfeatures', action='append',
            help="add <FEATURE> to the list of disabled features",
            metavar="FEATURE")

    parser.add_argument('-X', dest='extracts', action='append',
            help="add <ID:FILE> to the list of extracts to generate",
            metavar="ID:FILE")

    args = parser.parse_args()

    # Issue warnings after they (might) have been enabled.
    if args.kwArgs != 0 and args.warnings:
        print("the -k flag is deprecated")

    # Parse the input file.
    pt = parse(SIP_VERSION, SIP_VERSION_STR, args.filename,
            args.includeDirList, args.versions, args.backstops, args.xfeatures,
            args.kwArgs, args.protHack)

    # Verify and transform the parse tree.
    transform(pt)

    # Generate the code.
    if args.codeDir is not None:
        generateCode(pt, args.codeDir, args.srcSuffix, args.exceptions,
                args.tracing, args.releaseGIL, args.parts, args.versions,
                args.xfeatures, args.consModule, args.docs, args.timestamp)

    # Generate any extracts.
    generateExtracts(pt, args.extracts)

    # Generate the API file.
    if args.apiFile is not None:
        generateAPI(pt, args.apiFile)

    # Generate the XML export.
    if args.xmlFile is not None:
        generateXML(pt, args.xmlFile)

    # All done.
    return 0
