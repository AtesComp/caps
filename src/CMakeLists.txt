add_executable(caps main.cpp)
target_sources(caps
    PUBLIC
    ConcentratedSystem.cpp
    DistributedSystem.cpp
    LoadAndInteraction.cpp
    LoadNodal.cpp
    LoadPoint.cpp
    LoadTrapezoidal.cpp
    LoadUniform.cpp
    MaterialProperty.cpp
    Member.cpp
    Node.cpp
    PlaneFrame.cpp
    PlaneFrame.hpp
    PPSAFile.cpp
    ProblemSize.cpp
    Reaction.cpp
    Report.cpp
    StiffnessMatrix.cpp
    StressFactors.cpp
)
target_compile_features(caps PUBLIC cxx_std_20)

find_package(fmt)
target_link_libraries(caps fmt::fmt)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
