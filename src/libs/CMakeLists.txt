# SPDX-FileCopyrightText: 2026 Dennis Oberst
# SPDX-License-Identifier: MIT-0

# Everything below libs/ is plain C++: no moc, no uic, no Qt.
set(CMAKE_AUTOMOC OFF)
set(CMAKE_AUTOUIC OFF)

add_subdirectory(core)
