# Workflows Overview

This document describes the split CI/CD/Release workflows and how they interact.

## CI (`.github/workflows/ci.yml`)
- **Triggers**: push and pull_request on all branches (ignores docs/md/gitignore).
- **Jobs**:
  - `code-quality`: runs `scripts/lint.sh` if present.
  - `build-packages`: calls reusable `build.yml`; architecture matrix resolves to `amd64/arm64/armhf` for `main`/`develop` (and PRs targeting them), otherwise `amd64` only.
- **Outputs**: Build artifacts (.deb packages for core, ui, src, logs) and can be consumed by downstream workflows.

## Reusable Build (`.github/workflows/build.yml`)
- **Triggers**: `workflow_call` (preferred) and manual `workflow_dispatch`.
- **Inputs**: `version` (optional), `architectures` (space-separated, default `amd64 arm64 armhf`).
- **Jobs**:
  - `prepare`: compute version and matrix from inputs.
  - `build`: docker buildx per arch; uploads debs and logs.
  - `sbom`: generates CycloneDX SBOM and uploads.

## Changelog (`.github/workflows/changelog.yml`)
- **Triggers**: `workflow_call`; `pull_request` closed (merged) into `main`.
- **Outputs**: `changelog-latest` artifact, `changelog-version` artifact, and `version` output (via workflow_call).

## CD (`.github/workflows/cd.yml`)
- **Triggers**: `workflow_run` of CI on `main` (completed/success).
- **Jobs**:
  - `publish-apt`: calls `trigger-apt-publish.yml` with CI run ID.
  - `build-pi-images`: calls `build-pi-gen-lite.yml` (armhf/arm64) using the published packages.

## Trigger APT Publish (`.github/workflows/trigger-apt-publish.yml`)
- **Triggers**: `workflow_call` and `workflow_dispatch`.
- **Purpose**: dispatches to `opencardev/packages` repo to import DEBs into APT; inputs `build_run_id`, `distribution`.

## Pi-gen Images (`.github/workflows/build-pi-gen-lite.yml`)
- **Triggers**: `workflow_call` and `workflow_dispatch`; also push/PR touching `image_builder/**` for manual use.
- **Inputs**: `version`, `release` (default trixie), `auto_release`, `create_release_draft`.
- **Jobs**: build Raspberry Pi images for `armhf` and `arm64`, upload artifacts/logs.

## Release (`.github/workflows/release-from-pi-gen.yml`)
- **Triggers**: `workflow_run` of CD on `main` (success); also `workflow_call` and `workflow_dispatch` for manual fallback.
- **Jobs**:
  - Downloads pi-gen artifacts by run ID, computes checksums, generates SBOMs from images, and creates GitHub release (respects `create_draft`).

## Flow Summary
1. **CI** runs on every push/PR, performs lint, builds debs (arch matrix based on branch), and emits artifacts/SBOM.
2. On `main`, CI success triggers **CD**, which publishes debs to APT and builds Pi images from the published packages.
3. CD success on `main` triggers **Release**, which packages and publishes Pi images with checksums and SBOMs.
4. **Changelog** runs when PRs to `main` are merged (and is callable) to supply version/changelog artifacts.
