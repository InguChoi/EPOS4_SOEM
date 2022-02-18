
set(SOURCES epos4.c)
add_executable(epos4 ${SOURCES})
target_link_libraries(epos4 soem)
install(TARGETS epos4 DESTINATION bin)
