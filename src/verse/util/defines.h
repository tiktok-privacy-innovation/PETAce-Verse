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

#include <cstddef>
#include <memory>

#include "network/network.h"
#include "solo/ec_openssl.h"

namespace petace {
namespace verse {

// some declarations
using block = __m128i;
using EC = solo::ECOpenSSL;
const std::size_t kEccPointLen = 33;
const std::size_t kCurveID = 415;
const std::size_t kHashDigestLen = 32;

struct VerseParams {
    std::size_t base_ot_sizes;
    std::size_t ext_ot_sizes;
    std::shared_ptr<network::Network> net;
};

}  // namespace verse
}  // namespace petace
