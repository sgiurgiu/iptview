
set(IPTVIEW_SRCS main.cpp)

qt_add_resources(IPTVIEW_SRCS_RS_SRCS resources.qrc)
set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/../resources/iptview.rc")

qt_add_executable(${EXECUTABLE_NAME} ${IPTVIEW_SRCS} ${IPTVIEW_SRCS_RS_SRCS} ${app_icon_resource_windows})

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
    channelsmodel.h channelsmodel.cpp
    abstractchanneltreeitem.h abstractchanneltreeitem.cpp
    databaseprovider.h databaseprovider.cpp
    database.h database.cpp
    channeltreeitem.h channeltreeitem.cpp
    grouptreeitem.h grouptreeitem.cpp
    roottreeitem.h roottreeitem.cpp
    favouritestreeitem.h favouritestreeitem.cpp
    channelsfilteringmodel.h channelsfilteringmodel.cpp
    loadingchannelsthread.h loadingchannelsthread.cpp
    loadingchanneliconsworker.h loadingchanneliconsworker.cpp
    xstreamcodewizard.cpp xstreamcodewizard.h
    xstreaminfo.h
    loadxstreamchannelsworker.h loadxstreamchannelsworker.cpp
    epgwidget.h epgwidget.cpp
    xstreamchannelsmodel.h xstreamchannelsmodel.cpp
    servertreeitem.h servertreeitem.cpp
    )

if(ENABLE_DBUS)
    qt_add_dbus_adaptor(GENERATED_DBUS_ADAPTORS manager_adaptor.xml mprisdbus.h MPRISDBus mprisdbus_manager_adaptor DBusManagerAdaptor)
    qt_add_dbus_adaptor(GENERATED_DBUS_ADAPTORS player_adaptor.xml mprisdbus.h MPRISDBus mprisdbus_player_adaptor DBusPlayerAdaptor)

    list(APPEND IPTVIEWLIB_SRCS mprisdbus.h mprisdbus.cpp ${GENERATED_DBUS_ADAPTORS})
endif()


add_library(${LIBRARY_NAME} STATIC ${IPTVIEWLIB_SRCS} )

target_link_libraries(${EXECUTABLE_NAME} PRIVATE ${IPTVIEW_APP_LIBARIES} ${LIBMPV})
target_link_libraries(${EXECUTABLE_NAME} PRIVATE ${LIBRARY_NAME})

target_link_libraries(${LIBRARY_NAME} PUBLIC ${IPTVIEW_LIB_LIBARIES})
target_include_directories(${LIBRARY_NAME} PUBLIC ${IPTVIEW_LIB_INCLUDES})

foreach(TARGET_ITEM IN ITEMS ${EXECUTABLE_NAME} ${LIBRARY_NAME})
    target_compile_definitions(${TARGET_ITEM} PRIVATE "$<$<CONFIG:Debug>:IPTVIEW_DEBUG>")
    target_compile_definitions(${TARGET_ITEM} PRIVATE "$<$<BOOL:${ENABLE_DBUS}>:IPTVIEW_DBUS>")

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

if(WIN32)
    file(REAL_PATH ${LIBMPV_DIR} MPV_DIR)
    install(FILES "${MPV_DIR}/mpv-2.dll"  DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
