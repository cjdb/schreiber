# Copyright (c) LLVM Foundation
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
import lit.formats
import lit.util
import os
import platform
import re
import subprocess
import sys
import tempfile

from lit.llvm import llvm_config
from lit.llvm.subst import ToolSubst, FindTool

config.name = 'schreiber'
config.test_format = lit.formats.ShTest(not llvm_config.use_lit_shell)
config.suffixes = ['.cc']
config.excludes = []
config.test_source_root = os.path.dirname(__file__)
config.test_exec_root = os.path.join(config.clang_obj_root, 'test')

llvm_config.use_default_substitutions()
llvm_config.use_clang()

config.substitutions.append(
    ('%src_include_dir', config.clang_src_dir + '/include'))
config.substitutions.append(('%target_triple', config.target_triple))

llvm_config.with_system_environment(
    ['ASAN_SYMBOLIZER_PATH', 'MSAN_SYMBOLIZER_PATH'])

config.substitutions.append(('%PATH%', config.environment['PATH']))

tool_dirs = [config.clang_tools_dir, config.llvm_tools_dir]
tools = []

if platform.system() != 'Linux':
    print(f'Unsupported system: {platform.system()}', file=sys.stderr)
    sys.exit(1)

if not config.target_triple.startswith('x86_64'):
    print(f'Unsupported target triple: {config.target_triple}')
    sys.exit(1)
