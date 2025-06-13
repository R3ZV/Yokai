import socket

class Connection:
    def __init__(self, port: int):
        self.port = port
        self.sock = None

    def init_client(self, ip: str):
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.connect((ip, self.port))
            return None  # Success
        except socket.error as e:
            return e  # Error object

    def send_msg(self, msg: str):
        try:
            self.sock.sendall(msg.encode('utf-8'))
            return None
        except socket.error as e:
            return e
    def receive_msg(self, bufsize: int = 4096) -> str | None:
        try:
            data = self.sock.recv(bufsize)
            return data.decode('utf-8')
        except:
            return None
