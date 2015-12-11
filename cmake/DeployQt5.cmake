function(install_qt5_file srcPath fileName destPath)
    set(QT_PREFIX "")

    if(CMAKE_BUILD_TYPE STREQUAL Debug)
        set(QT_PREFIX "d")
    endif(CMAKE_BUILD_TYPE STREQUAL Debug)

    install(FILES "${srcPath}/${fileName}${QT_PREFIX}.dll" DESTINATION ${destPath} COMPONENT Runtime)
endfunction(install_qt5_file)

function(install_qt5_lib DEST)
    set (FilesToInstall)
    foreach(Qt5Lib ${ARGN})
        get_target_property(Qt5LibLocation Qt5::${Qt5Lib} LOCATION_${CMAKE_BUILD_TYPE})
        set (FilesToInstall ${FilesToInstall} ${Qt5LibLocation})
    endforeach(Qt5Lib ${ARGN})
    install(FILES ${FilesToInstall} DESTINATION ${DEST})
endfunction(install_qt5_lib)

function(install_qt5_dbdrivers dest)
    foreach(driver ${ARGN})
        install_qt5_file(${_qt5Core_install_prefix}/plugins/sqldrivers ${driver}  ${dest}/sqldrivers)
    endforeach(driver)
endfunction(install_qt5_dbdrivers)

function(install_qt5_imageformats dest)
    foreach(imgformat ${ARGN})
        install_qt5_file(${_qt5Core_install_prefix}/plugins/imageformats ${imgformat}  ${dest}/imageformats)
    endforeach(imgformat)
endfunction(install_qt5_imageformats)

function(install_qt5_platform dest)
    install_qt5_file(${_qt5Core_install_prefix}/plugins/platforms "qwindows" ${dest}/platforms)
endfunction(install_qt5_platform)

function(install_qt5_install_glob_files relpath dest expr)
    file(GLOB_RECURSE _QML_FILES "${_qt5Core_install_prefix}/qml/${relpath}/${expr}")
    foreach(_FILE ${_QML_FILES})
        string(LENGTH ${_qt5Core_install_prefix}/qml _FILE_LENGTH)
        string(SUBSTRING ${_FILE} ${_FILE_LENGTH} -1 _RELATIVE_FILE)
        get_filename_component(_RELATIVE_DIR ${_RELATIVE_FILE} DIRECTORY)
        install(FILES ${_FILE} DESTINATION ${dest}${_RELATIVE_DIR})
    endforeach()
endfunction(install_qt5_install_glob_files)

function(_install_qt5_qml_files relpath dest)
    install_qt5_install_glob_files(${relpath} ${dest} "*.qml")
    install_qt5_install_glob_files(${relpath} ${dest} "*.js")
    install_qt5_install_glob_files(${relpath} ${dest} "*.png")
    install_qt5_install_glob_files(${relpath} ${dest} "qmldir")
    install_qt5_install_glob_files(${relpath} ${dest} "plugins.qmltypes")
    install_qt5_install_glob_files(${relpath} ${dest} "*.ttf")
endfunction(_install_qt5_qml_files)

function(install_qt5_qml_plugin_qml dest)
    _install_qt5_qml_plugin("QtQml/Models.2" "modelsplugin" ${dest})
    _install_qt5_qml_files("QtQml/Models.2" ${dest})
endfunction(install_qt5_qml_plugin_qml)

function(install_qt5_qml_plugin_qtmultimedia dest)
    _install_qt5_qml_plugin("QtMultimedia" "declarative_multimedia" ${dest})
    _install_qt5_qml_files("QtMultimedia" ${dest})
endfunction(install_qt5_qml_plugin_qtmultimedia)

function(install_qt5_qml_plugin_qtquick2 dest)
    _install_qt5_qml_plugin("QtQuick.2" "qtquick2plugin" ${dest})
    _install_qt5_qml_files("QtQuick.2" ${dest})
endfunction(install_qt5_qml_plugin_qtquick2)

function(install_qt5_qml_plugin_qtquick_controls dest)
    _install_qt5_qml_plugin("QtQuick/Controls" "qtquickcontrolsplugin" ${dest})
    _install_qt5_qml_files("QtQuick/Controls" ${dest})
endfunction(install_qt5_qml_plugin_qtquick_controls)

function(install_qt5_qml_plugin_qtquick_layouts dest)
	_install_qt5_qml_plugin("QtQuick/Layouts" "qquicklayoutsplugin" ${dest})
    _install_qt5_qml_files("QtQuick/Layouts" ${dest})
endfunction(install_qt5_qml_plugin_qtquick_layouts)

function(install_qt5_qml_plugin_qtquick_dialogs dest)
	_install_qt5_qml_plugin("QtQuick/Dialogs" "dialogplugin" ${dest})
    _install_qt5_qml_plugin("QtQuick/Dialogs/Private" "dialogsprivateplugin" ${dest})
    _install_qt5_qml_files("QtQuick/Dialogs" ${dest})
endfunction(install_qt5_qml_plugin_qtquick_dialogs)

