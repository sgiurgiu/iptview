
set(IPTVIEW_SRCS main.cpp)

qt_add_resources(IPTVIEW_SRCS_RS_SRCS resources.qrc)

qt_add_executable(${EXECUTABLE_NAME} ${IPTVIEW_SRCS} ${IPTVIEW_SRCS_RS_SRCS})



set(IPTVIEWLIB_SRCS
    m3uparser.h m3uparser.cpp
    mediasegment.h mediasegment.cpp
    m3ulist.h m3ulist.cpp
    m3uparsercontroller.h m3uparsercontroller.cpp
    iptviewmainwindow.h iptviewmainwindow.cpp
    iptviewmainwidget.h iptviewmainwidget.cpp
    channelswidget.h channelswidget.cpp
    mediawidget.h mediawidget.cpp
    mpvwidget.h mpvwidget.cpp
    )

add_library(${LIBRARY_NAME} STATIC ${IPTVIEWLIB_SRCS})

target_link_libraries(${EXECUTABLE_NAME} PRIVATE ${IPTVIEW_APP_LIBARIES})
target_link_libraries(${EXECUTABLE_NAME} PRIVATE ${LIBRARY_NAME})

target_link_libraries(${LIBRARY_NAME} PUBLIC ${IPTVIEW_LIB_LIBARIES})
target_include_directories(${LIBRARY_NAME} PUBLIC ${IPTVIEW_LIB_INCLUDES})


foreach(TARGET_ITEM IN ITEMS ${EXECUTABLE_NAME} ${LIBRARY_NAME})
    set_property(TARGET ${TARGET_ITEM} PROPERTY  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    target_compile_definitions(${TARGET_ITEM} PRIVATE "$<$<C_COMPILER_ID:MSVC>:_WIN32_WINNT=0x0601>")
    target_compile_definitions(${TARGET_ITEM} PRIVATE "$<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>")
    target_compile_definitions(${TARGET_ITEM} PRIVATE "$<$<C_COMPILER_ID:MSVC>:NOMINMAX>")
    target_compile_definitions(${TARGET_ITEM} PRIVATE "$<$<C_COMPILER_ID:MSVC>:_UNICODE>")
    target_compile_definitions(${TARGET_ITEM} PRIVATE "$<$<C_COMPILER_ID:MSVC>:UNICODE>")
    target_compile_options(${TARGET_ITEM} PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:/bigobj>")
    target_compile_options(${TARGET_ITEM} PRIVATE "$<$<C_COMPILER_ID:MSVC>:/bigobj>")
    set_target_properties(${TARGET_ITEM} PROPERTIES WIN32_EXECUTABLE TRUE)
endforeach()

install(TARGETS ${EXECUTABLE_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
