project(tfc-docs)

# set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR})
include(tfc_docs)
set_docs(${CMAKE_CURRENT_SOURCE_DIR} ON)
