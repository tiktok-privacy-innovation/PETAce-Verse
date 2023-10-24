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
#include <vector>

#include "network/network.h"

#include "verse/util/defines.h"

namespace petace {
namespace verse {

/**
 * @brief 1-out-of-n ot extension framework [sender].
 *
 * @see Refer to README.md for more details.
 *
 * @par Example.
 * Refer to example.cpp.
 */
class NcoOtExtSender {
public:
    explicit NcoOtExtSender(const std::size_t base_ot_sizes) : base_ot_sizes_(base_ot_sizes) {
    }

    virtual ~NcoOtExtSender() {
    }

    /**
     * @brief The sender sets the base ots that are used to extend.
     *
     * @param[in] choices The chosen bits in the base ot.
     * @param[in] base_send_ots Base OTs that are used for 1-out-of-n ot extension.
     */
    virtual void set_base_ots(const std::vector<block>& choices, const std::vector<block>& base_recv_ots) = 0;

    /**
     * @brief The sender gets the random keys in the 1-out-of-n ot extension protocol.
     *
     * @param[in] net The network interface (e.g., PETAce-Network interface).
     * @param[in] ext_ot_sizes The size of 1-out-of-n ot.
     * @throws std::invalid_argument.
     */
    virtual void send(const std::shared_ptr<network::Network>& net, const std::size_t ext_ot_sizes) = 0;

    /**
     * @brief For the OT at index idx, the sender compute the OT with choice value input.
     *
     * @param[in] idx The OT index that should be encoded. Each OT index allows the sender to learn many messages.
     * @param[in] input The choice value that should be encoded.
     * @param[out] output The OT message encoding the input.
     */
    virtual void encode(const std::size_t idx, const block& input, block& output) = 0;

protected:
    std::size_t base_ot_sizes_ = 0;

    std::size_t ext_ot_sizes_ = 0;
};

}  // namespace verse
}  // namespace petace
