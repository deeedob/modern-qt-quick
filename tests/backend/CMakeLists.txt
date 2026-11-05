# SPDX-FileCopyrightText: 2026 Dennis Oberst
# SPDX-License-Identifier: MIT-0

app_add_qt_test(tst_TasksViewModel
    SOURCES tst_TasksViewModel.cpp InMemoryTaskRepository.hpp
    LIBRARIES app_backend_tasks
)
app_add_qt_test(tst_TimerStore SOURCES tst_TimerStore.cpp LIBRARIES app_backend_timer)
