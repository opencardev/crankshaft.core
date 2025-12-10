#!/bin/bash -e

# Add OpenCarDev Crankshaft
echo "Adding OpenCarDev Crankshaft..."
# This runs in chroot context so we can use normal file paths
systemctl daemon-reload
systemctl enable openauto.service
systemctl enable openauto-btservice.service

# Enable verbose logging for OpenAuto/AASDK and OpenSSL TLS trace
echo "Configuring OpenAuto debug logging..."
# Create drop-in directories for services
install -d -m 0755 /etc/systemd/system/openauto.service.d
install -d -m 0755 /etc/systemd/system/openauto-btservice.service.d

# Ensure log directory exists
install -d -m 0755 /var/log/openauto

# Drop-in for openauto.service
cat > /etc/systemd/system/openauto.service.d/10-logging.conf <<'EOF'
[Service]
# Increase application and AASDK log verbosity
Environment="AASDK_LOG_LEVEL=trace"
Environment="OPENAUTO_LOG_LEVEL=trace"

EOF

# Drop-in for openauto-btservice.service
cat > /etc/systemd/system/openauto-btservice.service.d/10-logging.conf <<'EOF'
[Service]
# Increase application and AASDK log verbosity for btservice as well
Environment="AASDK_LOG_LEVEL=trace"
Environment="OPENAUTO_LOG_LEVEL=trace"

EOF

systemctl daemon-reload
echo "OpenCarDev Crankshaft setup completed successfully"
