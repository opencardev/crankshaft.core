# Contract Versioning and Migration Policy

**Purpose**: Define API and contract evolution strategy for long-term maintainability

**Status**: Phase 8 Task T063 - Contract Versioning Policy Documentation

---

## Overview

This document defines how Crankshaft APIs and contracts evolve while maintaining stability and backward compatibility for clients.

**Key Principles**:
- Semantic versioning for all public APIs
- Explicit deprecation period before breaking changes
- Clear migration paths for clients
- Comprehensive changelog documentation

---

## Semantic Versioning

Crankshaft uses semantic versioning: `MAJOR.MINOR.PATCH`

### Version Format

- **MAJOR**: Incompatible API changes (rare, requires major version bump)
- **MINOR**: New functionality, backward-compatible additions
- **PATCH**: Bug fixes, security patches

### Examples

| Version | Change | Impact |
|---------|--------|--------|
| 1.0.0 → 1.1.0 | Add new WebSocket event type | Minor (backward-compatible) |
| 1.1.0 → 1.2.0 | New extension permission type | Minor (backward-compatible) |
| 1.2.0 → 1.2.1 | Fix JSON parsing bug | Patch (no API change) |
| 1.2.1 → 2.0.0 | Remove deprecated WebSocket endpoint | Major (breaking change) |

---

## Backward Compatibility Guarantees

### Within Minor Versions (1.0 → 1.5)

✅ **Guaranteed backward compatible**:
- New event types can be added
- New optional fields in JSON responses
- New endpoint paths can be added
- New permission types can be added
- Existing API behavior unchanged

❌ **NOT allowed**:
- Removing fields from JSON responses
- Removing event types
- Changing event structure
- Removing endpoints
- Changing method signatures

### Within Patch Versions (1.0.0 → 1.0.5)

✅ **Guaranteed**:
- Bug fixes
- Security patches
- Performance improvements
- No API changes
- Fully backward compatible

### Breaking Changes (Major Versions)

Breaking changes only occur with major version bumps (1.x → 2.x).

Before major bump:
1. Announce deprecation 2 versions in advance
2. Provide 6+ months notice
3. Document migration guide
4. Release deprecated version (e.g., 1.8, 1.9)

Example timeline:
```
1.7.0 (Dec 2025): Feature X released normally
1.8.0 (Mar 2026): Feature X deprecated (6-month grace period announced)
1.9.0 (Sep 2026): Final 1.x release, deprecation warnings emphasized
2.0.0 (Jan 2027): Feature X removed entirely
```

---

## Contract Evolution Patterns

### Pattern 1: Add New Optional Field

**Scenario**: WebSocket response needs additional data

**Before (v1.0)**:
```json
{
  "type": "android-auto/status",
  "state": "connected"
}
```

**After (v1.1)**:
```json
{
  "type": "android-auto/status",
  "state": "connected",
  "device_model": "Pixel 6"
}
```

**Migration**: Clients ignore unknown fields (no code change needed)

### Pattern 2: Add New Optional Parameter

**Scenario**: REST endpoint gains optional filter

**Before (v1.0)**:
```bash
GET /extensions
```

**After (v1.1)**:
```bash
GET /extensions?permission=ui.tile&state=running
```

**Migration**: Existing clients work unchanged, new clients can use optional params

### Pattern 3: Add New Event Type

**Scenario**: New service needs to emit events

**Before (v1.0)**:
Events: `android-auto/*`, `media/*`, `settings/*`

**After (v1.1)**:
Events: `android-auto/*`, `media/*`, `settings/*`, `navigation/*`

**Migration**: Clients subscribed to `#` wildcard get new events automatically

### Pattern 4: Deprecate Endpoint

**Timeline (6-month grace period)**:

**v1.7 (Initial)**:
```
GET /extensions → Works normally
```

**v1.8 (Deprecation announced)**:
```
GET /extensions → Works, includes X-Deprecated-Since: 1.8 header
Response includes: "deprecated": {"since": "1.8", "until": "2.0", "migration": "..."}
```

**v1.9 (Final warning)**:
```
GET /extensions → Works, headers emphasize final version
Logs: Warning "This endpoint is deprecated, will be removed in v2.0"
```

**v2.0 (Removal)**:
```
GET /extensions → Returns 404
Alternative: GET /extensions/list (new endpoint since 1.8)
```

### Pattern 5: Rename Field (Backward Compatible)

**Scenario**: Field name is misleading, needs rename

**v1.7**:
```json
{"state": "running"}
```

**v1.8** (Keep both names):
```json
{"state": "running", "status": "running"}  // Added alias
```

**v1.9** (Mark old name deprecated):
```json
{
  "state": "running",  // Deprecated, use "status"
  "status": "running"  // Preferred
}
```

**v2.0** (Remove old name):
```json
{"status": "running"}
```

---

## Contract Versioning in Crankshaft

### WebSocket API (contracts/ws-schema.json)

**Current Version**: 1.0

```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "title": "Crankshaft WebSocket Protocol",
  "version": "1.0",
  "properties": {
    "api_version": {"type": "string"},
    "protocol_version": {"type": "string"}
  }
}
```

