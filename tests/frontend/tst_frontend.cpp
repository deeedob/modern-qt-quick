// SPDX-FileCopyrightText: 2026 Dennis Oberst
// SPDX-License-Identifier: MIT-0

#include <QtQuickTest/quicktest.h>

// Runs every tst_*.qml file in QUICK_TEST_SOURCE_DIR. The QML modules under test are linked
// statically, so their imports resolve from the embedded qrc:/qt/qml resources.
QUICK_TEST_MAIN(frontend)
