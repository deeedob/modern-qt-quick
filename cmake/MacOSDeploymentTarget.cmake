# SPDX-FileCopyrightText: 2026 Dennis Oberst
# SPDX-License-Identifier: MIT-0
#
# Oldest macOS the app supports (Qt 6.12's minimum). Read by the top-level CMakeLists.txt for the
# app and by cmake/vcpkg-triplets/ for the dependencies, so both always agree.

set(APP_MACOS_DEPLOYMENT_TARGET "14.4")
