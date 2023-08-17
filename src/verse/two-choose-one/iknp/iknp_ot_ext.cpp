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

#include "verse/two-choose-one/iknp/iknp_ot_ext.h"

#include <cstring>
#include <stdexcept>

#include "verse/util/common.h"

namespace petace {
namespace verse {

void IknpOtExtSender::set_base_ots(const std::vector<block>& choices, const std::vector<block>& base_recv_ots) {
    solo::PRNGFactory prng_factory(solo::PRNGScheme::AES_ECB_CTR);
    for (std::size_t i = 0; i < base_recv_ots.size(); i++) {
        std::vector<solo::Byte> seed(sizeof(block));
        memcpy(seed.data(), reinterpret_cast<solo::Byte*>(const_cast<block*>(base_recv_ots.data() + i)), sizeof(block));
        prng_.emplace_back(prng_factory.create(seed));
    }
    base_choices_.insert(base_choices_.begin(), choices.begin(), choices.end());
    return;
}

void IknpOtExtSender::send(const std::shared_ptr<network::Network>& net, std::vector<std::array<block, 2>>& messages) {
    if (base_ot_sizes_ > 128) {
        throw std::invalid_argument("IKNP is only supported by 128-bit base-OT.");
    }
    if (base_ot_sizes_ % (sizeof(block) * 8) != 0) {
        throw std::invalid_argument("OT base size is not supported.");
    }
    if (ext_ot_sizes_ % (sizeof(block) * 8) != 0) {
        throw std::invalid_argument("OT extension size is not supported.");
    }

    std::size_t rows = base_ot_sizes_;
    std::size_t cols = ext_ot_sizes_ / (sizeof(block) * 8);

    std::vector<block> recv_matrix(rows * cols);
    recv_block(net, recv_matrix.data(), rows * cols);

    std::vector<block> ext_matrix(rows * cols);
    for (std::size_t i = 0; i < rows; i++) {
        prng_[i]->generate(cols * sizeof(block), reinterpret_cast<solo::Byte*>(&(ext_matrix[i * cols])));
        if (bit_from_blocks(base_choices_, i)) {
            for (std::size_t j = 0; j < cols; j++) {
                ext_matrix[i * cols + j] ^= recv_matrix[i * cols + j];
            }
        }
    }

    std::vector<block> input(rows);
    std::vector<std::vector<block>> output(cols, std::vector<block>(rows));
    for (std::size_t i = 0; i < cols; i++) {
        for (std::size_t j = 0; j < rows; j++) {
            input[j] = ext_matrix[j * cols + i];
        }
        matrix_transpose(input, rows, rows, output[i]);
    }

    auto hash = solo::Hash::create(solo::HashScheme::SHA_256);
    messages.resize(ext_ot_sizes_);
    for (std::size_t i = 0; i < ext_ot_sizes_; i++) {
        output[i / rows][i % rows] ^= _mm_set_epi64x(0, i);
        hash->compute(reinterpret_cast<solo::Byte*>(&output[i / rows][i % rows]), sizeof(block),
                reinterpret_cast<solo::Byte*>(&messages[i][0]), sizeof(block));
        output[i / rows][i % rows] ^= base_choices_.front();
        hash->compute(reinterpret_cast<solo::Byte*>(&output[i / rows][i % rows]), sizeof(block),
                reinterpret_cast<solo::Byte*>(&messages[i][1]), sizeof(block));
    }

    return;
}

void IknpOtExtReceiver::set_base_ots(const std::vector<std::array<block, 2>>& base_send_ots) {
    solo::PRNGFactory prng_factory(solo::PRNGScheme::AES_ECB_CTR);
    for (std::size_t i = 0; i < base_send_ots.size(); i++) {
        std::vector<solo::Byte> seed0(sizeof(block));
        std::vector<solo::Byte> seed1(sizeof(block));
        memcpy(seed0.data(), reinterpret_cast<solo::Byte*>(const_cast<block*>(&base_send_ots[i][0])), sizeof(block));
        memcpy(seed1.data(), reinterpret_cast<solo::Byte*>(const_cast<block*>(&base_send_ots[i][1])), sizeof(block));
        std::array<std::shared_ptr<solo::PRNG>, 2> values{prng_factory.create(seed0), prng_factory.create(seed1)};
        prng_.emplace_back(values);
    }
    return;
}

void IknpOtExtReceiver::receive(
        const std::shared_ptr<network::Network>& net, const std::vector<block>& choices, std::vector<block>& messages) {
    if (base_ot_sizes_ % (sizeof(block) * 8) != 0) {
        throw std::invalid_argument("OT base size is not supported.");
    }
    if (ext_ot_sizes_ % (sizeof(block) * 8) != 0) {
        throw std::invalid_argument("OT extension size is not supported.");
    }

    std::size_t rows = base_ot_sizes_;
    std::size_t cols = ext_ot_sizes_ / (sizeof(block) * 8);

    std::vector<block> t0(rows * cols);
    std::vector<block> t1(rows * cols);
    std::vector<block> send_matrix(rows * cols);
    for (std::size_t i = 0; i < rows; i++) {
        prng_[i][0]->generate(cols * sizeof(block), reinterpret_cast<solo::Byte*>(&(t0[i * cols])));
        prng_[i][1]->generate(cols * sizeof(block), reinterpret_cast<solo::Byte*>(&(t1[i * cols])));

        for (std::size_t j = 0; j < cols; j++) {
            send_matrix[i * cols + j] = t0[i * cols + j] ^ t1[i * cols + j] ^ choices[j];
        }
    }

    send_block(net, send_matrix.data(), rows * cols);

    std::vector<block> input(rows);
    std::vector<std::vector<block>> output(cols, std::vector<block>(rows));
    for (std::size_t i = 0; i < cols; i++) {
        for (std::size_t j = 0; j < rows; j++) {
            input[j] = t0[j * cols + i];
        }
        matrix_transpose(input, rows, rows, output[i]);
    }

    auto hash = solo::Hash::create(solo::HashScheme::SHA_256);
    messages.resize(ext_ot_sizes_);
    for (std::size_t i = 0; i < ext_ot_sizes_; i++) {
        output[i / rows][i % rows] ^= _mm_set_epi64x(0, i);
        hash->compute(reinterpret_cast<solo::Byte*>(&output[i / rows][i % rows]), sizeof(block),
                reinterpret_cast<solo::Byte*>(&messages[i]), sizeof(block));
    }

    return;
}

}  // namespace verse
}  // namespace petace
