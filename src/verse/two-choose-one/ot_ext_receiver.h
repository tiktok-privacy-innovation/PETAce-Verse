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

#include "verse/util/defines.h"

namespace petace {
namespace verse {

/**
 * @brief 1-out-of-2 ot extension framework [receiver].
 *
 * @see Refer to README.md for more details.
 *
 * @par Example.
 * Refer to example.cpp.
 */
class OtExtReceiver {
public:
    OtExtReceiver(std::size_t base_ot_sizes, std::size_t ext_ot_sizes)
            : base_ot_sizes_(base_ot_sizes), ext_ot_sizes_(ext_ot_sizes) {
    }

    virtual ~OtExtReceiver() {
    }

    /**
     * @brief The receiver sets the base ots that are used to extend.
     *
     * @param[in] base_send_ots Base OTs that are used for ot extension.
     */
    virtual void set_base_ots(const std::vector<std::array<block, 2>>& base_send_ots) = 0;

    /**
     * @brief The receiver gets chosen messages indexed by choices in the 1-out-of-2 oblivious transfer extension
     * protocol.
     *
     * @param[in] net The network interface (e.g., PETAce-Network interface).
     * @param[in] choices The chosen bits of receiver.
     * @param[out] messages The chosen messages indexed by choices.
     */
    virtual void receive(const std::shared_ptr<network::Network>& net, const std::vector<block>& choices,
            std::vector<block>& messages) = 0;

    std::size_t base_ot_sizes_ = 0;

    std::size_t ext_ot_sizes_ = 0;
};

}  // namespace verse
}  // namespace petace
