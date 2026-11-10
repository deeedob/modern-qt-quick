# A Qt Quick Controls style. Views import it instead of QtQuick.Controls: a compile-time choice,
# so bindings on controls compile to C++. Controls it does not define come from Basic.

set_source_files_properties(Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(app_controls
    URI App.Controls
    STATIC
    IMPORTS QtQuick.Controls.Basic
    QML_FILES
        ApplicationWindow.qml
        Button.qml
        Dialog.qml
        DialogButtonBox.qml
        ItemDelegate.qml
        TextField.qml
        Theme.qml
        ToolButton.qml
        ToolTip.qml
    RESOURCES
        fonts/InterVariable.ttf
        icons/arrow-left.svg
        icons/copy.svg
        icons/eraser.svg
        icons/info.svg
        icons/languages.svg
        icons/pencil.svg
        icons/plus.svg
        icons/redo-2.svg
        icons/trash.svg
        icons/undo-2.svg
)
target_link_libraries(app_controls PRIVATE Qt::Quick Qt::QuickControls2)
app_target_defaults(app_controls)

app_add_notice(inter-font fonts/LICENSE.txt)
app_add_notice(lucide-icons icons/LICENSE)
