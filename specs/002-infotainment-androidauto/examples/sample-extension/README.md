# Sample Extension for Crankshaft

This is a minimal sample extension demonstrating how to create a Crankshaft extension.

## Capabilities

- **UI Tile**: Provides a UI tile in the main dashboard
- **Media Source**: Registers as a media source (e.g., for podcasts, internet radio)
- **Network Access**: Can communicate over the network

## Files

- `manifest.json` - Extension metadata and configuration
- `run-sample-extension.sh` - Entrypoint script that starts the extension

## Installation

To install this extension:

```bash
# Copy the extension directory to the extensions folder
cp -r sample-extension /opt/crankshaft/extensions/

# Or use the REST API
curl -X POST http://127.0.0.1:9002/extensions \
  -H "Content-Type: application/json" \
  -d @manifest.json
```

## Running

The extension will be started automatically by the Crankshaft core once installed. It communicates with the core via WebSocket on port 9001.

### Offline Mode

If the WebSocket connection fails, the extension will run in offline mode for testing/development purposes.

## Permissions Used

- `ui.tile` - Allows displaying a tile in the UI
- `media.source` - Allows acting as a media source
- `network` - Allows network access for streaming content

## Extending the Sample

To create your own extension based on this sample:

1. Update the `manifest.json` with your extension's metadata
2. Modify the Python code in `run-sample-extension.sh` to implement your functionality
3. Add any additional files (icons, configuration) to the extension directory
4. Install and test with Crankshaft

## Troubleshooting

Check the Crankshaft logs for extension errors:

```bash
journalctl -u crankshaft-core -f
```

Or check the extension process directly:

```bash
ps aux | grep sample-extension
```
