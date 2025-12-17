#pragma once

enum class CmpOrder {
    // tells that order is [0] = first, [1] = second
    FirstIsBeforeSecond,
    // tells that order is [0] = second, [1] = first
    // or that first == second
    FirstIsNotBeforeSecond
};
