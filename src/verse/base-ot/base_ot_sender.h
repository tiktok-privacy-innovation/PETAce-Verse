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
#include "solo/prng.h"

#include "verse/util/defines.h"

namespace petace {
namespace verse {

/**
 * @brief 1-out-of-2 base ot framework [sender].
 *
 * @see Refer to README.md for more details.
 *
 * @par Example.
 * Refer to example.cpp.
 */
class BaseOtSender {
public:
    explicit BaseOtSender(std::size_t base_ot_sizes) : base_ot_sizes_(base_ot_sizes) {
    }

    virtual ~BaseOtSender() {
    }

    /**
     * @brief The sender gets the random messages of 1-out-of-2 oblivious transfer protocol.
     *
     * @param[in] net The network interface (e.g., PETAce-Network interface).
     * @param[out] messages The random output messages of the sender.
     */
    virtual void send(const std::shared_ptr<network::Network>& net, std::vector<std::array<block, 2>>& messages) = 0;

protected:
    std::size_t base_ot_sizes_ = 0;
};

}  // namespace verse
}  // namespace petace
