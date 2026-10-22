cmake_minimum_required(VERSION 3.19)
project(oj LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
    add_compile_options(/Zc:__cplusplus)
endif()

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Network Sql Multimedia Test)

qt_standard_project_setup()

qt_add_executable(oj
    WIN32
    MACOSX_BUNDLE
    main.cpp
    resources.qrc
    app_icon.rc
    src/ui/mainwindow.cpp
    src/ui/mainwindow.h
    src/ui/mainwindow.ui
    src/ui/lightmodeiconhelper.cpp
    src/ui/lightmodeiconhelper.h
    src/ui/treesittersyntaxhighlighter.cpp
    src/ui/treesittersyntaxhighlighter.h
    src/ui/pages/loginpage.cpp
    src/ui/pages/loginpage.h
    src/ui/pages/aiconfigpage.cpp
    src/ui/pages/aiconfigpage.h
    src/ui/pages/homepage.cpp
    src/ui/pages/homepage.h
    src/ui/pages/classpage.cpp
    src/ui/pages/classpage.h
    src/ui/pages/contestpage.cpp
    src/ui/pages/contestpage.h
    src/ui/pages/problempage.cpp
    src/ui/pages/problempage.h
    src/ui/pages/favoritepage.cpp
    src/ui/pages/favoritepage.h
    src/ui/pages/taskboardpage.cpp
    src/ui/pages/taskboardpage.h
    src/ui/pages/statspage.cpp
    src/ui/pages/statspage.h
    src/ui/pages/reviewpage.cpp
    src/ui/pages/reviewpage.h
    src/ui/pages/settingspage.cpp
    src/ui/pages/settingspage.h
    src/ui/pages/storagepage.cpp
    src/ui/pages/storagepage.h
    src/config/appconfig.cpp
    src/config/appconfig.h
    src/config/apppaths.cpp
    src/config/apppaths.h
    src/network/cookiestore.cpp
    src/network/cookiestore.h
    src/network/openaiclient.cpp
    src/network/openaiclient.h
    src/network/openjudgeclient.cpp
    src/network/openjudgeclient.h
    src/parser/loginparser.cpp
    src/parser/loginparser.h
    src/parser/classparser.cpp
    src/parser/classparser.h
    src/parser/groupparser.cpp
    src/parser/groupparser.h
    src/parser/contestparser.cpp
    src/parser/contestparser.h
    src/parser/problemparser.cpp
    src/parser/problemparser.h
    src/parser/resultparser.cpp
    src/parser/resultparser.h
    src/parser/submitparser.cpp
    src/parser/submitparser.h
    src/parser/parsercommon.cpp
    src/parser/parsercommon.h
    src/repository/submit/homerepository.cpp
    src/repository/submit/homerepository.h
    src/repository/cache/homecacherepository.cpp
    src/repository/cache/homecacherepository.h
    src/repository/cache/classcacherepository.cpp
    src/repository/cache/classcacherepository.h
    src/repository/submit/classrepository.cpp
    src/repository/submit/classrepository.h
    src/repository/cache/contestcacherepository.cpp
    src/repository/cache/contestcacherepository.h
    src/repository/submit/contestrepository.cpp
    src/repository/submit/contestrepository.h
    src/repository/submit/problemrepository.cpp
    src/repository/submit/problemrepository.h
    src/repository/submit/resultrepository.cpp
    src/repository/submit/resultrepository.h
    src/repository/submit/submitrepository.cpp
    src/repository/submit/submitrepository.h
    src/repository/cache/problemcacherepository.cpp
    src/repository/cache/problemcacherepository.h
    src/repository/favorite/favoriteproblemrepository.cpp
    src/repository/favorite/favoriteproblemrepository.h
    src/repository/meta/problemmetarepository.cpp
    src/repository/meta/problemmetarepository.h
    src/repository/login/logincacherepository.cpp
    src/repository/login/logincacherepository.h
    src/service/browse/homeservice.cpp
    src/service/browse/homeservice.h
    src/service/browse/classservice.cpp
    src/service/browse/classservice.h
    src/service/browse/contestservice.cpp
    src/service/browse/contestservice.h
    src/service/browse/problemservice.cpp
    src/service/browse/problemservice.h
    src/service/app/applicationsizeservice.cpp
    src/service/app/applicationsizeservice.h
    src/service/ai/aiservice.cpp
    src/service/ai/aiservice.h
    src/service/cache/cacheservice.cpp
    src/service/cache/cacheservice.h
    src/service/reminder/deadlinealarmservice.cpp
    src/service/reminder/deadlinealarmservice.h
    src/service/reminder/reminderservice.cpp
    src/service/reminder/reminderservice.h
    src/service/submit/resultservice.cpp
    src/service/submit/resultservice.h
    src/service/submit/submitservice.cpp
    src/service/submit/submitservice.h
    src/service/favorite/favoriteproblemservice.cpp
    src/service/favorite/favoriteproblemservice.h
    src/service/meta/problemmetaservice.cpp
    src/service/meta/problemmetaservice.h
    src/service/login/emailverifyservice.cpp
    src/service/login/emailverifyservice.h
    src/service/login/logincacheservice.cpp
    src/service/login/logincacheservice.h
    src/service/login/loginservice.cpp
    src/service/login/loginservice.h
    third_party/tree-sitter/lib/src/lib.c
    third_party/tree-sitter-cpp/src/parser.c
    third_party/tree-sitter-cpp/src/scanner.c
    third_party/tree-sitter-python/src/parser.c
    third_party/tree-sitter-python/src/scanner.c
)

