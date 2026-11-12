#pragma once

#include "common/primitive_types.h"
#include <vector>
#include <algorithm>

namespace ff::utils {
// void
// unordered_erase(std::vector<T> &vec, size_t index) {
//     if (index >= vec.size()) return;

//     // Move the last element into the spot of the element being removed
//     vec[index] = std::move(vec.back());

//     // Remove the last element in O(1) time
//     vec.pop_back();
// }

template<typename T>
void erase_unordered(std::vector<T>& vec, size_t index) {
    if (index >= vec.size()) return;
    
    // Move the last element into the spot of the element being removed
    vec[index] = std::move(vec.back());
    
    // Remove the last element in O(1) time
    vec.pop_back();
}
} // namespace ff::utils