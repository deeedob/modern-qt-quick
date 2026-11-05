# SPDX-FileCopyrightText: 2026 Dennis Oberst
# SPDX-License-Identifier: MIT-0

qt_add_executable(tst_frontend tst_frontend.cpp tst_IconButton.qml tst_ProgressRing.qml)
target_compile_definitions(tst_frontend PRIVATE QUICK_TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
# Static QML modules register themselves only when their plugin target is linked.
target_link_libraries(tst_frontend PRIVATE Qt::QuickTest app_frontend_uiplugin)
app_target_defaults(tst_frontend)
add_test(NAME tst_frontend COMMAND tst_frontend)
