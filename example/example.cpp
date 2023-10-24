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

#include "example.h"

#include <iostream>

// ./build/bin/verse_example  127.0.0.1 8899 127.0.0.1 8890 0
// ./build/bin/verse_example  127.0.0.1 8890 127.0.0.1 8899 1
int main(int argc, char* argv[]) {
    if (argc != 6) {
        throw std::runtime_error("Parameters are incorrect!");
    }
    while (true) {
        std::cout << "+--------------------------------------------------------------------+" << std::endl;
        std::cout << "| The following examples should be executed while reading            |" << std::endl;
        std::cout << "| comments in associated files in examples/.                         |" << std::endl;
        std::cout << "+--------------------------------------------------------------------+" << std::endl;
        std::cout << "| Examples                              | Source Files               |" << std::endl;
        std::cout << "+----------------------------+---------------------------------------+" << std::endl;
        std::cout << "| 1. Naor Pinkas OT                     | np_ot_example.cpp          |" << std::endl;
        std::cout << "| 2. IKNP 1-out-of-2 OT                 | iknp_ot_example.cpp        |" << std::endl;
        std::cout << "| 3. KKRT 1-out-of-n OT                 | kkrt_ot_example.cpp        |" << std::endl;
        std::cout << "+----------------------------+---------------------------------------+" << std::endl;

        int selection = 0;
        std::cin >> selection;
        switch (selection) {
            case 1:
                np_ot_example(atoi(argv[5]), std::string(argv[1]), atoi(argv[2]), std::string(argv[3]), atoi(argv[4]));
                break;
            case 2:
                iknp_ot_example(
                        atoi(argv[5]), std::string(argv[1]), atoi(argv[2]), std::string(argv[3]), atoi(argv[4]));
                break;
            case 3:
                kkrt_ot_example(
                        atoi(argv[5]), std::string(argv[1]), atoi(argv[2]), std::string(argv[3]), atoi(argv[4]));
                break;
            case 0:
                return 0;
        }
    }
    return 0;
}
