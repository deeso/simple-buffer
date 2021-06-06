set(CMAKE_CXX_STANDARD 17)
set(LINKED_LIBRARIES stdc++ simple_buffer ${Boost_LIBRARIES} pthread)
add_executable(simple_main main.cpp ../${INCLUDE}/options.h ../${INCLUDE}/buffer.h)
target_link_libraries(simple_main  ${LINKED_LIBRARIES})
