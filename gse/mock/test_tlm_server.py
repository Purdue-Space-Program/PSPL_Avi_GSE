import socket
from time import time
from random import randint
from ..utils import get_telem_configs, get_logger
import gse.configs.constants as constants
log = get_logger('Telemetry Mock Server')

telem_df = get_telem_configs()

def main():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as listener:
        listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        listener.bind((constants.AVI_IP, constants.AVI_TELEM_PORT))
        listener.listen(5)
        log.info(f"Listening on {constants.AVI_IP}:{constants.AVI_TELEM_PORT}")

        try:
            while True:
                conn, addr = listener.accept()
                log.info(f"Accepted connection from {addr}")

                try:
                    with conn:
                        while True:
                            for _, row in telem_df.iterrows():
                                packet = constants.TELEM_STRUCT.pack(
                                    int(time() * 1000000),
                                    randint(0, 2147483647),
                                    row['ID']
                                )
                                conn.sendall(packet)
                except ConnectionResetError:
                    log.info('Client disconnected...')
        except KeyboardInterrupt:
            log.info('Shutting down server...')


if __name__ == "__main__":
    main()
