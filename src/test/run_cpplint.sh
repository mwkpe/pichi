#!/bin/sh
cd ../ext/cpplint
./cpplint.py \
--linelength=100 \
--filter=\
-build/include,\
-build/header_guard,\
-build/c++11,\
-whitespace/braces,\
-whitespace/indent,\
-whitespace/newline,\
-whitespace/braces,\
-readability/todo,\
-readability/braces,\
-legal/copyright \
$(find ../.. -name \*.h -or -name \*.cpp | \
grep -vE "doctest|gsl|mainwindow\.|\/test")