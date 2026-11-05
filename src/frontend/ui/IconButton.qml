// SPDX-FileCopyrightText: 2026 Dennis Oberst
// SPDX-License-Identifier: MIT-0

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Material

// A pure QML component: a small typed API over a standard control, which tints the SVG icon.
ToolButton {
    id: root

    required property string iconName
    property string toolTip

    display: AbstractButton.IconOnly
    icon.source: Theme.icon(root.iconName)
    icon.width: 20
    icon.height: 20
    text: root.toolTip
    ToolTip.visible: root.toolTip.length > 0 && root.hovered
    ToolTip.text: root.toolTip
    ToolTip.delay: 500
}
