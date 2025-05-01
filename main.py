from flask import Flask, jsonify
import requests
import os
import logging

logging.basicConfig(
    level=os.getenv("LOG_LEVEL", "INFO"),
    format="%(asctime)s [%(levelname)s] 1#1: %(message)s",
    datefmt="%Y/%m/%d %H:%M:%S",
)

app = Flask(__name__)

PROXMOX_HOST = os.getenv("PROXMOX_HOST", None)
PROXMOX_PORT = os.getenv("PROXMOX_PORT", 8006)
USERNAME = os.getenv("USERNAME", "root@pam")
PASSWORD = os.getenv("PASSWORD", None)
NODE_NAME = os.getenv("NODE_NAME", "proxmox")

def get_proxmox_status():
    
    auth_url = f"https://{PROXMOX_HOST}:{PROXMOX_PORT}/api2/json/access/ticket"
    auth_data = {"username": USERNAME, "password": PASSWORD}
    response = requests.post(auth_url, data=auth_data, verify=False)
    
    if response.status_code != 200:
        logging.error("Fail to authenticate on Proxmox")
        raise Exception("Fail to authenticate on Proxmox")
    
    ticket = response.json().get('data', {}).get('ticket')
    cookie = {"PVEAuthCookie": ticket}
    
    
    status_url = f"https://{PROXMOX_HOST}:{PROXMOX_PORT}/api2/json/nodes/{NODE_NAME}/status"
    response = requests.get(status_url, cookies=cookie, verify=False)
    
    if response.status_code != 200:
        logging.error("Fail to get Proxmox status")
        raise Exception("Fail to get Proxmox status")
    
    status_data = response.json().get('data', {})
    
    cpu = status_data.get('cpu', 0)
    memory_used = status_data.get('memory', {}).get('used', 0)
    memory_total = status_data.get('memory', {}).get('total', 0)
    disk_used = status_data.get('rootfs', {}).get('used', 0)
    disk_total = status_data.get('rootfs', {}).get('total', 0)
    
    cpu_percent = cpu * 100
    memory_percent = (memory_used / memory_total) * 100 if memory_total else 0
    disk_percent = (disk_used / disk_total) * 100 if disk_total else 0
    
    result = {
        "cpu_percent": round(cpu_percent, 2),
        "disk_percent": round(disk_percent, 2),
        "disk_total_bytes": disk_total,
        "disk_used_bytes": disk_used,
        "memory_percent": round(memory_percent, 2),
        "memory_total_bytes": memory_total,
        "memory_used_bytes": memory_used
    }
    return result

@app.route('/')
def index():
    try:
        data = get_proxmox_status()
        return jsonify(data)  
    except Exception as e:
        return jsonify({"error": str(e)}), 500

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8000)
