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

#include <array>
#include <memory>
#include <vector>

#include "network/network.h"
#include "solo/hash.h"
#include "solo/prng.h"

#include "verse/two-choose-one/ot_ext_receiver.h"
#include "verse/two-choose-one/ot_ext_sender.h"
#include "verse/util/defines.h"

namespace petace {
namespace verse {

/**
 * @brief 1-out-of-2 iknp ot extension [sender].
 *
 * @see Refer to README.md for more details.
 *
 * @par Example.
 * Refer to example.cpp.
 */
class IknpOtExtSender : public OtExtSender {
public:
    IknpOtExtSender(std::size_t base_ot_sizes, std::size_t ext_ot_sizes) : OtExtSender(base_ot_sizes, ext_ot_sizes) {
    }

    ~IknpOtExtSender() {
    }

    /**
     * @brief The sender sets the base ots that are used to extend.
     *
     * @param[in] base_send_ots Base OTs that are used for iknp ot extension.
     */
    void set_base_ots(const std::vector<block>& choices, const std::vector<block>& base_recv_ots) override;

    /**
     * @brief The sender gets the random messages in the iknp ot extension protocol.
     *
     * @param[in] net The network interface (e.g., PETAce-Network interface).
     * @param[out] messages The random output messages of the sender.
     * @throws std::invalid_argument.
     */
    void send(const std::shared_ptr<network::Network>& net, std::vector<std::array<block, 2>>& messages) override;

private:
    std::vector<block> base_choices_{};

    std::vector<std::shared_ptr<solo::PRNG>> prng_{};
};

/**
 * @brief 1-out-of-2 iknp ot extension [receiver].
 *
 * @see Refer to README.md for more details.
 *
 * @par Example.
 * Refer to example.cpp.
 */
class IknpOtExtReceiver : public OtExtReceiver {
public:
    IknpOtExtReceiver(std::size_t base_ot_sizes, std::size_t ext_ot_sizes)
            : OtExtReceiver(base_ot_sizes, ext_ot_sizes) {
    }

    ~IknpOtExtReceiver() {
    }

    /**
     * @brief The receiver sets the base ots that are used to extend.
     *
     * @param[in] base_send_ots Base OTs that are used for iknp ot extension.
     */
    void set_base_ots(const std::vector<std::array<block, 2>>& base_send_ots) override;

    /**
     * @brief The receiver gets chosen messages indexed by choices in the iknp ot extension protocol.
     *
     * @param[in] net The network interface (e.g., PETAce-Network interface).
     * @param[in] choices The chosen bits of receiver.
     * @param[out] messages The chosen messages indexed by choices.
     * @throws std::invalid_argument.
     */
    void receive(const std::shared_ptr<network::Network>& net, const std::vector<block>& choices,
            std::vector<block>& messages) override;

private:
    std::vector<block> base_choices{};

    std::vector<std::array<std::shared_ptr<solo::PRNG>, 2>> prng_{};
};

inline std::unique_ptr<OtExtSender> create_iknp_ext_sender(const VerseParams& params) {
    return std::make_unique<IknpOtExtSender>(params.base_ot_sizes, params.ext_ot_sizes);
}

inline std::unique_ptr<OtExtReceiver> create_iknp_ext_receiver(const VerseParams& params) {
    return std::make_unique<IknpOtExtReceiver>(params.base_ot_sizes, params.ext_ot_sizes);
}

}  // namespace verse
}  // namespace petace
