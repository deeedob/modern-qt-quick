# SPDX-FileCopyrightText: 2026 Dennis Oberst
# SPDX-License-Identifier: MIT-0
#
# Layers, in dependency order. Each layer may only link the layers above it:
#   libs      pure C++, no Qt
#   platform  Qt without QML: OS integration, persistence, logging
#   backend   QML-facing C++ (Qt::Qml, never Qt::Quick): view models, stores, item models
#   frontend  QML views and visual C++ items (Qt::Quick)
#   app       the executable: composition root

add_subdirectory(libs)
add_subdirectory(platform)
add_subdirectory(backend)
add_subdirectory(frontend)
add_subdirectory(app)