function(install_qt5_qml_plugin_qtquick_localstorage dest)
	_install_qt5_qml_plugin("QtQuick/LocalStorage" "qmllocalstorageplugin" ${dest})
    _install_qt5_qml_files("QtQuick/LocalStorage" ${dest})
endfunction(install_qt5_qml_plugin_qtquick_localstorage)

function(install_qt5_qml_plugin_qtquick_particles dest)
	_install_qt5_qml_plugin("QtQuick/Particles.2" "particlesplugin" ${dest})
    _install_qt5_qml_files("QtQuick/Particles.2" ${dest})
endfunction(install_qt5_qml_plugin_qtquick_particles)

function(install_qt5_qml_plugin_qtquick_privatewidgets dest)
	_install_qt5_qml_plugin("QtQuick/PrivateWidgets" "widgetsplugin" ${dest})
    _install_qt5_qml_files("QtQuick/PrivateWidgets" ${dest})
endfunction(install_qt5_qml_plugin_qtquick_privatewidgets)

function(install_qt5_qml_plugin_qtquick_window dest)
	_install_qt5_qml_plugin("QtQuick/Window.2" "windowplugin" ${dest})
    _install_qt5_qml_files("QtQuick/Window.2" ${dest})
endfunction(install_qt5_qml_plugin_qtquick_window)

function(install_qt5_qml_plugin_qtquick_xmllistmodel dest)
	_install_qt5_qml_plugin("QtQuick/XmlListModel" "qmlxmllistmodelplugin" ${dest})
    _install_qt5_qml_files("QtQuick/XmlListModel" ${dest})
endfunction(install_qt5_qml_plugin_qtquick_xmllistmodel)

function(install_qt5_qml_plugin_qt_labs_folderlistmodel dest)
	_install_qt5_qml_plugin("Qt/labs/folderlistmodel" "qmlfolderlistmodelplugin" ${dest})
    _install_qt5_qml_files("Qt/labs/folderlistmodel" ${dest})
endfunction(install_qt5_qml_plugin_qtquick_xmllistmodel)

function(install_qt5_qml_plugin_qt_labs_settings dest)
	_install_qt5_qml_plugin("Qt/labs/settings" "qmlsettingsplugin" ${dest})
    _install_qt5_qml_files("Qt/labs/settings" ${dest})
endfunction(install_qt5_qml_plugin_qtquick_xmllistmodel)

function(install_qt5_qml_plugin_all dest)
    install_qt5_qml_plugin_qml(${dest})
    install_qt5_qml_plugin_qtmultimedia(${dest})
    install_qt5_qml_plugin_qtquick_controls(${dest})
    install_qt5_qml_plugin_qtquick_dialogs(${dest})
    install_qt5_qml_plugin_qtquick_dialogs(${dest})
    install_qt5_qml_plugin_qtquick_layouts(${dest})
    install_qt5_qml_plugin_qtquick_localstorage(${dest})
    install_qt5_qml_plugin_qtquick_particles(${dest})
    install_qt5_qml_plugin_qtquick_privatewidgets(${dest})
    install_qt5_qml_plugin_qtquick_window(${dest})
    install_qt5_qml_plugin_qtquick_xmllistmodel(${dest})
    install_qt5_qml_plugin_qtquick2(${dest})
	install_qt5_qml_plugin_qt_labs_folderlistmodel(${dest})
	install_qt5_qml_plugin_qt_labs_settings(${dest})
endfunction(install_qt5_qml_plugin_all)

function(_install_qt5_qml_plugin relpath name dest)
    install_qt5_file(${_qt5Core_install_prefix}/qml/${relpath} ${name} ${dest}/${relpath})
endfunction(_install_qt5_qml_plugin)

function(install_qt5_V8 dest)
    install_qt5_file(${_qt5Core_install_prefix}/bin Qt5V8 ${dest})
endfunction(install_qt5_V8)

function(install_qt5_icu dest)
    set(_icu_version_str "52")
    if(Qt5Core_VERSION_MINOR EQUAL 4)
        set(_icu_version_str "53")
    elseif(Qt5Core_VERSION_MINOR EQUAL 5)
        set(_icu_version_str "54")
    endif()
    install(FILES ${_qt5Core_install_prefix}/bin/icudt${_icu_version_str}.dll
              ${_qt5Core_install_prefix}/bin/icuin${_icu_version_str}.dll
              ${_qt5Core_install_prefix}/bin/icuuc${_icu_version_str}.dll
              DESTINATION ${dest})
endfunction(install_qt5_icu)

function(install_qt_mingw_rt dest)
    install(FILES ${_qt5Core_install_prefix}/bin/libgcc_s_dw2-1.dll
                  ${_qt5Core_install_prefix}/bin/libstdc++-6.dll
                  ${_qt5Core_install_prefix}/bin/libwinpthread-1.dll
            DESTINATION ${dest})
endfunction(install_qt_mingw_rt)


function(install_qt5_translations locale dest)
    file(GLOB _TR_FILES "${_qt5Core_install_prefix}/translations/*_${locale}.qm")
    install(FILES ${_TR_FILES}
            DESTINATION ${dest})
endfunction()
