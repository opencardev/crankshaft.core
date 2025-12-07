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

param(
    [string]$BuildType = "Debug"
)

$ErrorActionPreference = "Stop"

$BUILD_DIR = "build"

Write-Host "Building Crankshaft MVP in $BuildType mode via WSL..." -ForegroundColor Cyan

# Run build in WSL
wsl bash -lc "cd $(wsl wslpath -u '$PWD') && ./scripts/build.sh $BuildType"

Write-Host "Build complete!" -ForegroundColor Green
