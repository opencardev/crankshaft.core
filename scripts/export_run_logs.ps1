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

[CmdletBinding(PositionalBinding=$false)]
param(
  [Parameter(Mandatory=$true)] [string] $RunId,
  [Parameter()] [string] $Repo = 'opencardev/crankshaft.core',
  [Parameter()] [string] $OutDir = (Join-Path -Path (Get-Location) -ChildPath ("run-" + $RunId + "-logs")),
  [Parameter()] [string] $GhPath = 'C:\\Program Files\\GitHub CLI\\gh.exe',
  [Parameter()] [switch] $DownloadArtifacts,
  [Parameter()] [string] $ArtifactName = 'build-logs-amd64',
  [Parameter()] [string] $JobId
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Test-GhCli {
  param([string] $Path)
  if (-not (Test-Path -LiteralPath $Path)) {
    throw "GitHub CLI not found at '$Path'. Please install gh or update -GhPath.";
  }
}

function New-OutputDirectory {
  param([string] $Path)
  if (-not (Test-Path -LiteralPath $Path)) {
    New-Item -ItemType Directory -Path $Path | Out-Null;
  }
}

function Save-RunLog {
  param([string] $Gh, [string] $Run, [string] $Repository, [string] $Dest)
  Write-Output "Saving run log for $Run to '$Dest'...";
  & $Gh run view $Run --log --repo $Repository > $Dest 2>&1;
}

function Get-RunJobs {
  param([string] $Gh, [string] $Run, [string] $Repository)
  Write-Output "Enumerating jobs for run $Run...";
  $json = & $Gh run view $Run --repo $Repository --json jobs 2>&1;
  try {
    $obj = $json | ConvertFrom-Json;
    if ($null -eq $obj) { return @(); }
    if ($obj.PSObject.Properties.Name -contains 'jobs') {
      $jobsProp = $obj.jobs;
      if ($jobsProp -is [array]) { return $jobsProp; }
      if ($jobsProp -and ($jobsProp.PSObject.Properties.Name -contains 'nodes')) {
        return $jobsProp.nodes;
      }
      return @($jobsProp);
    }
    return @();
  } catch {
    Write-Output "Failed to parse jobs JSON: $($json)";
    return @();
  }
}

function Save-JobLog {
  param([string] $Gh, [string] $Job, [string] $Repository, [string] $Dest)
  Write-Output "Saving job log for $Job to '$Dest'...";
  & $Gh run view --job $Job --log --repo $Repository > $Dest 2>&1;
}

function Download-RunArtifacts {
  param([string] $Gh, [string] $Run, [string] $Repository, [string] $Name, [string] $DestDir)
  Write-Output "Downloading artifact '$Name' for run $Run to '$DestDir'...";
  New-OutputDirectory -Path $DestDir;
  & $Gh run download $Run --repo $Repository -n $Name -D $DestDir 2>&1 | Out-File -FilePath (Join-Path $DestDir "artifact-download-$Name.txt") -Encoding utf8;
}

function Write-LogSummary {
  param([string] $RunLogPath, [string] $JobLogPath, [string] $OutPath, [string] $ArtifactRoot)
  $patterns = @(
    'error:',
    'undefined reference',
    'CPack Error',
    'No .deb packages found',
    'collect2:'
  );

  $summary = New-Object System.Collections.Generic.List[string];
  $summary.Add("Crankshaft GitHub Actions Log Summary");
  $summary.Add("Run log: $RunLogPath");
  if ($JobLogPath) { $summary.Add("Job log: $JobLogPath"); }
  $summary.Add("Generated: " + (Get-Date).ToString('yyyy-MM-dd HH:mm:ss'));
  $summary.Add("");

  if (Test-Path -LiteralPath $RunLogPath) {
    $summary.Add("--- Key lines from run log ---");
    foreach ($pat in $patterns) {
      $matches = Select-String -Path $RunLogPath -Pattern $pat -SimpleMatch -Context 2;
      if ($matches) { $summary.AddRange(($matches | ForEach-Object { $_.ToString() })); }
    }
    $summary.Add("");
  }

  if ($JobLogPath -and (Test-Path -LiteralPath $JobLogPath)) {
    $summary.Add("--- Key lines from job log ---");
    foreach ($pat in $patterns) {
      $matches = Select-String -Path $JobLogPath -Pattern $pat -SimpleMatch -Context 2;
      if ($matches) { $summary.AddRange(($matches | ForEach-Object { $_.ToString() })); }
    }
    $summary.Add("");
  }

  if ($ArtifactRoot -and (Test-Path -LiteralPath $ArtifactRoot)) {
    $preinstall = Get-ChildItem -Recurse -LiteralPath $ArtifactRoot -Filter 'PreinstallOutput.log' -ErrorAction SilentlyContinue | Select-Object -First 1;
    if ($preinstall) {
      $summary.Add("--- Tail of PreinstallOutput.log ---");
      $tail = Get-Content -LiteralPath $preinstall.FullName -Tail 200;
      $summary.AddRange($tail);
    } else {
      $summary.Add("PreinstallOutput.log not found in downloaded artifacts.");
    }
  }

  $summary | Out-File -FilePath $OutPath -Encoding utf8;
  Write-Output "Summary written to '$OutPath'";
}

try {
  Test-GhCli -Path $GhPath;
  New-OutputDirectory -Path $OutDir;

  $runLog = Join-Path $OutDir ("run-" + $RunId + ".log");
  try {
    Save-RunLog -Gh $GhPath -Run $RunId -Repository $Repo -Dest $runLog;
  } catch {
    Write-Warning "Run-level log retrieval failed; falling back to job logs.";
    $jobs = Get-RunJobs -Gh $GhPath -Run $RunId -Repository $Repo;
    foreach ($j in $jobs) {
      $jid = $null;
      if ($j.PSObject.Properties.Name -contains 'databaseId') { $jid = $j.databaseId; }
      elseif ($j.PSObject.Properties.Name -contains 'id') { $jid = $j.id; }
      if ($null -ne $jid) {
        $jl = Join-Path $OutDir ("job-" + $jid + ".log");
        try { Save-JobLog -Gh $GhPath -Job $jid -Repository $Repo -Dest $jl; } catch { Write-Warning "Failed to save job log for $jid"; }
      }
    }
  }

  $jobLog = $null;
  if ($JobId) {
    $jobLog = Join-Path $OutDir ("job-" + $JobId + ".log");
    Save-JobLog -Gh $GhPath -Job $JobId -Repository $Repo -Dest $jobLog;
  }

  $artifactDir = $null;
  if ($DownloadArtifacts.IsPresent) {
    $artifactDir = Join-Path $OutDir $ArtifactName;
    Download-RunArtifacts -Gh $GhPath -Run $RunId -Repository $Repo -Name $ArtifactName -DestDir $artifactDir;
  }

  $summaryPath = Join-Path $OutDir 'summary.txt';
  Write-LogSummary -RunLogPath $runLog -JobLogPath $jobLog -OutPath $summaryPath -ArtifactRoot $artifactDir;

  Write-Output "All logs saved in '$OutDir'";
}
catch {
  Write-Error "Failed to export logs: $($_.Exception.Message)";
  exit 1;
}
