cmake_minimum_required(VERSION 3.19)
project(oj LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
    add_compile_options(/Zc:__cplusplus)
endif()

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Network Sql Multimedia Test)

set(LEXBOR_BUILD_SHARED OFF)
set(LEXBOR_BUILD_STATIC ON)
set(LEXBOR_BUILD_TESTS OFF)
set(LEXBOR_BUILD_EXAMPLES OFF)
set(LEXBOR_BUILD_UTILS OFF)
add_subdirectory(lexbor EXCLUDE_FROM_ALL)

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
    src/ai/aiconfig.cpp
    src/ai/aiconfig.h
    src/ai/openaiclient.cpp
    src/ai/openaiclient.h
    src/ai/aiservice.cpp
    src/ai/aiservice.h
    src/network/cookiestore.cpp
    src/network/cookiestore.h
    src/network/openjudgeclient.cpp
    src/network/openjudgeclient.h
    src/parser/Homepage.cpp
    src/parser/Homepage.h
    src/parser/HtmlParserCommon.h
    src/parser/GroupHomepage.cpp
    src/parser/GroupHomepage.h
    src/parser/UserHomepage.cpp
    src/parser/UserHomepage.h
    src/parser/GroupPage.cpp
    src/parser/GroupPage.h
    src/parser/ContestPage.cpp
    src/parser/ContestPage.h
    src/parser/ProblemPage.cpp
    src/parser/ProblemPage.h
    src/parser/ResultPage.cpp
    src/parser/ResultPage.h
    src/parser/SubmitPage.cpp
    src/parser/SubmitPage.h
    src/repository/favorite/favoriteproblemrepository.cpp
    src/repository/favorite/favoriteproblemrepository.h
    src/repository/meta/problemmetarepository.cpp
    src/repository/meta/problemmetarepository.h
    src/login/logincache.cpp
    src/login/logincache.h
    src/login/loginservice.cpp
    src/login/loginservice.h
    src/login/emailverifyservice.cpp
    src/login/emailverifyservice.h
    src/app/applicationsizeservice.cpp
    src/app/applicationsizeservice.h
    src/service/reminder/deadlinealarmservice.cpp
    src/service/reminder/deadlinealarmservice.h
    src/service/reminder/reminderservice.cpp
    src/service/reminder/reminderservice.h
    src/submit/resultservice.cpp
    src/submit/resultservice.h
    src/submit/submitservice.cpp
    src/submit/submitservice.h
    src/browse/Group.cpp
    src/browse/Group.h
    src/browse/GroupHomepage.cpp
    src/browse/GroupHomepage.h
    src/browse/GroupPage.cpp
    src/browse/GroupPage.h
    src/browse/Homepage.cpp
    src/browse/Homepage.h
    src/browse/UserHomepage.cpp
    src/browse/UserHomepage.h
    src/browse/ContestPage.cpp
    src/browse/ContestPage.h
    src/browse/ProblemPage.cpp
    src/browse/ProblemPage.h
    src/cache/CacheService.cpp
    src/cache/CacheService.h
    src/cache/PageCacheStore.h
    src/cache/HomepageCache.cpp
    src/cache/HomepageCache.h
    src/cache/UserHomepageCache.cpp
    src/cache/UserHomepageCache.h
    src/cache/GroupHomepageCache.cpp
    src/cache/GroupHomepageCache.h
    src/cache/GroupPageCache.cpp
    src/cache/GroupPageCache.h
    src/cache/ContestPageCache.cpp
    src/cache/ContestPageCache.h
    src/cache/ProblemPageCache.cpp
    src/cache/ProblemPageCache.h
    src/service/favorite/favoriteproblemservice.cpp
    src/service/favorite/favoriteproblemservice.h
    src/service/meta/problemmetaservice.cpp
    src/service/meta/problemmetaservice.h
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
        lexbor_static
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

enable_testing()
file(TO_CMAKE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/html_examples" OJ_HTML_EXAMPLES_DIR)
qt_add_executable(parser_tests
    tests/parser/ParserTests.cpp
    src/parser/Homepage.cpp
    src/parser/GroupHomepage.cpp
    src/parser/UserHomepage.cpp
    src/parser/GroupPage.cpp
    src/parser/ContestPage.cpp
    src/parser/ProblemPage.cpp
    src/parser/ResultPage.cpp
    src/parser/SubmitPage.cpp
)
target_include_directories(parser_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/lexbor/source
)
target_compile_definitions(parser_tests PRIVATE
    OJ_HTML_EXAMPLES_DIR="${OJ_HTML_EXAMPLES_DIR}"
)
target_link_libraries(parser_tests PRIVATE Qt::Core lexbor_static)
add_test(NAME parser_tests COMMAND parser_tests)

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
