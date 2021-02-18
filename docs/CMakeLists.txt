# ----------------------------------------------------------------------------
# PIGO
# Copyright (c) GT-TDAlab
# ----------------------------------------------------------------------------

# ----------------------------------------------------------------------------
# Enable doxygen
set(DOXYGEN_EXTRACT_ALL YES)
set(DOXYGEN_BUILTIN_STL_SUPPORT YES)

# ----------------------------------------------------------------------------
# Specify doxygen sources
doxygen_add_docs(docs
    pigo.hpp
    pigo/coo.hpp
    pigo/csr.hpp
    pigo/graph.hpp
    pigo/matrix.hpp
    "${CMAKE_CURRENT_SOURCE_DIR}/mainpage.md"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/include")
