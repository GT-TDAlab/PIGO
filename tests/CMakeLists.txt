# ----------------------------------------------------------------------------
# PIGO Test System
# Copyright (c) GT-TDAlab
#
# This is built to test PIGO. Similar to other cmake systems, it should be
# used by making a new directory.
# ----------------------------------------------------------------------------

# ----------------------------------------------------------------------------
# Add the tester file as an interface
add_library(pigo_test_utils INTERFACE)
target_include_directories(pigo_test_utils INTERFACE ./)

# ----------------------------------------------------------------------------
# Add in testing directories
add_subdirectory(base)
add_subdirectory(csr)
add_subdirectory(coo)
add_subdirectory(tensor)
