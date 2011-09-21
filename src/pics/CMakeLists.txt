# Created by František Brožka 7.9.2011 .

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/logo.png ${CMAKE_CURRENT_BINARY_DIR}/logo.png COPYONLY)
install(FILES hi16-app-kbarcode.png DESTINATION ${DATA_INSTALL_DIR}/kbarcode003)

message(STATUS "\${CMAKE_CURRENT_SOURCE_DIR} is: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "\${CMAKE_CURRENT_BINARY_DIR} is: ${CMAKE_CURRENT_BINARY_DIR}")

