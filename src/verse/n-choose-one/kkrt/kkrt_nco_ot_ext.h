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

#include <memory>
#include <string>
#include <vector>

#include "network/network.h"

#include "verse/n-choose-one/nco_ot_ext_receiver.h"
#include "verse/n-choose-one/nco_ot_ext_sender.h"
#include "verse/util/defines.h"

namespace petace {
namespace verse {

/**
 * @brief 1-out-of-n kkrt ot extension [sender].
 *
 * @see Refer to README.md for more details.
 *
 * @par Example.
 * Refer to example.cpp.
 */
class KkrtNcoOtExtSender : public NcoOtExtSender {
public:
    explicit KkrtNcoOtExtSender(const std::size_t base_ot_sizes) : NcoOtExtSender(base_ot_sizes) {
    }

    ~KkrtNcoOtExtSender() {
    }

    /**
     * @brief The sender sets the base ots that are used to extend.
     *
     * @param[in] choices The chosen bits in the base ot.
     * @param[in] base_send_ots Base OTs that are used for kkrt ot extension.
     */
    void set_base_ots(const std::vector<block>& choices, const std::vector<block>& base_recv_ots) override;

    /**
     * @brief The sender gets the random keys in the kkrt ot extension protocol.
     *
     * @param[in] net The network interface (e.g., PETAce-Network interface).
     * @param[in] ext_ot_sizes The size of kkrt 1-out-of-n ot.
     * @throws std::invalid_argument.
     */
    void send(const std::shared_ptr<network::Network>& net, const std::size_t ext_ot_sizes) override;

    /**
     * @brief For the OT at index idx, the sender compute the OT with choice value input.
     *
     * @param[in] idx The OT index that should be encoded. Each OT index allows the sender to learn many messages.
     * @param[in] input The choice value that should be encoded.
     * @param[out] output The OT message encoding the input.
     */
    void encode(const std::size_t idx, const block& input, block& output) override;

private:
    std::vector<block> base_choices_{};

    std::vector<std::shared_ptr<solo::PRNG>> prng_{};

    std::vector<std::vector<block>> q_mat{};
};

/**
 * @brief 1-out-of-n kkrt ot extension [receiver].
 *
 * @see Refer to README.md for more details.
 *
 * @par Example.
 * Refer to example.cpp.
 */
class KkrtNcoOtExtReceiver : public NcoOtExtReceiver {
public:
    explicit KkrtNcoOtExtReceiver(const std::size_t base_ot_sizes) : NcoOtExtReceiver(base_ot_sizes) {
    }

    ~KkrtNcoOtExtReceiver() {
    }

    /**
     * @brief The receiver sets the base ots that are used to extend.
     *
     * @param[in] base_send_ots Base OTs that are used for kkrt ot extension.
     */
    void set_base_ots(const std::vector<std::array<block, 2>>& base_send_ots) override;

    /**
     * @brief The receiver gets chosen messages indexed by choices in the kkrt ot extension protocol.
     *
     * @param[in] net The network interface (e.g., PETAce-Network interface).
     * @param[in] choices The receiver's chosen number, which is stored in 128 bits.
     * @param[out] messages The chosen messages indexed by choices.
     * @throws std::invalid_argument.
     */
    void receive(const std::shared_ptr<network::Network>& net, const std::vector<block>& choices,
            std::vector<block>& messages);

private:
    std::vector<block> base_choices{};

    std::vector<std::array<std::shared_ptr<solo::PRNG>, 2>> prng_{};
};

inline std::unique_ptr<NcoOtExtSender> create_kkrt_ext_sender(const VerseParams& params) {
    return std::make_unique<KkrtNcoOtExtSender>(params.base_ot_sizes);
}

inline std::unique_ptr<NcoOtExtReceiver> create_kkrt_ext_receiver(const VerseParams& params) {
    return std::make_unique<KkrtNcoOtExtReceiver>(params.base_ot_sizes);
}

}  // namespace verse
}  // namespace petace
