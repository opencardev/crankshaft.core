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

SEARCH_DIRS=(core ui extensions)
TMP_FILE="/tmp/missing_license.txt"

: > "$TMP_FILE"

find "${SEARCH_DIRS[@]}" -type f \( -name '*.cpp' -o -name '*.hpp' -o -name '*.h' -o -name '*.cc' \) -print0 \
  | xargs -0 -I {} sh -c "grep -q 'GNU General Public License' '{}' || echo '{}' >> '$TMP_FILE'"

if [ -s "$TMP_FILE" ]; then
  echo "Files missing license headers:" >&2
  cat "$TMP_FILE" >&2
  exit 1
else
  echo "All checked files contain the required license header."
fi
