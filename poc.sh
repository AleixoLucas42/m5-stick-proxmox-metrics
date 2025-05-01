#!/bin/bash

PROXMOX_HOST=""
PROXMOX_PORT="8006"
USERNAME="root@pam"                    
PASSWORD=""             
NODE_NAME="proxmox"                

echo "[+] Autenticando no Proxmox..."
AUTH_JSON=$(curl -sk -X POST \
  -d "username=${USERNAME}&password=${PASSWORD}" \
  "https://${PROXMOX_HOST}:${PROXMOX_PORT}/api2/json/access/ticket")

TICKET=$(echo "$AUTH_JSON" | jq -r '.data.ticket')
COOKIE="PVEAuthCookie=${TICKET}"

if [[ -z "$TICKET" || "$TICKET" == "null" ]]; then
  echo "[-] Falha ao autenticar no Proxmox"
  exit 1
fi

echo "[+] Buscando status do nó: $NODE_NAME"
STATUS_JSON=$(curl -sk -H "Cookie: ${COOKIE}" \
  "https://${PROXMOX_HOST}:${PROXMOX_PORT}/api2/json/nodes/${NODE_NAME}/status")

CPU=$(echo "$STATUS_JSON" | jq '.data.cpu')
MEM_USED=$(echo "$STATUS_JSON" | jq '.data.memory.used')
MEM_TOTAL=$(echo "$STATUS_JSON" | jq '.data.memory.total')

DISK_USED=$(echo "$STATUS_JSON" | jq '.data.rootfs.used')
DISK_TOTAL=$(echo "$STATUS_JSON" | jq '.data.rootfs.total')
DISK_PERCENT=$(echo "$DISK_USED / $DISK_TOTAL * 100" | bc -l | xargs printf "%.2f")

CPU_PERCENT=$(echo "$CPU * 100" | bc -l | xargs printf "%.2f")
MEM_PERCENT=$(echo "$MEM_USED / $MEM_TOTAL * 100" | bc -l | xargs printf "%.2f")

MEM_USED_FMT=$(numfmt --to=iec-i --suffix=B "$MEM_USED")
MEM_TOTAL_FMT=$(numfmt --to=iec-i --suffix=B "$MEM_TOTAL")

DISK_USED_FMT=$(numfmt --to=iec-i --suffix=B "$DISK_USED")
DISK_TOTAL_FMT=$(numfmt --to=iec-i --suffix=B "$DISK_TOTAL")

echo ""
echo "==== STATUS DO NÓ ===="
echo "CPU em uso:     $CPU_PERCENT %"
echo "Memória em uso: $MEM_PERCENT %"
echo "Memória usada:  $MEM_USED_FMT de $MEM_TOTAL_FMT"
echo "Disco em uso:   $DISK_PERCENT %"
echo "Disco usado:    $DISK_USED_FMT de $DISK_TOTAL_FMT"

