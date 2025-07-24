set(PROJECT_NAME "LSCSim")

project(${PROJECT_NAME})
message(STATUS "cmaking ${PROJECT_NAME} ...")

# ----------------------------------------------------------------------------
# Locate sources and headers for this project
# ----------------------------------------------------------------------------
file(GLOB ${PROJECT_NAME}_SOURCES ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB ${PROJECT_NAME}_HEADERS ${PROJECT_SOURCE_DIR}/${PROJECT_NAME}/*.hh)

file(MAKE_DIRECTORY ${PROJECT_INCLUDE_DIR}/${PROJECT_NAME})

foreach(HEADER_i ${${PROJECT_NAME}_HEADERS})
  configure_file(${HEADER_i} ${PROJECT_INCLUDE_DIR}/${PROJECT_NAME} COPYONLY)
endforeach(HEADER_i)

add_library(${PROJECT_NAME} SHARED
  ${${PROJECT_NAME}_SOURCES}
)

install(PROGRAMS
  ${CMAKE_CURRENT_BINARY_DIR}/lib${PROJECT_NAME}.so
  DESTINATION
  ${CMAKE_INSTALL_LIBDIR}
)

# ----------------------------------------------------------------------------
# build executables
# ----------------------------------------------------------------------------
set(LINK_PACKAGES LSCSim MCObjs GLG4Sim)
#set(LINK_PACKAGES MCObjs GLG4Sim LSCSim)
set(LINK_EXTERNALS ${ROOT_LIBRARIES} ${Geant4_LIBRARIES})

file(GLOB ${PROJECT_NAME}_EXECUTABLES ${PROJECT_SOURCE_DIR}/test/*.cc)

foreach(testsourcefile ${${PROJECT_NAME}_EXECUTABLES})
  get_filename_component(testname ${testsourcefile} NAME_WE)
  add_executable(${testname} ${testsourcefile})
  target_link_libraries(${testname} ${LINK_PACKAGES} ${LINK_EXTERNALS})
  install(TARGETS ${testname} DESTINATION ${CMAKE_INSTALL_BINDIR})
endforeach(testsourcefile ${${PROJECT_NAME}_EXECUTABLES})

install(DIRECTORY ${PROJECT_SOURCE_DIR}/mac DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY ${PROJECT_SOURCE_DIR}/data DESTINATION ${CMAKE_INSTALL_PREFIX})
