# Android Auto Phone Detection Troubleshooting

Use this checklist first, then follow the detailed steps. Commands assume a shell on the Pi (use `wsl bash -lc "<cmd>"` from Windows if needed).

## Printable checklist
- [ ] Use a short, data-rated USB 3.0 cable; plug directly into the Pi (no unpowered hub).
- [ ] On the phone: enable Android Auto, allow USB data, and choose "Use USB for: Android Auto/File Transfer" when prompted.
- [ ] While plugging in, watch `dmesg -w` for new USB lines.
- [ ] Capture `lsusb` before and after plugging in; note VID:PID and speed from `lsusb -t`.
- [ ] Check `/dev/bus/usb/...` permissions; apply a udev rule if `crankshaft` lacks access.
- [ ] Stop `adb` if running (`adb kill-server`) so Android Auto can claim the device.
- [ ] Restart services: `sudo systemctl restart crankshaft-core crankshaft-ui` after any changes.
- [ ] Enable debug logging (AASDK + Qt) and collect `journalctl -u crankshaft-core -b` around the plug event.
- [ ] Verify power: stable 5V/3A+ supply or powered hub if the phone flaps.

## Detailed steps

### 1) Hardware and cable
- Prefer a short (<1 m) shielded, data-rated cable; avoid charge-only leads.
- Use a direct Pi host port; if you must use a hub, ensure it is powered.

### 2) Kernel sees the phone
```bash
dmesg -w
# plug the phone; expect new USB device lines
```
If nothing appears, focus on cable/port/power.

### 3) Enumerate USB and speed
```bash
lsusb
lsusb -t  # check negotiated speed; avoid full/low speed if possible
```
Note the VID:PID for permissions and logging.

### 4) Permissions and udev
If `crankshaft` cannot open the device node under `/dev/bus/usb`, add a udev rule (replace VID/PID):
```bash
echo 'SUBSYSTEM=="usb", ATTR{idVendor}=="18d1", ATTR{idProduct}=="4ee7", MODE="0664", GROUP="crankshaft"' | sudo tee /etc/udev/rules.d/99-androidauto.rules
sudo udevadm control --reload
sudo udevadm trigger
```
Replug the phone afterwards.

### 5) Phone-side settings
- In Android Auto settings, enable "Add new cars" and allow running while locked.
- When prompted after plugging in, choose Android Auto / File Transfer instead of charging only.
- If previously denied trust, reset by toggling Developer Settings for Android Auto and re-authorise.

### 6) Service health and restart
```bash
sudo systemctl daemon-reload
sudo systemctl restart crankshaft-core crankshaft-ui
sudo systemctl status crankshaft-core crankshaft-ui
```
Ensure Core is up before UI (UI unit requires Core).

### 7) Logs and debug output
- Core and Android Auto (AASDK) logs:
  ```bash
  journalctl -u crankshaft-core -b -e | grep -i "android\|aasdk\|usb"
  ```
- Enable verbose logging temporarily:
  ```bash
  export AASDK_LOG_LEVEL=DEBUG
  QT_LOGGING_RULES="*=true" QT_DEBUG_PLUGINS=1 crankshaft-ui
  ```
  For systemd override:
  ```bash
  sudo systemctl edit crankshaft-core
  # [Service]
  # Environment="AASDK_LOG_LEVEL=DEBUG"

  sudo systemctl edit crankshaft-ui
  # [Service]
  # Environment="QT_LOGGING_RULES=*=true" "QT_DEBUG_PLUGINS=1"

  sudo systemctl restart crankshaft-core crankshaft-ui
  ```

### 8) Conflicts with adb or other claimants
If `adb devices` shows the phone, stop adb so Android Auto can claim the accessory interface:
```bash
adb kill-server
```

### 9) Power stability
- Use a reliable 5V/3A (or better) supply.
- If the phone connects and drops repeatedly, test with a powered USB hub to offload current draw.

## Next steps / what to capture for support
- `lsusb` and `lsusb -t` output before/after plugging in.
- `dmesg -w` snippet during plug event.
- `journalctl -u crankshaft-core -b` around the failure (with DEBUG enabled if possible).
- Phone model, Android version, cable used, and power arrangement.

With these details, open an issue and include the logs and steps tried.

## Analysis of supplied `usb_events.log`

