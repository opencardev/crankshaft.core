#!/usr/bin/env python3
"""
Quick reference for wireless Android Auto configuration in Crankshaft.

This script shows examples of how to programmatically configure wireless AA
from Python (useful for testing or automated configuration).
"""

# Example 1: Set to wireless-only mode (PC development)
wireless_config = {
    "connectionMode": "wireless",
    "wireless.enabled": True,
    "wireless.host": "192.168.1.100",  # Phone IP address
    "wireless.port": 5277,
}

# Example 2: Set to USB-only mode (production)
usb_config = {
    "connectionMode": "usb",
    "wireless.enabled": False,
    "wireless.host": "",
    "wireless.port": 5277,
}

# Example 3: Set to auto-fallback mode (try USB, then wireless)
auto_config = {
    "connectionMode": "auto",
    "wireless.enabled": True,
    "wireless.host": "192.168.1.100",
    "wireless.port": 5277,
}

# Example 4: Using mDNS hostname instead of IP
mdns_config = {
    "connectionMode": "wireless",
    "wireless.enabled": True,
    "wireless.host": "my-phone.local",  # mDNS hostname
    "wireless.port": 5277,
}

if __name__ == "__main__":
    print("Wireless Android Auto Configuration Examples")
    print("=" * 50)
    print()
    
    print("1. WIRELESS MODE (PC Development):")
    print(f"   Mode: wireless")
    print(f"   Host: {wireless_config['wireless.host']}")
    print(f"   Port: {wireless_config['wireless.port']}")
    print()
    
    print("2. USB MODE (Production - Default):")
    print(f"   Mode: usb")
    print(f"   USB device detection enabled")
    print()
    
    print("3. AUTO MODE (Flexible):")
    print(f"   Mode: auto")
    print(f"   Primary: USB device detection")
    print(f"   Fallback: Wireless to {auto_config['wireless.host']}")
    print()
    
    print("4. MDNS MODE (Network Auto-Discovery):")
    print(f"   Mode: wireless")
    print(f"   Host: {mdns_config['wireless.host']} (resolves via mDNS)")
    print()
    
    print("=" * 50)
    print("Configuration in C++:")
    print()
    print("""
    // Read from ProfileManager
    DeviceConfig config = profile.devices[0];
    
    // Access wireless settings
    QString mode = config.settings["connectionMode"].toString();
    QString host = config.settings["wireless.host"].toString();
    quint16 port = config.settings["wireless.port"].toUInt();
    
    // Pass to service
    androidAutoService->configureTransport(config.settings);
    """)
    
    print()
    print("=" * 50)
    print("Debugging Tips:")
    print()
    print("1. Check logs:")
    print("   QT_LOGGING_RULES='*=true' ./crankshaft-ui | grep -i wireless")
    print()
    print("2. Verify connectivity:")
    print("   ping 192.168.1.100")
    print("   telnet 192.168.1.100 5277")
    print()
    print("3. Check firewall:")
    print("   sudo ufw allow 5277/tcp")
    print()
    print("4. Monitor connection:")
    print("   journalctl -u crankshaft-core -f | grep AndroidAuto")
