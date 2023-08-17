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

#include "verse/base-ot/naor-pinkas-ot/naor_pinkas_ot.h"

#include "verse/util/common.h"
#include "verse/verse_factory.h"

namespace petace {
namespace verse {

void NaorPinkasSender::send(const std::shared_ptr<network::Network>& net, std::vector<std::array<block, 2>>& messages) {
    std::vector<EC::SecretKey> c_sk(base_ot_sizes_);
    std::vector<std::shared_ptr<EC::Point>> c_pk(base_ot_sizes_);
    for (std::size_t i = 0; i < base_ot_sizes_; i++) {
        c_pk[i] = std::make_shared<EC::Point>(*ec_);
    }

    for (std::size_t i = 0; i < base_ot_sizes_; i++) {
        ec_->create_secret_key(prng_, c_sk[i]);
        ec_->create_public_key(c_sk[i], *c_pk[i]);
    }

    std::vector<EC::SecretKey> gr_sk(base_ot_sizes_);
    std::vector<std::shared_ptr<EC::Point>> gr_pk(base_ot_sizes_);
    for (std::size_t i = 0; i < base_ot_sizes_; i++) {
        gr_pk[i] = std::make_shared<EC::Point>(*ec_);
    }
    for (std::size_t i = 0; i < base_ot_sizes_; i++) {
        ec_->create_secret_key(prng_, gr_sk[i]);
        ec_->create_public_key(gr_sk[i], *gr_pk[i]);
    }

    std::vector<solo::Byte> buff(2 * base_ot_sizes_ * kEccPointLen);
    for (std::size_t i = 0; i < base_ot_sizes_; i++) {
        ec_->point_to_bytes(*c_pk[i], kEccPointLen, buff.data() + i * kEccPointLen);
    }
    for (std::size_t i = 0; i < base_ot_sizes_; i++) {
        ec_->point_to_bytes(*gr_pk[i], kEccPointLen, buff.data() + i * kEccPointLen + base_ot_sizes_ * kEccPointLen);
    }
    net->send_data(buff.data(), buff.size());

    std::vector<std::shared_ptr<EC::Point>> pk0_pk(base_ot_sizes_);
    for (std::size_t i = 0; i < base_ot_sizes_; i++) {
        pk0_pk[i] = std::make_shared<EC::Point>(*ec_);
    }
    std::vector<std::shared_ptr<EC::Point>> pk0_r_pk(base_ot_sizes_);
    for (std::size_t i = 0; i < base_ot_sizes_; i++) {
        pk0_r_pk[i] = std::make_shared<EC::Point>(*ec_);
    }
    net->recv_data(buff.data(), base_ot_sizes_ * kEccPointLen);
    for (std::size_t i = 0; i < base_ot_sizes_; i++) {
        ec_->point_from_bytes(buff.data() + i * kEccPointLen, kEccPointLen, *pk0_pk[i]);
        ec_->encrypt(*pk0_pk[i], gr_sk[i], *pk0_r_pk[i]);
    }

    std::vector<std::shared_ptr<EC::Point>> c_r_pk(base_ot_sizes_);
    for (std::size_t i = 0; i < base_ot_sizes_; i++) {
        c_r_pk[i] = std::make_shared<EC::Point>(*ec_);
        ec_->encrypt(*c_pk[i], gr_sk[i], *c_r_pk[i]);
    }

    std::vector<std::shared_ptr<EC::Point>> pk1_r_pk(base_ot_sizes_);
    for (std::size_t i = 0; i < base_ot_sizes_; i++) {
        pk1_r_pk[i] = std::make_shared<EC::Point>(*ec_);
    }

    for (std::size_t i = 0; i < base_ot_sizes_; i++) {
        ec_->invert(*pk0_r_pk[i], *pk1_r_pk[i]);
        ec_->add(*c_r_pk[i], *pk1_r_pk[i], *pk1_r_pk[i]);
    }

    std::vector<solo::Byte> msg(2 * base_ot_sizes_ * kEccPointLen);
    for (std::size_t i = 0; i < base_ot_sizes_; i++) {
        ec_->point_to_bytes(*pk0_r_pk[i], kEccPointLen, msg.data() + i * kEccPointLen);
    }
    for (std::size_t i = 0; i < base_ot_sizes_; i++) {
        ec_->point_to_bytes(*pk1_r_pk[i], kEccPointLen, msg.data() + i * kEccPointLen + base_ot_sizes_ * kEccPointLen);
        msg[i * kEccPointLen + base_ot_sizes_ * kEccPointLen] = static_cast<solo::Byte>(
                static_cast<unsigned char>(msg[i * kEccPointLen + base_ot_sizes_ * kEccPointLen]) ^ 1);
    }

    auto hash = solo::Hash::create(solo::HashScheme::SHA_256);
    messages.resize(base_ot_sizes_);
    for (std::size_t i = 0; i < base_ot_sizes_; i++) {
        hash->compute(msg.data() + i * kEccPointLen, kEccPointLen, reinterpret_cast<solo::Byte*>(&messages[i][0]),
                sizeof(block));
        hash->compute(msg.data() + i * kEccPointLen + base_ot_sizes_ * kEccPointLen, kEccPointLen,
                reinterpret_cast<solo::Byte*>(&messages[i][1]), sizeof(block));
    }
    return;
}

void NaorPinkasReceiver::receive(
        const std::shared_ptr<network::Network>& net, const std::vector<block>& choices, std::vector<block>& messages) {
    std::vector<EC::SecretKey> c_sk(base_ot_sizes_);
    std::vector<std::shared_ptr<EC::Point>> c_pk(base_ot_sizes_);
    for (std::size_t i = 0; i < base_ot_sizes_; i++) {
        c_pk[i] = std::make_shared<EC::Point>(*ec_);
    }

    std::vector<EC::SecretKey> gr_sk(base_ot_sizes_);
    std::vector<std::shared_ptr<EC::Point>> gr_pk(base_ot_sizes_);
    for (std::size_t i = 0; i < base_ot_sizes_; i++) {
        gr_pk[i] = std::make_shared<EC::Point>(*ec_);
    }

    std::vector<solo::Byte> buff(2 * base_ot_sizes_ * kEccPointLen);
    net->recv_data(buff.data(), buff.size());
    for (std::size_t i = 0; i < base_ot_sizes_; i++) {
        ec_->point_from_bytes(buff.data() + i * kEccPointLen, kEccPointLen, *c_pk[i]);
    }
    for (std::size_t i = 0; i < base_ot_sizes_; i++) {
        ec_->point_from_bytes(buff.data() + i * kEccPointLen + base_ot_sizes_ * kEccPointLen, kEccPointLen, *gr_pk[i]);
    }

    std::vector<EC::SecretKey> k_sigma_sk(base_ot_sizes_);
    std::vector<std::shared_ptr<EC::Point>> k_sigma_pk(base_ot_sizes_);
    for (std::size_t i = 0; i < base_ot_sizes_; i++) {
        k_sigma_pk[i] = std::make_shared<EC::Point>(*ec_);
    }

    for (std::size_t i = 0; i < base_ot_sizes_; i++) {
        ec_->create_secret_key(prng_, k_sigma_sk[i]);
        ec_->create_public_key(k_sigma_sk[i], *k_sigma_pk[i]);
    }

    for (std::size_t i = 0; i < base_ot_sizes_; i++) {
        if (bit_from_blocks(choices, i)) {
            ec_->invert(*k_sigma_pk[i], *k_sigma_pk[i]);
            ec_->add(*k_sigma_pk[i], *c_pk[i], *k_sigma_pk[i]);
        }
    }

    for (std::size_t i = 0; i < base_ot_sizes_; i++) {
        ec_->point_to_bytes(*k_sigma_pk[i], kEccPointLen, buff.data() + i * kEccPointLen);
    }
    net->send_data(buff.data(), base_ot_sizes_ * kEccPointLen);

    for (std::size_t i = 0; i < base_ot_sizes_; i++) {
        ec_->encrypt(*gr_pk[i], k_sigma_sk[i], *gr_pk[i]);
    }

    std::vector<solo::Byte> msg(base_ot_sizes_ * kEccPointLen);
    for (std::size_t i = 0; i < base_ot_sizes_; i++) {
        ec_->point_to_bytes(*gr_pk[i], kEccPointLen, msg.data() + i * kEccPointLen);
        msg[i * kEccPointLen] =
                static_cast<solo::Byte>(static_cast<int>(msg[i * kEccPointLen]) ^ bit_from_blocks(choices, i));
    }

    auto hash = solo::Hash::create(solo::HashScheme::SHA_256);
    messages.resize(base_ot_sizes_);
    for (std::size_t i = 0; i < base_ot_sizes_; i++) {
        hash->compute(msg.data() + i * kEccPointLen, kEccPointLen, reinterpret_cast<solo::Byte*>(&messages[i]),
                sizeof(block));
    }
    return;
}

}  // namespace verse
}  // namespace petace
