
set(CMAKE_INCLUDE_CURRENT_DIR ON)
enable_testing(true)

qt_add_executable(test_m3uparser
    testm3uparser.cpp
    )
add_test(NAME test_m3uparser COMMAND test_m3uparser)

qt_add_executable(test_mediasegment
    TestMediaSegment.cpp
    )
add_test(NAME test_mediasegment COMMAND test_mediasegment)

qt_add_executable(test_m3ulist
    TestM3UList.cpp
    )
add_test(NAME test_m3ulist COMMAND test_m3ulist)


foreach(TARGET_ITEM IN ITEMS test_m3uparser test_mediasegment test_m3ulist)
    target_link_libraries(${TARGET_ITEM} PRIVATE ${IPTVIEW_TEST_LIBARIES})
    target_link_libraries(${TARGET_ITEM} PRIVATE ${LIBRARY_NAME})
    set_property(TARGET ${TARGET_ITEM} PROPERTY  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    target_compile_definitions(${TARGET_ITEM} PRIVATE "$<$<C_COMPILER_ID:MSVC>:_WIN32_WINNT=0x0601>")
    target_compile_definitions(${TARGET_ITEM} PRIVATE "$<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>")
    target_compile_definitions(${TARGET_ITEM} PRIVATE "$<$<C_COMPILER_ID:MSVC>:NOMINMAX>")
    target_compile_definitions(${TARGET_ITEM} PRIVATE "$<$<C_COMPILER_ID:MSVC>:_UNICODE>")
    target_compile_definitions(${TARGET_ITEM} PRIVATE "$<$<C_COMPILER_ID:MSVC>:UNICODE>")
    set_target_properties(${TARGET_ITEM} PROPERTIES WIN32_EXECUTABLE TRUE)
endforeach()

