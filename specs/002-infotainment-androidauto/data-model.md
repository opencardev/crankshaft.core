# Data Model

This document describes the primary entities, fields, relationships and state transitions for the Infotainment + Android Auto feature.

All names are technology-agnostic; concrete storage formats (SQLite, files) are decided during implementation.

## Entities

1. AndroidDevice
   - id: string (UUID) — unique identifier for the device session
   - model: string — device model name
   - android_version: string
   - connection_type: enum {wired, wireless}
   - paired: boolean
   - last_seen: timestamp
   - capabilities: array[string] (e.g., ["media", "maps", "messaging"])

   Relationships:
   - one-to-many Sessions

   Validation:
   - `id` required
   - `connection_type` required

2. Session
   - id: string (UUID)
   - device_id: string (AndroidDevice.id)
   - state: enum {negotiating, active, suspended, ended, error}
   - started_at: timestamp
   - ended_at: timestamp | null
   - last_heartbeat: timestamp

   State transitions:
   - negotiating -> active (on successful handshake)
   - active -> suspended (on network drop or user switchover)
   - any -> ended (on disconnect or fatal error)

3. Extension
   - id: string (reverse-domain style, e.g., org.open.card.media)
   - name: string
   - version: semver
   - entrypoint: string (path/command)
   - permissions: array[string] (see manifest schema)
   - status: enum {stopped, starting, running, crashed, updating}
   - resources: object {cpu_limit: optional, memory_limit: optional}

   Relationships:
   - may publish Events
   - may register UI Tiles

4. Event
   - id: string (UUID)
   - type: string (topic)
   - source: enum {core, ui, extension}
   - payload: object
   - timestamp: timestamp

5. UserPreference
   - user_id: optional (for multi-user support)
   - key: string
   - value: any (JSON-serialisable)

## Message/Transport model

- WebSocket messages follow the existing implementation in `core/services/websocket`: `subscribe`, `unsubscribe`, `publish`, `event`, `service_command`, `service_response` (see `contracts/ws-schema.json`).
- Required fields: `type` for all messages; `topic` for subscribe/unsubscribe/publish/event; `payload` as object for publish/event; `command` and `params` object for service_command; service commands are allowlisted.
- EventBus (Qt signal-based) is the in-process event dispatcher in core; WebSocketServer bridges publish/subscribe between WS clients and EventBus topics.
- Core provides a REST diagnostics API (OpenAPI spec in `contracts/openapi-diagnostics.yaml`) for lifecycle operations and health checks.

## Validation rules

- All incoming WS messages must include `type` and the fields applicable to that type (`topic`, `payload`, `command`, `params`); unknown message types or commands are rejected.
- `session` objects must transition through valid states; invalid transitions are rejected with an error event.

## Notes on persistence

- Persist AndroidDevice and Session metadata to SQLite (Qt SQL) to support reconnects and diagnostics; include timestamps for last_seen and connection history.
- Preferences persisted using SQLite-backed key-value table; cache frequently used entries in memory for fast reads.

## SQLite Schema Definitions

### AndroidDevice Table
```sql
CREATE TABLE android_devices (
    id TEXT PRIMARY KEY,
    model TEXT NOT NULL,
    android_version TEXT,
    connection_type TEXT CHECK(connection_type IN ('wired', 'wireless')),
    paired INTEGER NOT NULL DEFAULT 0,
    last_seen INTEGER NOT NULL,
    capabilities TEXT  -- JSON array
);
```

### Session Table
```sql
CREATE TABLE sessions (
    id TEXT PRIMARY KEY,
    device_id TEXT NOT NULL,
    state TEXT CHECK(state IN ('negotiating', 'active', 'suspended', 'ended', 'error')),
    started_at INTEGER NOT NULL,
    ended_at INTEGER,
    last_heartbeat INTEGER NOT NULL,
    FOREIGN KEY (device_id) REFERENCES android_devices(id)
);
```

### Preferences Table
```sql
CREATE TABLE preferences (
    user_id TEXT DEFAULT 'default',
    key TEXT NOT NULL,
    value TEXT NOT NULL,  -- JSON-serialized
    PRIMARY KEY (user_id, key)
);
```