target_include_directories(oj
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/tree-sitter/lib/include
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/tree-sitter/lib/src
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/tree-sitter-cpp/src
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/tree-sitter-python/src
)

target_link_libraries(oj
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Network
        Qt::Sql
        Qt::Multimedia
)

if (WIN32)
    get_target_property(QT_QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(QT_BIN_DIR "${QT_QMAKE_EXECUTABLE}" DIRECTORY)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_BIN_DIR}")
    get_filename_component(CXX_COMPILER_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)

    foreach(runtime_dll
            libgcc_s_seh-1.dll
            libstdc++-6.dll
            libwinpthread-1.dll)
        if(EXISTS "${CXX_COMPILER_DIR}/${runtime_dll}")
            add_custom_command(TARGET oj POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${CXX_COMPILER_DIR}/${runtime_dll}"
                    "$<TARGET_FILE_DIR:oj>/${runtime_dll}"
                COMMENT "Copying ${runtime_dll} next to oj.exe"
            )
        endif()
    endforeach()

    if (WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET oj POST_BUILD
            COMMAND "${WINDEPLOYQT_EXECUTABLE}" --no-compiler-runtime "$<TARGET_FILE:oj>"
            COMMENT "Deploying Qt runtime dependencies"
        )
    endif()
endif()

install(TARGETS oj
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET oj
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})

# ---------------------------------------------------------------------------
# Parser unit tests (QtTest)
# ---------------------------------------------------------------------------
enable_testing()

qt_add_executable(oj_parser_tests
    tests/test_parsers.cpp
    src/parser/problemparser.cpp
    src/parser/problemparser.h
    src/parser/contestparser.cpp
    src/parser/contestparser.h
    src/parser/parsercommon.cpp
    src/parser/parsercommon.h
)

target_include_directories(oj_parser_tests
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(oj_parser_tests
    PRIVATE
        Qt::Core
        Qt::Test
)

add_test(NAME oj_parser_tests COMMAND oj_parser_tests)

# ---------------------------------------------------------------------------
# Packaging (CPack): macOS .dmg / Linux .tar.gz
# ---------------------------------------------------------------------------
set(CPACK_PACKAGE_NAME "oj-client")
set(CPACK_PACKAGE_VENDOR "oj-client")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "OpenJudge desktop client")

if (APPLE)
    set(CPACK_GENERATOR "DragNDrop")
elseif (UNIX)
    set(CPACK_GENERATOR "TGZ")
elseif (WIN32)
    set(CPACK_GENERATOR "ZIP")
endif()

include(CPack)
