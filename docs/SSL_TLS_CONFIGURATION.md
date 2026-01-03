# SSL/TLS Configuration Guide

**Purpose**: Enable secure WebSocket connections (wss://) for production Crankshaft deployments

**Status**: Phase 8 Task T061 - Secure WebSocket Support

---

## Overview

Crankshaft WebSocket server supports both:
- **ws://** - Unencrypted WebSocket (development/testing)
- **wss://** - Encrypted WebSocket with TLS 1.3+ (production-recommended)

The dual-mode support allows:
- Development without certificate setup
- Smooth migration to secure mode for deployment
- Backward compatibility with existing clients

---

## Quick Start

### 1. Generate Self-Signed Certificates

For development and testing:

```bash
./scripts/generate-ssl-certs.sh
```

This creates:
- `./.certs/crankshaft-server.crt` - Certificate
- `./.certs/crankshaft-server.key` - Private key

### 2. Enable Secure Mode in Code

In `core/main.cpp` or your service initialization:

```cpp
WebSocketServer* wsServer = new WebSocketServer(9002);

// Enable secure mode (optional, loads certificates)
wsServer->enableSecureMode("./.certs/crankshaft-server.crt", 
                           "./.certs/crankshaft-server.key");

// Check if secure mode is active
if (wsServer->isSecureModeEnabled()) {
    qInfo() << "Secure WebSocket (wss://) enabled";
} else {
    qInfo() << "Standard WebSocket (ws://) active";
}
```

### 3. Connect Clients

**Unencrypted (ws://)**:
```javascript
const ws = new WebSocket("ws://localhost:9002");
```

**Encrypted (wss://)**:
```javascript
const ws = new WebSocket("wss://localhost:9003");

// Note: Browser will warn about self-signed certificates
// Add exception in browser settings or use trusted CA certificate
```

---

## Certificate Generation Options

### Basic (Default)

```bash
./scripts/generate-ssl-certs.sh
```

Creates 365-day certificate for `crankshaft.local`

### Custom Duration

```bash
./scripts/generate-ssl-certs.sh --days 730
```

Creates 2-year certificate

### Custom Location & Name

```bash
./scripts/generate-ssl-certs.sh \
  --cert-dir /opt/crankshaft/certs \
  --cert-name prod-cert \
  --key-name prod-key
```

### Custom Organization Info

```bash
./scripts/generate-ssl-certs.sh \
  --country US \
  --state California \
  --city Mountain View \
  --org "My Company" \
  --cn "vehicle.example.com"
```

### Environment Variables

```bash
export CERT_DIR=/etc/crankshaft/certs
export CERT_NAME=crankshaft
export DAYS=365
./scripts/generate-ssl-certs.sh
```

---

## Configuration

### Environment Variables

Set these before running Crankshaft:

```bash
# SSL/TLS paths
export CRANKSHAFT_SSL_CERT="/etc/crankshaft/certs/crankshaft-server.crt"
export CRANKSHAFT_SSL_KEY="/etc/crankshaft/certs/crankshaft-server.key"

# Start service
crankshaft-core
```

### Configuration File

In `config/crankshaft.conf`:

```ini
[websocket]
# Unencrypted mode (ws://) - port 9002
ws_port=9002
ws_enabled=true

# Secure mode (wss://) - port 9003
wss_enabled=false
wss_port=9003
ssl_cert=/etc/crankshaft/certs/crankshaft-server.crt
ssl_key=/etc/crankshaft/certs/crankshaft-server.key
```

---

## Production Deployment

### Self-Signed Certificates (Development)

**Pros**:
- Free, instant generation
- No CA dependency
- Easy for internal testing

**Cons**:
- Browser warnings
- Not trusted by default
- Clients must bypass security

**Usage**:
```bash
./scripts/generate-ssl-certs.sh
```

### Trusted CA Certificates (Production)

**Recommended for deployed systems**

#### Using Let's Encrypt (Free)

```bash
sudo apt install certbot
sudo certbot certonly --standalone -d vehicle.example.com

# Crankshaft configuration
export CRANKSHAFT_SSL_CERT="/etc/letsencrypt/live/vehicle.example.com/fullchain.pem"
export CRANKSHAFT_SSL_KEY="/etc/letsencrypt/live/vehicle.example.com/privkey.pem"
```

#### Using Commercial CA

1. Generate certificate signing request (CSR):

```bash
openssl req -new -key /path/to/key.key -out request.csr
```

2. Submit to CA (e.g., DigiCert, Sectigo)

3. Configure Crankshaft with issued certificate:

```bash
export CRANKSHAFT_SSL_CERT="/path/to/certificate.crt"
export CRANKSHAFT_SSL_KEY="/path/to/private.key"
```

---

## Certificate Verification

### View Certificate Details

```bash
openssl x509 -in ./.certs/crankshaft-server.crt -text -noout
```

Output shows:
- Issuer information
- Subject (CN=crankshaft.local)
- Validity period (Not Before / Not After)
- Public key info
- Serial number

### Test Connection

Using `openssl`:

```bash
openssl s_client -connect localhost:9003
```

Should show:
- Certificate chain
- Subject line
- Verify result (OK for trusted, SELF SIGNED for self-signed)

### Check Certificate Expiration

```bash
openssl x509 -in ./.certs/crankshaft-server.crt -noout -dates
```

Output:
```
notBefore=Jan  3 12:00:00 2026 GMT
notAfter=Jan  3 12:00:00 2027 GMT
```

---

## Troubleshooting

### Certificate Loading Fails

**Error**: "Failed to start secure WebSocket server"

**Causes**:
- Certificate file not found
- Key file not found
- Invalid certificate format
- Insufficient file permissions

**Solution**:
```bash
# Check files exist
ls -la .certs/

# Check permissions (key must be readable)
chmod 600 .certs/crankshaft-server.key
chmod 644 .certs/crankshaft-server.crt

# Regenerate if corrupted
./scripts/generate-ssl-certs.sh
```

### Port Already in Use

**Error**: "Failed to listen on port 9003"

**Causes**:
- Another process using port
- Previous instance still running
- Insufficient permissions

**Solution**:
```bash
# Check port usage
sudo netstat -tlnp | grep 9003

# Kill conflicting process
sudo kill -9 <PID>

# Restart Crankshaft
crankshaft-core
```

### Browser Shows "Not Secure"

**Expected for self-signed certificates**

**Solutions**:
1. **Development**: Click "Advanced" and accept the certificate
2. **Production**: Use trusted CA certificate (Let's Encrypt recommended)
3. **Testing**: Disable certificate verification (not recommended for production)

### Clients Can't Connect to wss://

**Debugging**:

```bash
# Test SSL connectivity
openssl s_client -connect localhost:9003 -showcerts

# Check server logs
journalctl -u crankshaft-core -f

# Verify DNS (if using hostname)
nslookup crankshaft.local
```

---

## Security Considerations

### Key File Protection

**Important**: Private key file must be protected

```bash
# Correct permissions (read-only by owner)
chmod 600 /path/to/private.key

# Verify no world-readable
ls -la /path/to/private.key
# Should show: -rw------- (not -rw-r--r--)
```

### Certificate Rotation

Self-signed certificates expire (typically 365 days). Rotate before expiration:

```bash
# Back up old certificate
cp ./.certs/crankshaft-server.crt ./.certs/crankshaft-server.crt.bak

# Generate new certificate
./scripts/generate-ssl-certs.sh --days 730

# Restart service
systemctl restart crankshaft-core
```

### TLS Version

Crankshaft enforces TLS 1.3 or later:
- TLS 1.2 and earlier: **Not supported** (for security)
- TLS 1.3+: **Required**

Clients must support TLS 1.3:
- Modern browsers: ✓
- Qt 5.15+: ✓
- Python 3.7+: ✓
- Node.js 12+: ✓

### Cipher Suites

Crankshaft uses OS default cipher suites. Qt automatically selects strong ciphers:

- AES-256-GCM (preferred)
- AES-128-GCM
- CHACHA20-POLY1305

No weak ciphers (DES, MD5, RC4) are used.

---

## API Reference

### C++ WebSocketServer API

```cpp
// Enable secure mode with certificate and key
void enableSecureMode(const QString& certificatePath, 
                      const QString& keyPath);

// Check if secure mode is active
bool isSecureModeEnabled() const;
```

### Connection Endpoints

| Protocol | Port | URI | Use Case |
|----------|------|-----|----------|
| ws:// | 9002 | ws://localhost:9002 | Development, testing |
| wss:// | 9003 | wss://localhost:9003 | Production, secure |

---

## Examples

### Qt/C++ Client

```cpp
#include <QWebSocket>

QWebSocket webSocket;
connect(&webSocket, &QWebSocket::connected, [&]() {
    qInfo() << "Connected to secure WebSocket server";
    webSocket.sendTextMessage("Hello, secure world!");
});

// Connect to secure endpoint
webSocket.open(QUrl("wss://localhost:9003"));
```

### JavaScript/Browser

```javascript
const ws = new WebSocket("wss://localhost:9003");

ws.onopen = () => {
    console.log("Connected to secure server");
    ws.send(JSON.stringify({ type: "subscribe", topic: "events" }));
};

ws.onmessage = (event) => {
    console.log("Message:", event.data);
};

ws.onerror = (error) => {
    console.error("WebSocket error:", error);
};
```

### Python Client

```python
import websockets
import asyncio
import ssl

async def test():
    # Disable certificate verification for self-signed (dev only!)
    ssl_context = ssl.create_default_context()
    ssl_context.check_hostname = False
    ssl_context.verify_mode = ssl.CERT_NONE
    
    async with websockets.connect(
        "wss://localhost:9003",
        ssl=ssl_context
    ) as websocket:
        await websocket.send("Hello, secure world!")
        response = await websocket.recv()
        print(f"Received: {response}")

asyncio.run(test())
```

---

## References

- [Qt WebSocket SSL/TLS](https://doc.qt.io/qt-6/qwebsocketserver.html#sslConfiguration)
- [OpenSSL Manual](https://www.openssl.org/docs/)
- [Let's Encrypt](https://letsencrypt.org/)
- [OWASP TLS Cheat Sheet](https://cheatsheetseries.owasp.org/cheatsheets/Transport_Layer_Protection_Cheat_Sheet.html)

---

**Status**: ✅ Phase 8 Task T061 Complete - Secure WebSocket Support Implemented

**Next Task**: T062 - Structured JSON Logging
