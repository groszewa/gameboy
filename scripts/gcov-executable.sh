#!/bin/sh
# Print a gcov-compatible reader for the coverage data.
# Candidates are probed in order; the first that actually runs wins.
# xcrun first: it only succeeds on an Apple toolchain.
if xcrun llvm-cov gcov --version >/dev/null 2>&1; then
    echo "xcrun llvm-cov gcov"
# GNU gcov next: the canonical reader for GCC-produced data (CI).
elif gcov --version 2>/dev/null | grep -q GCC; then
    echo gcov
elif command -v llvm-cov >/dev/null 2>&1; then
    echo "llvm-cov gcov"
else
    echo gcov
fi