**Versioning Rules**:
- `version` field indicates contract version
- New event types: MINOR version bump
- New command parameters: MINOR version bump
- Event removal: MAJOR version bump

### REST API (contracts/openapi-diagnostics.yaml)

**Current Version**: 1.0

```yaml
openapi: 3.0.0
info:
  title: Crankshaft Diagnostics API
  version: 1.0.0
```

**Versioning Rules**:
- `version` field in API spec
- New endpoints: MINOR version bump
- New response fields: MINOR version bump
- Removed endpoints: MAJOR version bump (with deprecation period)

### Extension Manifest (contracts/extension-manifest.schema.json)

**Current Version**: 1.0

```json
{
  "type": "object",
  "properties": {
    "manifest_version": {"type": "integer", "const": 1}
  }
}
```

**Versioning Rules**:
- `manifest_version` indicates schema version
- New optional fields: Same version
- New required fields: NEW version (extensions must update)
- Removed fields: NEW version (breaking)

---

## Deprecation Process

### Step 1: Announce (Release N)

**Documentation**:
- Add to CHANGELOG.md: "DEPRECATED: Feature X (will be removed in vN+2)"
- Update API documentation with deprecation notice
- Email API subscribers if mailing list exists

**Code**:
```cpp
if (featureX_used) {
  Logger::instance().warning("Feature X is deprecated since v1.8, "
                              "will be removed in v2.0. "
                              "Use Feature Y instead.");
}
```

**Response Headers** (REST APIs):
```http
X-Deprecated-Since: 1.8
X-Will-Be-Removed-In: 2.0
X-Alternative: /new-endpoint-path
```

### Step 2: Support (Releases N+1)

Continue to support deprecated feature with warnings.

**Documentation**:
- Migration guide in docs/MIGRATION_GUIDES/
- Examples of old vs. new approach
- Performance/security benefits of new approach

### Step 3: Final Warning (Release N+2)

**Last version before removal**

**Code**:
```cpp
if (featureX_used) {
  Logger::instance().error("Feature X is FINAL RELEASE in v1.9, "
                            "will be removed in v2.0. "
                            "Immediate migration required!");
}
```

**Documentation**:
- Prominent warning in API docs
- Blog post or announcement if major feature
- Support team prepared for migration questions

### Step 4: Remove (Release N+3, Major Version)

**Breaking change release**

**Code**: Feature completely removed

**Changelog**:
```
BREAKING: Removed Feature X (deprecated since v1.8)
- Use Feature Y instead (available since v1.8)
- See MIGRATION_GUIDES/feature-x-to-y.md
```

---

## Migration Guides

### Structure

Store in `docs/MIGRATION_GUIDES/`

**Naming**: `{old-feature}-to-{new-feature}.md`

**Sections**:
1. Overview (what changed, why)
2. Impact (who is affected)
3. Old approach (how it was done)
4. New approach (how to do it now)
5. Comparison (side-by-side code example)
6. Troubleshooting (common issues)
7. Timeline (deprecation → removal dates)

### Example: WebSocket Endpoint Rename

**File**: `docs/MIGRATION_GUIDES/websocket-subscribe-to-subscribe-v2.md`

```markdown
# Migrating from /subscribe to /subscribe/v2

## Overview
WebSocket subscription endpoint changed from `/subscribe` to `/subscribe/v2` 
with improved filtering and fewer round-trips.

## Who is Affected
- Clients using WebSocket `/subscribe` command
- All client types: Qt, JavaScript, Python, etc.

## Old Approach
\`\`\`json
{"command": "/subscribe", "topic": "media/status"}
\`\`\`

## New Approach
\`\`\`json
{"command": "/subscribe/v2", "topic": "media/*", "filter": {"active": true}}
\`\`\`

## Timeline
- v1.8: /subscribe/v2 introduced
- v1.9: /subscribe marked deprecated
- v2.0: /subscribe removed

## Impact Analysis
- Smaller message size for complex filters
- Single subscription for multiple topics
- More flexible filtering
```

---

## Changelog Format

### Standard Sections

```markdown
## [2.0.0] - 2027-01-15

### Added
- New `/extensions/v2` REST endpoint with improved filtering
- Support for extension dependencies in manifest

### Changed
- WebSocket authentication now uses JWT tokens (breaking change)
- Settings API response includes metadata

### Deprecated
- WebSocket `/auth` command (use JWT header instead)
- Settings API v1 endpoints

### Removed
- WebSocket `/subscribe` endpoint (use `/subscribe/v2` instead)
- Legacy theme format (use CSS variables)

### Fixed
- Memory leak in extension cleanup
- Incorrect timestamp in log files

### Security
- Enforce TLS 1.3 for wss connections
- Validate manifest signatures for extensions
```

---

## Compatibility Testing

### Version Matrix

Test each new version against:

