#
#  Copyright Christopher Di Bella
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

string(
  JOIN " " CMAKE_CXX_FLAGS
    -std=c++20
    -fdiagnostics-color=always
    -fstack-protector-strong
    -fvisibility=hidden
    -fsanitize=address,undefined
    -rtlib=compiler-rt
    -unwindlib=libunwind
    -pedantic
    -static-libgcc
    -Wall
    -Wattributes
    -Wcast-align
    -Wconversion
    -Wdouble-promotion
    -Wextra
    -Wformat=2
    -Wnon-virtual-dtor
    -Wnull-dereference
    -Wodr
    -Wold-style-cast
    -Woverloaded-virtual
    -Wshadow
    -Wsign-conversion
    -Wsign-promo
    -Wunused
    -Wno-bit-int-extension
    -Wno-ignored-attributes
    -Wno-cxx-attribute-extension
    -Wno-gnu-include-next
    -Wno-private-header
    -Wno-unused-command-line-argument
    -Werror
    -ftemplate-backtrace-limit=0
    -fno-rtti
    -fuse-ld=lld
)
