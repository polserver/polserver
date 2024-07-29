#!/bin/sh
#
# Run from escript dir to update all .out files for listfile tests.
#
# Since this updates the expected output for the tests to _be_ what the test
# outputs (kinda circular?) the .out files **MUST** be manually verified! Since this
# runs on *all* listfile tests, should be easy to spot unintended consequences
# to implementation changes.
#

set -e

for F in $(ls funcexpr/listfile-*.src); do
# F=funcexpr/listfile-complex-2.src
    ../../bin/ecompile -f -l $F
    rm -rf $basefile.lst
    basefile=$(dirname $F)/$(basename $F .src)
    ../../bin/runecl $basefile.ecl > $basefile.out
    rm -rf $basefile.ecl $basefile.lst
done
