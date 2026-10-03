import socket
import struct
import time
import sys

def send_packages(host, port, count, interval_ns):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
    sock.connect((host, port))
    
    print(f"Connected to {host}:{port}")
    print(f"Sending {count} packages")

    for i in range(count):
        timestamp = int(time.time_ns())
        order_id = i
        price = 10000 + i 
        quantity = 100 + i
        side = 0

        pkg = struct.pack(">QQIIB7x", timestamp, order_id, price, quantity, side)
        sock.send(pkg)


        if (i + 1) % 10000 == 0:
            print(f" Sent {i + 1} packages...")


    print("EVERYTHING SENT")
    sock.close()

if __name__ == "__main__":
    host = "localhost"
    port = 8080
    count = int(sys.argv[1]) if len(sys.argv) > 1 else 10000
    interval_ns = int(sys.argv[2]) if len(sys.argv) > 2 else 0 

    print("START SEND")

    send_packages(host, port, count, interval_ns)
