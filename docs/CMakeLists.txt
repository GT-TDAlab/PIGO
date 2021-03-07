find_package(Doxygen QUIET)
find_package(Sphinx QUIET)

if (Doxygen_FOUND AND Sphinx_FOUND)
    set(DOXYGEN_INPUT_DIR ${PROJECT_SOURCE_DIR})
    set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/doxygen)
    set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIR}/xml/index.xml)
    set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    # Replace placeholders
    configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

    file(MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIR})
    add_custom_command(OUTPUT ${DOXYGEN_INDEX_FILE}
            DEPENDS ${header_files}
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            MAIN_DEPENDENCY ${DOXYFILE_OUT} ${DOXYFILE_IN}
            COMMENT "Generating Docs"
            VERBATIM)

    set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR})
    set(SPHINX_BUILD ${CMAKE_CURRENT_BINARY_DIR}/sphinx)
    set(SPHINX_INDEX_FILE ${SPHINX_BUILD}/index.html)
    set(SPHINX_CUSTOM_SIDE ${SPHINX_BUILD}/_static/css/side_color.css)

    add_custom_command(OUTPUT ${SPHINX_INDEX_FILE}
            COMMAND
            ${SPHINX_EXECUTABLE} -b html -Dbreathe_projects.pigo=${DOXYGEN_OUTPUT_DIR}/xml
            ${SPHINX_SOURCE} ${SPHINX_BUILD}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            DEPENDS
            ${CMAKE_CURRENT_SOURCE_DIR}/index.rst
            ${DOXYGEN_INDEX_FILE}
            MAIN_DEPENDENCY ${SPHINX_SOURCE}/conf.py
            COMMENT "Generating documentation with Sphinx")

    add_custom_command(OUTPUT ${SPHINX_CUSTOM_SIDE}
            COMMAND
            echo ".wy-menu-vertical header, .wy-menu-vertical p.caption { color: #55a5d9; } \
              .wy-side-nav-search > a img.logo, .wy-side-nav-search .wy-dropdown > a img.logo { height: 55px; }" > ${SPHINX_CUSTOM_CSS}
            DEPENDS
            ${SPHINX_INDEX_FILE}
            VERBATIM)

    if (NOT TARGET docs)
        add_custom_target(docs DEPENDS ${DOXYGEN_INDEX_FILE} ${SPHINX_INDEX_FILE} ${SPHINX_CUSTOM_SIDE})
    endif ()

    # Install if docs are built
    if (EXISTS ${SPHINX_BUILD})
        install(DIRECTORY ${SPHINX_BUILD}/ DESTINATION ${CMAKE_INSTALL_DOCDIR})
    endif ()

else ()
    message(WARNING "Doxygen and Sphinx needed for Doc generation. Skipping target")
endif ()
