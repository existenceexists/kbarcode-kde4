set(ProjectName kbarcode005)

project(${ProjectName})

# search packages used by KDE
find_package(KDE4 REQUIRED)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} 
                      ${KDE4_DATA_DIR}/cmake/modules
                      ${CMAKE_SOURCE_DIR}/cmake/modules )


include (KDE4Defaults)
include (MacroLibrary)

add_definitions (-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

add_subdirectory(src)
