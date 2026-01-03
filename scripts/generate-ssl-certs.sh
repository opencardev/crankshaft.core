#!/bin/bash

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

set -eu

# Default values
CERT_DIR="${CERT_DIR:-./.certs}"
CERT_NAME="${CERT_NAME:-crankshaft-server}"
KEY_NAME="${KEY_NAME:-crankshaft-server}"
DAYS="${DAYS:-365}"
COUNTRY="${COUNTRY:-GB}"
STATE="${STATE:-England}"
CITY="${CITY:-London}"
ORG="${ORG:-OpenCarDev}"
CN="${CN:-crankshaft.local}"

# Function to display usage
usage() {
  echo "Usage: $0 [OPTIONS]"
  echo ""
  echo "Generate self-signed SSL/TLS certificates for Crankshaft WebSocket server"
  echo ""
  echo "Options:"
  echo "  --cert-dir DIR      Directory to store certificates (default: ./.certs)"
  echo "  --cert-name NAME    Certificate name (default: crankshaft-server)"
  echo "  --key-name NAME     Key name (default: crankshaft-server)"
  echo "  --days DAYS         Certificate validity in days (default: 365)"
  echo "  --country C         Country code (default: GB)"
  echo "  --state S           State/Province (default: England)"
  echo "  --city C            City (default: London)"
  echo "  --org ORG           Organization (default: OpenCarDev)"
  echo "  --cn CN             Common Name (default: crankshaft.local)"
  echo "  --help              Display this help message"
  echo ""
  echo "Environment variables:"
  echo "  CERT_DIR, CERT_NAME, KEY_NAME, DAYS, COUNTRY, STATE, CITY, ORG, CN"
  echo ""
  echo "Examples:"
  echo "  $0"
  echo "  $0 --cert-dir /etc/crankshaft/certs --days 730 --cn myvehicle.local"
  echo "  CERT_DIR=/opt/certs $0"
  exit 0
}

# Parse command-line arguments
while [[ $# -gt 0 ]]; do
  case $1 in
    --cert-dir)
      CERT_DIR="$2"
      shift 2
      ;;
    --cert-name)
      CERT_NAME="$2"
      shift 2
      ;;
    --key-name)
      KEY_NAME="$2"
      shift 2
      ;;
    --days)
      DAYS="$2"
      shift 2
      ;;
    --country)
      COUNTRY="$2"
      shift 2
      ;;
    --state)
      STATE="$2"
      shift 2
      ;;
    --city)
      CITY="$2"
      shift 2
      ;;
    --org)
      ORG="$2"
      shift 2
      ;;
    --cn)
      CN="$2"
      shift 2
      ;;
    --help)
      usage
      ;;
    *)
      echo "Unknown option: $1"
      usage
      ;;
  esac
done

# Create certificate directory
mkdir -p "$CERT_DIR"

# File paths
CERT_FILE="$CERT_DIR/${CERT_NAME}.crt"
KEY_FILE="$CERT_DIR/${KEY_NAME}.key"

echo "Generating SSL/TLS certificates for Crankshaft..."
echo "  Certificate directory: $CERT_DIR"
echo "  Certificate file: $CERT_FILE"
echo "  Key file: $KEY_FILE"
echo "  Validity: $DAYS days"
echo "  Common Name: $CN"
echo ""

# Check if OpenSSL is available
if ! command -v openssl &> /dev/null; then
  echo "Error: openssl not found. Please install openssl package."
  echo "  Ubuntu/Debian: sudo apt install openssl"
  exit 1
fi

# Generate private key and self-signed certificate
openssl req -x509 \
  -newkey rsa:2048 \
  -keyout "$KEY_FILE" \
  -out "$CERT_FILE" \
  -days "$DAYS" \
  -nodes \
  -subj "/C=$COUNTRY/ST=$STATE/L=$CITY/O=$ORG/CN=$CN"

# Set secure permissions
chmod 600 "$KEY_FILE"
chmod 644 "$CERT_FILE"

echo ""
echo "✓ SSL/TLS certificates generated successfully!"
echo ""
echo "Certificate info:"
openssl x509 -in "$CERT_FILE" -text -noout | grep -E "Subject:|Issuer:|Not Before|Not After|Public-Key"

echo ""
echo "Configuration for Crankshaft:"
echo "  Add to core/main.cpp or configuration:"
echo "    websocketServer->enableSecureMode(\"$CERT_FILE\", \"$KEY_FILE\");"
echo ""
echo "Or set environment variables:"
echo "  export CRANKSHAFT_SSL_CERT=\"$CERT_FILE\""
echo "  export CRANKSHAFT_SSL_KEY=\"$KEY_FILE\""
echo ""
echo "Client connection (wss):"
echo "  wss://crankshaft.local:9003"
echo ""
echo "Warning: Self-signed certificates will show warnings in browsers/clients."
echo "         For production, use certificates from a trusted CA."
