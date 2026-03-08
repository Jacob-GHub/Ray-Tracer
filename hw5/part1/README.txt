This homework continues to build upon the ray tracing code the
previous assignments.  It contains two new header and two new source
files (as well as a written portion, which also needs to be
completed).  These source files should be added to the source files
you already have from homework 4.  In addition, you will need to
modify your existing registration.cpp file by uncommenting the lines
related to the files you are adding.  If your program fails to parse
the input files, you have likely not un-commented enough lines (or too
many lines) in the registration.cpp file.  If parsing succeeds, you
have done this correctly.

The zip file contains a directory called "tests-hw5", which contains a
bunch of new test cases; you will need to copy this over as well.
These are the test cases that will be used for grading the coding
portion of homework 5.  You can run the grading script for homework 5
as "python ./grading-script.py tests-hw5" .  Note that running "python
./grading-script.py tests-hw4" will continue to produce your grade for
homework 4.  As you go, it is helpful to rerun the grading script from
prior homework as well.  Although we will not be doing so while
grading, it can help you identify regressions that may have been
introduced on test cases that are easier to debug.  Note also that
homework 5 builds on what you did for homework 4.  If you still have
failing test cases for homework 4, you will need to fix those to pass
the test cases for homework 5.
