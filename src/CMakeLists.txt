
set(STATEEDITOR_APPNAME StateEditor)

add_subdirectory(QsLog)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
 # Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Find the QtWidgets library
find_package(Qt5Core)
find_package(Qt5Gui)
find_package(Qt5Network)
find_package(Qt5Widgets)
find_package(Qt5WebKitWidgets)
find_package(Qt5XmlPatterns)

include(InstallRequiredSystemLibraries)

# enable warnings
#TODO
#add_definitions(-Wall)

# Qt designer ui file
set(QT_UIS 
    StateEditorMainWindow.ui
    AboutDialog.ui
)

# transform ui files into cpp code
qt5_wrap_ui(QT_UIS_H ${QT_UIS})

#Add the include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR} 
                    ${CMAKE_CURRENT_BINARY_DIR}
                    ${QSLOG_INCLUDE_DIR}
                    )

set(STATEEDITOR_SRCS 
    main.cpp 
    StateEditorMainWindow.cpp 
    StateEditorMainWindow.h
    AboutDialog.cpp
    AboutDialog.h
    GeneratorInfo.h
    GeneratorInfo.cpp
    StateEditorDefs.h
)


if(WIN32)
  set(STATEEDITOR_SRCS ${STATEEDITOR_SRCS} StateEditor.rc)
endif(WIN32)
IF(APPLE)
  set(STATEEDITOR_SRCS ${STATEEDITOR_SRCS} StateEditor.icns)
  set(MACOSX_BUNDLE_ICON_FILE StateEditor.icns)
  set_source_files_properties(StateEditor.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
endif(APPLE)

set(STATEEDITOR_RC_SRCS StateEditor.qrc)

qt5_add_resources(STATEEDITOR_RC_SRCS ${STATEEDITOR_RC_SRCS} )

add_executable(${STATEEDITOR_APPNAME} WIN32 MACOSX_BUNDLE 
   ${STATEEDITOR_SRCS} 
   ${QT_UIS_H}
   ${STATEEDITOR_RC_SRCS})

# Use the Widgets module from Qt 5.
qt5_use_modules(${STATEEDITOR_APPNAME} Widgets Gui WebKitWidgets XmlPatterns)
if(APPLE)
  set_target_properties(${STATEEDITOR_APPNAME} PROPERTIES OUTPUT_NAME ${CMAKE_BUNDLE_NAME})
endif(APPLE)
set(CMAKE_INSTALL_DESTINATION_ARGS BUNDLE DESTINATION "${CMAKE_BUNDLE_LOCATION}")

set(CMAKE_INSTALL_PREFIX ${STATEEDITOR_SOURCEDIR})


#--------------------------------------------------------------------------------
# Now the installation stuff below
#--------------------------------------------------------------------------------
set(plugin_dest_dir bin)
set(qtconf_dest_dir bin)
set(APPS "\${CMAKE_INSTALL_PREFIX}/bin/${STATEEDITOR_APPNAME}")
if(APPLE)
  SET(plugin_dest_dir ${STATEEDITOR_APPNAME}.app/Contents/MacOS)
  SET(qtconf_dest_dir ${STATEEDITOR_APPNAME}.app/Contents/Resources)
  SET(APPS "\${CMAKE_INSTALL_PREFIX}/${STATEEDITOR_APPNAME}.app")
endif(APPLE)
if(WIN32)
  set(APPS "\${CMAKE_INSTALL_PREFIX}/bin/${STATEEDITOR_APPNAME}.exe")
endif(WIN32)

install(TARGETS ${STATEEDITOR_APPNAME} 
    BUNDLE DESTINATION . COMPONENT Runtime
    RUNTIME DESTINATION bin COMPONENT Runtime)
    
target_link_libraries(${STATEEDITOR_APPNAME}
                      ${QSLOG_LIBRARY}
					  Qt5::WinMain)

add_test(StateEditor StateEditor)

#--------------------------------------------------------------------------------
# Install needed Qt plugins by copying directories from the qt installation
# One can cull what gets copied by using 'REGEX "..." EXCLUDE'
#INSTALL(DIRECTORY "${QT_PLUGINS_DIR}/imageformats" DESTINATION ${plugin_dest_dir}/plugins COMPONENT Runtime)

#--------------------------------------------------------------------------------
# install a qt.conf file
# this inserts some cmake code into the install script to write the file
INSTALL(CODE "
    file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${qtconf_dest_dir}/qt.conf\" \"\")
    " COMPONENT Runtime)

#--------------------------------------------------------------------------------
# Use BundleUtilities to get all other dependencies for the application to work.
# It takes a bundle or executable along with possible plugins and inspects it
# for dependencies.  If they are not system dependencies, they are copied.

# directories to look for dependencies
SET(DIRS ${QT_LIBRARY_DIRS})

# Now the work of copying dependencies into the bundle/package
# The quotes are escaped and variables to use at install time have their $ escaped
# An alternative is the do a configure_file() on a script and use install(SCRIPT  ...).
# Note that the image plugins depend on QtSvg and QtXml, and it got those copied
# over.
install(CODE "
    file(GLOB_RECURSE QTPLUGINS
      \"\${CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}/plugins/*${CMAKE_SHARED_LIBRARY_SUFFIX}\")
    include(BundleUtilities)
    fixup_bundle(\"${APPS}\" \"\${QTPLUGINS}\" \"${DIRS}\")
    " COMPONENT Runtime)


# To Create a package, one can run "cpack -G DragNDrop CPackConfig.cmake" on Mac OS X
# where CPackConfig.cmake is created by including CPack
# And then there's ways to customize this as well
set(CPACK_BINARY_DRAGNDROP ON)
include(CPack)

