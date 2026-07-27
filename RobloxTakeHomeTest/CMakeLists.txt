# CMakeList.txt : CMake project for RobloxTakeHomeTest, include source and define
# project specific logic here.
#

# Create a library target called NumberGenerationAlgorithms
add_library(NumberGenerationAlgorithms "Algorithms.cpp" "Algorithms.h"  )

# Add source to this project's executable.
add_executable (Tests "Tests.cpp"  )

# Link the library to the executable
target_link_libraries(Tests PRIVATE NumberGenerationAlgorithms)


if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Tests PROPERTY CXX_STANDARD 20)
endif()