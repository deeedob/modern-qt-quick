# A Qt Quick Controls style. Views import it instead of QtQuick.Controls: a compile-time choice,
# so bindings on controls compile to C++. Controls it does not define come from Basic.

set_source_files_properties(style/ApplicationWindow.qml PROPERTIES QT_RESOURCE_ALIAS ApplicationWindow.qml)
set_source_files_properties(style/Button.qml PROPERTIES QT_RESOURCE_ALIAS Button.qml)
set_source_files_properties(style/ComboBox.qml PROPERTIES QT_RESOURCE_ALIAS ComboBox.qml)
set_source_files_properties(style/Dialog.qml PROPERTIES QT_RESOURCE_ALIAS Dialog.qml)
set_source_files_properties(style/DialogButtonBox.qml PROPERTIES QT_RESOURCE_ALIAS DialogButtonBox.qml)
set_source_files_properties(style/ItemDelegate.qml PROPERTIES QT_RESOURCE_ALIAS ItemDelegate.qml)
set_source_files_properties(style/Slider.qml PROPERTIES QT_RESOURCE_ALIAS Slider.qml)
set_source_files_properties(style/SpinBox.qml PROPERTIES QT_RESOURCE_ALIAS SpinBox.qml)
set_source_files_properties(style/Switch.qml PROPERTIES QT_RESOURCE_ALIAS Switch.qml)
set_source_files_properties(style/TextField.qml PROPERTIES QT_RESOURCE_ALIAS TextField.qml)
set_source_files_properties(style/ToolButton.qml PROPERTIES QT_RESOURCE_ALIAS ToolButton.qml)
set_source_files_properties(style/ToolTip.qml PROPERTIES QT_RESOURCE_ALIAS ToolTip.qml)

set_source_files_properties(Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(app_controls
    URI App.Controls
    STATIC
    IMPORTS QtQuick.Controls.Basic
    QML_FILES
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
        Theme.qml
        LanguageSelector.qml
        style/ToolButton.qml
        style/ToolTip.qml
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
