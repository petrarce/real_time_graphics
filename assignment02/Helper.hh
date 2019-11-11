#pragma once

#include <algorithm>
#include <memory>
#include <vector>

template <typename T, typename U>
void removeElement(std::vector<std::shared_ptr<T>>& vec, std::shared_ptr<U> const& e)
{
    auto ce = std::dynamic_pointer_cast<T>(e);
    if (ce)
        vec.erase(std::remove(vec.begin(), vec.end(), ce), vec.end());
}

// template <typename T>
// void removeElement(std::vector<T>& vec, T const& e)
// {
//     vec.erase(std::remove(vec.begin(), vec.end(), e), vec.end());
// }
