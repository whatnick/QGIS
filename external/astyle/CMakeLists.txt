SET(ASTYLE_SRCS
  astyle_main.cpp
  ASBeautifier.cpp
  ASEnhancer.cpp
  ASFormatter.cpp
  ASLocalizer.cpp
  ASResource.cpp
)

# No warnings for astyle build
IF(NOT MSVC)
  SET_SOURCE_FILES_PROPERTIES(${ASTYLE_SRCS} PROPERTIES COMPILE_FLAGS -w)
ENDIF(NOT MSVC)
ADD_EXECUTABLE(qgisstyle ${ASTYLE_SRCS})
SET_TARGET_PROPERTIES(qgisstyle PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/scripts)