Your log excerpt shows repeated detection entries for the same phone model (Pixel_8_Pro) followed by a warning and a disconnect with an empty model field, then the device reappears on a different USB path:

- Multiple detections at `Path: /dev/bus/usb/004/002` (12:16:58, 12:16:59, 21:50:53, 21:50:54)
- `Device remove event without USB path` and a disconnect for `Path: /dev/bus/usb/004/002` at 21:57:45
- Immediately afterwards the device is detected on `Path: /dev/bus/usb/004/003` (21:57:49)

What this suggests:
- The kernel or USB hub is causing the phone to re-enumerate — the device node changed from `/004/002` to `/004/003`.
- The disconnect event lacked a model string, indicating the hotplug handler lost some identifying metadata during the removal (race, partial disconnect, or transient USB reset).
- Re-enumeration like this commonly points to: unreliable cable/port, power drop/reset, hub behaviour, kernel USB reset due to interface claim changes (ADB vs MTP vs Accessory), or autosuspend/driver issues.

**CRITICAL: Mock service in use**  
The Core application reads the ProfileManager's active host profile at startup to determine which services to start. The AndroidAuto device in profiles has a `useMock` flag that controls whether `MockAndroidAutoService` or `RealAndroidAutoService` (AASDK) is instantiated. By default, the AndroidAuto device has `useMock: true`, which is why you see device detection logs (from udev/hotplug scripts) but no deeper AASDK protocol negotiation. The mock service simulates connection state changes but does not interact with the real phone over USB.

To enable the real AASDK-based service:
1. Check which service is running at startup:
   ```bash
   journalctl -u crankshaft-core -b | grep "AndroidAuto service"
   # Look for: "AndroidAuto service started successfully (Mock mode)" or "(Real mode)"
   ```
2. Update the active host profile's AndroidAuto device configuration to set `useMock: false`:
   - Via ProfileManager API (when exposed): `profileManager->setDeviceUseMock(profileId, "AndroidAuto", false)`
   - Or edit the profile JSON directly (if persisted to disk in `/etc/crankshaft/profiles/`)
   - Or modify the default profile code in `ProfileManager::initializeDefaultProfiles()` and rebuild
3. Rebuild if needed: `cmake --build build -j`.
4. Restart `crankshaft-core`: `sudo systemctl restart crankshaft-core`.
5. Verify the service mode in logs: `journalctl -u crankshaft-core -b | grep "Starting AndroidAuto"`
6. Enable AASDK debug logging (see section 7) and capture full `journalctl` output during plug/unplug.

Checklist of focused diagnostics (run on Pi while reproducing):
- [ ] Capture kernel messages during the event: `sudo journalctl -k -f` or `dmesg -w` while plugging/unplugging.
- [ ] Capture systemd journal for crankshaft around timestamps: `sudo journalctl -u crankshaft-core -u crankshaft-ui --since "2025-12-17 21:57:40" --until "2025-12-17 21:58:00" -o short-iso` (adjust times).
- [ ] Run `udevadm monitor --udev --property` before plugging to see the full env for add/remove events.
- [ ] Run `lsusb -v -d <VID:PID>` (replace with phone VID:PID) to inspect interfaces and whether it exposes multiple interfaces that might be claimed/unclaimed.
- [ ] Check power/voltage fluctuation logs (if available) and test with a known-good cable and powered hub.
- [ ] Confirm adb is not intermittently claiming interfaces: `ps aux | grep adb` and `adb devices`.

Suggested mitigations to try next
- Use a different high-quality cable and plug into another Pi USB port; avoid hubs during testing.
- Disable USB autosuspend to rule out power-save resets:
  ```bash
  echo -1 | sudo tee /sys/module/usbcore/parameters/autosuspend
  ```
- Add a udev rule to capture and set stable permissions but avoid relying on `/dev/bus/usb/<bus>/<dev>` paths; prefer matching by `ATTRS{serial}` or `ENV{ID_SERIAL}` where possible.
- If ADB is present on the Pi for other tooling, stop it during tests: `adb kill-server`.
- Enable AASDK debug logging and capture a full session `journalctl -u crankshaft-core -f` while plugging the phone.

If you want, I can:
- add the recommended diagnostic commands into a small script in `scripts/` to run while reproducing, or
- open a small issue template pre-filled with the logs/commands to collect from the Pi.

