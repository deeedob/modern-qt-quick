// SPDX-FileCopyrightText: 2026 Dennis Oberst
// SPDX-License-Identifier: MIT-0

pragma Singleton

import QtQuick

// Design tokens. A singleton fits: the values are the same everywhere and hold no app state.
QtObject {
    readonly property color accent: "#5b5bd6"
    readonly property int spacing: 12
    readonly property string fontFamily: interFont.name

    function icon(name: string): url {
        return Qt.resolvedUrl(`icons/${name}.svg`);
    }

    readonly property FontLoader interFont: FontLoader {
        source: Qt.resolvedUrl("fonts/InterVariable.ttf")
    }
}
