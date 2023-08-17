// Copyright 2023 TikTok Pte. Ltd.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <emmintrin.h>

#include <memory>
#include <stdexcept>
#include <vector>

#include "solo/prng.h"

#include "verse/util/defines.h"

namespace petace {
namespace verse {

inline block read_block_from_dev_urandom() {
    block ret;
    solo::PRNG::get_random_byte_array(sizeof(ret), reinterpret_cast<solo::Byte*>(&ret));
    return ret;
}

inline std::size_t bit_from_blocks(const std::vector<block>& input, std::size_t ids_of_bits) {
    const std::uint8_t* bits = reinterpret_cast<const std::uint8_t*>(input.data());
    std::size_t ret = static_cast<std::size_t>(bits[ids_of_bits / 8]) >> (ids_of_bits % 8);
    return ret & 1;
}

inline void matrix_transpose(
        const std::vector<block>& in, std::size_t rows, std::size_t cols, std::vector<block>& out) {
    if ((rows % 128 != 0) || (cols % 128 != 0)) {
        throw std::invalid_argument("Transpose size is not supported.");
    }

    char* ptr_in = reinterpret_cast<char*>(const_cast<block*>(in.data()));
    char* ptr_out = reinterpret_cast<char*>(const_cast<block*>(out.data()));

    auto f = [&](std::size_t x, std::size_t y) { return ptr_in[x * cols / 8 + y / 8]; };
    auto g = [&](std::size_t x, std::size_t y) { return &ptr_out[y * rows / 8 + x / 8]; };

    for (std::size_t i = 0; i < rows; i += 16) {
        for (std::size_t j = 0; j < cols; j += 8) {
            auto data = _mm_set_epi8(f(i + 15, j), f(i + 14, j), f(i + 13, j), f(i + 12, j), f(i + 11, j), f(i + 10, j),
                    f(i + 9, j), f(i + 8, j), f(i + 7, j), f(i + 6, j), f(i + 5, j), f(i + 4, j), f(i + 3, j),
                    f(i + 2, j), f(i + 1, j), f(i, j));

            for (std::size_t k = 8; k != 0; --k) {
                *reinterpret_cast<uint16_t*>(g(i, j + k - 1)) = static_cast<uint16_t>(_mm_movemask_epi8(data));
                data = _mm_slli_epi64(data, 1);
            }
        }
    }

    return;
}

inline void send_block(const std::shared_ptr<network::Network>& net, const block* data, std::size_t nblock) {
    net->send_data(data, nblock * sizeof(block));
}

inline void recv_block(const std::shared_ptr<network::Network>& net, block* data, std::size_t nblock) {
    net->recv_data(data, nblock * sizeof(block));
}

}  // namespace verse
}  // namespace petace
