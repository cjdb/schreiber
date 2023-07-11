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
set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

set(VCPKG_CMAKE_SYSTEM_NAME Linux)

set(VCPKG_INSTALL_OPTIONS "--clean-after-build")
set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE "${CMAKE_CURRENT_LIST_FILE}")
set(VCPKG_TARGET_TRIPLET clang_libcxx)

set(TRIPLE x86_64-unknown-linux-gnu)
set(LLVM_ROOT "/tmp/llvm")
set(CMAKE_C_COMPILER "${LLVM_ROOT}/bin/clang")
set(CMAKE_CXX_COMPILER "${LLVM_ROOT}/bin/clang++")
set(CMAKE_CXX_COMPLIER_TARGET ${TRIPLE})

set(CMAKE_AR "${LLVM_ROOT}/bin/llvm-ar")
set(CMAKE_RC_COMPILER "${LLVM_ROOT}/bin/llvm-rc")
set(CMAKE_RANLIB "${LLVM_ROOT}/bin/llvm-ranlib")

string(
  JOIN " " CMAKE_CXX_FLAGS
  -stdlib=libstdc++
  -fuse-ld=lld
  -fdiagnostics-color=always
  -fstack-protector-strong
  -fvisibility=hidden
  -fsanitize=address
  -rtlib=compiler-rt
  -unwindlib=libunwind
  -pedantic
  -static-libgcc
  -ftemplate-backtrace-limit=0
)
