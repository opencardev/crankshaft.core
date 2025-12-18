# Project: Crankshaft
# This file is part of Crankshaft project.
# Copyright (C) 2025 OpenCarDev Team
#
#  Crankshaft is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 3 of the License, or
#  (at your option) any later version.
#
#  Crankshaft is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with Crankshaft. If not, see <http://www.gnu.org/licenses/>.

#!/usr/bin/env bash
set -euo pipefail

MODE=${1:-check}
SEARCH_DIRS=(core ui extensions)

# Build file list safely
mapfile -t FILES < <(find "${SEARCH_DIRS[@]}" -type f \( -name '*.cpp' -o -name '*.hpp' -o -name '*.h' -o -name '*.cc' \) 2>/dev/null)

if [ ${#FILES[@]} -eq 0 ]; then
  echo "No C/C++ source files found under: ${SEARCH_DIRS[*]}" >&2
  exit 0
fi

if [ "$MODE" = "fix" ]; then
  printf '%s\0' "${FILES[@]}" | xargs -0 clang-format -i
  echo "clang-format: applied formatting to ${#FILES[@]} files"
else
  printf '%s\0' "${FILES[@]}" | xargs -0 clang-format --dry-run --Werror
  echo "clang-format: formatting OK for ${#FILES[@]} files"
fi
