#!/usr/bin/env bash
# Generates a self-signed device certificate and private key into crt/.
#
#   scripts/gen_device_cert.sh [common-name] [days]
#
# Produces crt/device.crt and crt/device.key (EC P-256, which mbedTLS on the
# ESP32 handles well), plus crt/device.crt.fingerprint holding the certificate's
# SHA-256 fingerprint (lowercase hex, no colons). Valid until 9999-12-31 unless [days] is given.
# Refuses to overwrite existing files.
set -euo pipefail

CN="${1:-esp-device}"
# Default: valid until 9999-12-31, the X.509 "no expiry" date, as devices run indefinitely.
NO_EXPIRY_DAYS=$(( ($(date -u -d 9999-12-31 +%s) - $(date -u +%s)) / 86400 ))
DAYS="${2:-$NO_EXPIRY_DAYS}"

OUT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)/crt"
CRT="$OUT_DIR/device.crt"
KEY="$OUT_DIR/device.key"
FPR="$OUT_DIR/device.crt.fingerprint"

mkdir -p "$OUT_DIR"

if [[ -e "$CRT" || -e "$KEY" || -e "$FPR" ]]; then
	echo "error: $CRT, $KEY or $FPR already exists; delete them first to regenerate" >&2
	exit 1
fi

# Relative file names: a native Windows openssl can't read Git Bash's /c/... paths.
# MSYS_NO_PATHCONV stops Git Bash rewriting the "/CN=" subject as a path.
cd "$OUT_DIR"
MSYS_NO_PATHCONV=1 openssl req -x509 -nodes -newkey ec -pkeyopt ec_paramgen_curve:prime256v1 \
	-keyout device.key -out device.crt -days "$DAYS" -subj "/CN=$CN"

# SHA-256 of the DER-encoded certificate as lowercase hex, no colons.
openssl x509 -in device.crt -noout -fingerprint -sha256 \
	| sed 's/^.*=//; s/://g' | tr 'A-F' 'a-f' > device.crt.fingerprint

chmod 600 "$KEY" 2>/dev/null || true

echo "created:"
echo "  $CRT"
echo "  $KEY"
echo "  $FPR"
