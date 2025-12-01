import socket
import struct

# --- CONFIGURATION ---
# Listen on ALL network interfaces (WiFi, Ethernet, etc.)
UDP_IP = "0.0.0.0" 
# MUST MATCH 'TARGET_PORT' in your common.h
UDP_PORT = 8080   

def start_server():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    
    try:
        sock.bind((UDP_IP, UDP_PORT))
        print(f"✅ Listening for IMU data on port {UDP_PORT}...")
        print("Press Ctrl+C to stop.\n")
    except Exception as e:
        print(f"❌ Error binding to port {UDP_PORT}. Is another program using it?")
        print(f"Error details: {e}")
        return

    # Expected Struct Format:
    # < = Little Endian (Standard for ESP32)
    # f = float (4 bytes) * 7 (w, x, y, z, ax, ay, az)
    # B = unsigned char (1 byte) * 1 (tracker_id)
    # Total size = 29 bytes
    unpack_format = "<fffffffB"
    expected_size = struct.calcsize(unpack_format)

    while True:
        try:
            data, addr = sock.recvfrom(1024) # buffer size is 1024 bytes
            
            if len(data) == expected_size:
                # Unpack the binary data into variables
                w, x, y, z, ax, ay, az, t_id = struct.unpack(unpack_format, data)
                
                # Print formatted output
                print(f"[Tracker {t_id}] "
                      f"Quat: ({w:.2f}, {x:.2f}, {y:.2f}, {z:.2f}) "
                      f"Accel: ({ax:.2f}, {ay:.2f}, {az:.2f})")
            else:
                print(f"⚠️ Received packet of unexpected size: {len(data)} bytes (Expected {expected_size})")

        except KeyboardInterrupt:
            print("\nStopping server...")
            break
        except Exception as e:
            print(f"Error processing packet: {e}")

if __name__ == "__main__":
    start_server()