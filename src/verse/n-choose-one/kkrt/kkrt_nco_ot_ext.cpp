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

#include "verse/n-choose-one/kkrt/kkrt_nco_ot_ext.h"

#include <cstring>

#include "verse/util/common.h"

namespace petace {
namespace verse {

void KkrtNcoOtExtSender::set_base_ots(const std::vector<block>& choices, const std::vector<block>& base_recv_ots) {
    solo::PRNGFactory prng_factory(solo::PRNGScheme::AES_ECB_CTR);
    for (std::size_t i = 0; i < base_recv_ots.size(); i++) {
        std::vector<solo::Byte> seed(sizeof(block));
        memcpy(seed.data(), reinterpret_cast<solo::Byte*>(const_cast<block*>(base_recv_ots.data() + i)), sizeof(block));
        prng_.emplace_back(prng_factory.create(seed));
    }
    base_choices_.insert(base_choices_.begin(), choices.begin(), choices.end());
    return;
}

void KkrtNcoOtExtSender::send(const std::shared_ptr<network::Network>& net, const std::size_t ext_ot_sizes) {
    if (base_ot_sizes_ % (sizeof(block) * 8) != 0) {
        throw std::invalid_argument("OT base size is not supported.");
    }
    ext_ot_sizes_ = ext_ot_sizes;
    if (ext_ot_sizes_ % (sizeof(block) * 8) != 0) {
        ext_ot_sizes_ += sizeof(block) * 8 - (ext_ot_sizes_ % (sizeof(block) * 8));
    }
    if (ext_ot_sizes_ % (sizeof(block) * 8) != 0) {
        throw std::invalid_argument("OT extension size is not supported.");
    }

    std::size_t rows = base_ot_sizes_;
    std::size_t cols = ext_ot_sizes_ / (sizeof(block) * 8);

    std::vector<block> recv_matrix(rows * cols);
    recv_block(net, recv_matrix.data(), rows * cols);

    std::vector<std::vector<block>> ext_matrix(rows, std::vector<block>(cols));
    for (std::size_t i = 0; i < rows; i++) {
        prng_[i]->generate(cols * sizeof(block), reinterpret_cast<solo::Byte*>(&(ext_matrix[i][0])));
    }

    std::vector<block> input(sizeof(block) * 8);
    std::vector<block> output(sizeof(block) * 8);
    std::size_t threshhold = rows / (sizeof(block) * 8);

    q_mat.resize(ext_ot_sizes_);
    for (std::size_t i = 0; i < ext_ot_sizes_; i++) {
        q_mat[i].resize(threshhold);
    }
    for (std::size_t i = 0; i < cols; i++) {
        for (std::size_t j = 0; j < threshhold; j++) {
            for (std::size_t k = 0; k < sizeof(block) * 8; k++) {
                input[k] = ext_matrix[j * sizeof(block) * 8 + k][i];
            }
            matrix_transpose(input, sizeof(block) * 8, sizeof(block) * 8, output);

            for (std::size_t k = 0; k < sizeof(block) * 8; k++) {
                q_mat[i * sizeof(block) * 8 + k][j] = output[k];
                q_mat[i * sizeof(block) * 8 + k][j] ^=
                        (recv_matrix[(i * sizeof(block) * 8 + k) * threshhold + j] & base_choices_[j]);
            }
        }
    }

    return;
}

void KkrtNcoOtExtSender::encode(const std::size_t idx, const block& input, block& output) {
    std::size_t threshhold = base_ot_sizes_ / (sizeof(block) * 8);
    std::vector<block> enc_input(threshhold);
    auto hash = solo::Hash::create(solo::HashScheme::SHA_256);

    for (std::size_t j = 0; j < threshhold; j++) {
        auto hash_in = input ^ _mm_set_epi64x(0, j);
        hash->compute(reinterpret_cast<solo::Byte*>(&hash_in), sizeof(block),
                reinterpret_cast<solo::Byte*>(&enc_input[j]), sizeof(block));
        enc_input[j] = base_choices_[j] & (enc_input[j] ^ input);
    }

    block enc_output = _mm_set_epi64x(0, 0);
    for (std::size_t j = 0; j < threshhold; j++) {
        enc_output ^= enc_input[j] ^ q_mat[idx][j] ^ _mm_set_epi64x(0, idx);
        hash->compute(reinterpret_cast<solo::Byte*>(&enc_output), sizeof(block),
                reinterpret_cast<solo::Byte*>(&enc_output), sizeof(block));
    }
    output = enc_output;
}

void KkrtNcoOtExtReceiver::set_base_ots(const std::vector<std::array<block, 2>>& base_send_ots) {
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

void KkrtNcoOtExtReceiver::receive(
        const std::shared_ptr<network::Network>& net, const std::vector<block>& choices, std::vector<block>& messages) {
    if (base_ot_sizes_ % (sizeof(block) * 8) != 0) {
        throw std::invalid_argument("OT base size is not supported.");
    }
    ext_ot_sizes_ = choices.size();
    if (ext_ot_sizes_ % (sizeof(block) * 8) != 0) {
        ext_ot_sizes_ += sizeof(block) * 8 - (ext_ot_sizes_ % (sizeof(block) * 8));
    }
    if (ext_ot_sizes_ % (sizeof(block) * 8) != 0) {
        throw std::invalid_argument("OT extension size is not supported.");
    }

    std::size_t rows = base_ot_sizes_;
    std::size_t cols = ext_ot_sizes_ / (sizeof(block) * 8);

    std::vector<std::vector<block>> t0(rows, std::vector<block>(cols));
    std::vector<std::vector<block>> t1(rows, std::vector<block>(cols));

    for (std::size_t i = 0; i < rows; i++) {
        prng_[i][0]->generate(cols * sizeof(block), reinterpret_cast<solo::Byte*>(&(t0[i][0])));
        prng_[i][1]->generate(cols * sizeof(block), reinterpret_cast<solo::Byte*>(&(t1[i][0])));
    }

    std::size_t threshhold = rows / (sizeof(block) * 8);
    std::vector<block> input(sizeof(block) * 8);
    std::vector<block> output(sizeof(block) * 8);

    std::vector<std::vector<block>> row_mat0(ext_ot_sizes_, std::vector<block>(threshhold));
    for (std::size_t i = 0; i < cols; i++) {
        for (std::size_t j = 0; j < threshhold; j++) {
            for (std::size_t k = 0; k < sizeof(block) * 8; k++) {
                input[k] = t0[j * sizeof(block) * 8 + k][i];
            }
            matrix_transpose(input, sizeof(block) * 8, sizeof(block) * 8, output);

            for (std::size_t k = 0; k < sizeof(block) * 8; k++) {
                row_mat0[i * sizeof(block) * 8 + k][j] = output[k];
            }
        }
    }

    std::vector<std::vector<block>> row_mat1(ext_ot_sizes_, std::vector<block>(threshhold));
    for (std::size_t i = 0; i < cols; i++) {
        for (std::size_t j = 0; j < threshhold; j++) {
            for (std::size_t k = 0; k < sizeof(block) * 8; k++) {
                input[k] = t1[j * sizeof(block) * 8 + k][i];
            }
            matrix_transpose(input, sizeof(block) * 8, sizeof(block) * 8, output);

            for (std::size_t k = 0; k < sizeof(block) * 8; k++) {
                row_mat1[i * sizeof(block) * 8 + k][j] = output[k];
            }
        }
    }

    auto hash = solo::Hash::create(solo::HashScheme::SHA_256);
    std::vector<block> row_mat(ext_ot_sizes_ * threshhold);
    for (std::size_t i = 0; i < ext_ot_sizes_; i++) {
        for (std::size_t j = 0; j < threshhold; j++) {
            if (i < choices.size()) {
                auto hash_in = choices[i] ^ _mm_set_epi64x(0, j);
                hash->compute(reinterpret_cast<solo::Byte*>(&hash_in), sizeof(block),
                        reinterpret_cast<solo::Byte*>(&row_mat[i * threshhold + j]), sizeof(block));
                row_mat[i * threshhold + j] ^= row_mat0[i][j] ^ row_mat1[i][j] ^ choices[i];
            } else {
                auto hash_in = _mm_set_epi64x(0, j);
                hash->compute(reinterpret_cast<solo::Byte*>(&hash_in), sizeof(block),
                        reinterpret_cast<solo::Byte*>(&row_mat[i * threshhold + j]), sizeof(block));
                row_mat[i * threshhold + j] ^= row_mat0[i][j] ^ row_mat1[i][j];
            }
        }
    }

    send_block(net, row_mat.data(), ext_ot_sizes_ * threshhold);

    messages.resize(choices.size());
    for (std::size_t i = 0; i < choices.size(); i++) {
        block hash_in = _mm_set_epi64x(0, 0);
        for (std::size_t j = 0; j < threshhold; j++) {
            hash_in ^= row_mat0[i][j] ^ _mm_set_epi64x(0, i);
            hash->compute(reinterpret_cast<solo::Byte*>(&hash_in), sizeof(block),
                    reinterpret_cast<solo::Byte*>(&hash_in), sizeof(block));
        }
        messages[i] = hash_in;
    }

    return;
}

}  // namespace verse
}  // namespace petace
