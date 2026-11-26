/**
 * @file Function.hpp
 * @brief TFunction type definition.
 */

#pragma once

#include <functional>

namespace Lumina
{

template <typename TFunc> using TFunction = std::function<TFunc>;

} // namespace Lumina
