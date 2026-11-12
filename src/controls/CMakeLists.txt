# A Qt Quick Controls style. Views import it instead of QtQuick.Controls: a compile-time choice,
# so bindings on controls compile to C++. Controls it does not define come from Basic.

set(_style_files
    style/ApplicationWindow.qml
    style/Button.qml
    style/ComboBox.qml
    style/Dialog.qml
    style/DialogButtonBox.qml
    style/ItemDelegate.qml
    style/Slider.qml
    style/SpinBox.qml
    style/Switch.qml
    style/TextField.qml
    style/ToolButton.qml
    style/ToolTip.qml
)
# Style controls must be at the module resource root to override the Basic controls.
foreach(_style_file IN LISTS _style_files)
    get_filename_component(_style_name "${_style_file}" NAME)
    set_source_files_properties("${_style_file}" PROPERTIES QT_RESOURCE_ALIAS "${_style_name}")
endforeach()

set_source_files_properties(Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(app_controls
    URI App.Controls
    STATIC
    IMPORTS QtQuick.Controls.Basic
    QML_FILES
        ${_style_files}
        Theme.qml
        LanguageSelector.qml
    RESOURCES
        fonts/InterVariable.ttf
        icons/arrow-left.svg
        icons/book-open.svg
        icons/copy.svg
        icons/eraser.svg
        icons/external-link.svg
        icons/info.svg
        icons/languages.svg
        icons/link-2.svg
        icons/moon.svg
        icons/pencil.svg
        icons/plus.svg
        icons/redo-2.svg
        icons/settings-2.svg
        icons/sun.svg
        icons/trash.svg
        icons/undo-2.svg
        icons/unlink-2.svg
)
target_link_libraries(app_controls PRIVATE Qt::Quick Qt::QuickControls2)
app_target_defaults(app_controls)

app_add_notice(inter-font fonts/LICENSE.txt)
app_add_notice(lucide-icons icons/LICENSE)