| Client Version | v1.8 | v1.9 | v2.0 |
|---|---|---|---|
| v1.5 | ✅ Works | ✅ Works (warnings) | ❌ Incompatible |
| v1.8 | ✅ Works | ✅ Works | ⚠️ Partial (needs migration) |
| v1.9 | ✅ Works | ✅ Works | ⚠️ Needs migration |
| v2.0 | ❌ | ⚠️ | ✅ Works |

### Testing Checklist

- [ ] New minor version: Clients from N-2 versions still work
- [ ] New major version: Clients must have explicit upgrade plan
- [ ] Deprecated features: Work with warnings
- [ ] Removed features: Proper error messages
- [ ] Migration guides: Complete and tested

---

## Communication Plan

### For Minor Versions

1. **Release notes** on GitHub
2. **Changelog** entry
3. **Commit message** references issue if available

### For Deprecated Features

1. **Announcement** 6 months before removal
2. **Documentation** updates with migration guides
3. **Email notification** to API subscribers (if list maintained)
4. **Code warnings** when deprecated feature used
5. **Blog post** for major deprecations

### For Major Versions

1. **Roadmap** published 6+ months in advance
2. **Migration guide** for each breaking change
3. **Webinar or meeting** for partners/integrators
4. **Detailed changelog** with impact analysis
5. **Support resources** for questions

---

## Extension Manifest Versioning

### Current Manifest Version: 1

```json
{
  "manifest_version": 1,
  "id": "com.example.ext",
  "name": "Example Extension",
  "version": "1.0.0"
}
```

### Proposed Manifest Version: 2

**New Features**:
- Dependency declaration
- Permission groups
- Conditional permissions

```json
{
  "manifest_version": 2,
  "id": "com.example.ext",
  "name": "Example Extension",
  "version": "1.0.0",
  "dependencies": {
    "crankshaft": ">=1.8.0",
    "media-service": ">=1.0.0"
  },
  "permission_groups": {
    "storage": ["storage.read", "storage.write"]
  }
}
```

### Migration Path

| Manifest Version | Support | Crankshaft Versions |
|---|---|---|
| 1 | Supported | 1.0 → 1.9 |
| 2 | Supported | 1.8+ |
| 1 (deprecated) | Supported with warnings | 2.0+ |
| 1 | Removed | 3.0+ |

---

## API Stability Levels

### Level 1: Stable (Default)

- Public, documented API
- Backward compatible guarantees
- 6-month deprecation period

**Example**: `/extensions` REST endpoint

### Level 2: Preview (Experimental)

- New, subject to change
- No backward compatibility guarantee
- May be renamed, removed, or restructured
- API documentation clearly marked "PREVIEW"

**Example**: `/extensions/v2` (new in v1.8)

### Level 3: Internal (Not for Public Use)

- Internal APIs between components
- No stability guarantee
- May change in any release

**Example**: `Core::internal::parseManifest()`

---

## Versioning Decision Matrix

| Situation | Version Bump | Example |
|-----------|--------------|---------|
| New optional endpoint param | MINOR | Add `?recursive=true` to GET |
| New event type | MINOR | Add `navigation/status` events |
| New response field | MINOR | Add `timestamp` to existing response |
| Remove endpoint | MAJOR | Delete `/old-endpoint` |
| Change event structure | MAJOR | Rename `state` to `status` |
| Change required param | MAJOR | Make optional `token` param required |
| Bug fix | PATCH | Fix JSON parsing error |
| Performance improvement | PATCH | Optimize query speed |
| Add deprecation warning | MINOR | Mark feature for removal |

---

## Best Practices

1. **Plan ahead**: Think about evolution before releasing API
2. **Deprecate early**: Mark deprecated before removing
3. **Document thoroughly**: Migration guides, examples, rationale
4. **Test thoroughly**: Ensure backward compatibility
5. **Communicate clearly**: Announce changes in advance
6. **Support transitions**: Help clients migrate
7. **Provide alternatives**: Always offer new way before removing old
8. **Monitor adoption**: Track deprecated feature usage before removal

---

## Questions & Answers

### Q: Can we add fields to JSON responses in PATCH releases?

**A**: Yes, if optional. Clients must ignore unknown fields. PATCH releases never change existing fields.

### Q: When can we remove an endpoint?

**A**: Only in MAJOR version with 6+ month deprecation period. Mark deprecated in MINOR version, remove in MAJOR.

### Q: How long do we support old manifest versions?

**A**: At least 2 major versions. Manifest v1 supported until v3 at minimum.

### Q: What if a bug requires breaking change immediately?

**A**: Still follow deprecation process. If critical security issue, can be exception with explanation.

### Q: Can extensions require specific Crankshaft versions?

**A**: Yes, via `dependencies.crankshaft` in manifest v2+. Manifest v1 doesn't support this.

---

## References

- [Semantic Versioning](https://semver.org/)
- [API Design Best Practices](https://swagger.io/resources/articles/best-practices-in-api-design/)
- [Deprecation Policy Examples](https://developers.google.com/maps/deprecations)

---

**Status**: ✅ Phase 8 Task T063 Complete - Contract Versioning Policy Documented

**Next Task**: T064 - Design for Driving Compliance
