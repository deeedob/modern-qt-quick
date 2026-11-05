// SPDX-FileCopyrightText: 2026 Dennis Oberst
// SPDX-License-Identifier: MIT-0

#pragma once

#include <string>

namespace app::core {

struct Task
{
    std::string title;
    bool done = false;

    friend bool operator==(const Task &, const Task &) = default;
};

} // namespace app::core
