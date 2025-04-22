import socketserver
import utils
from configs.constants import ADC_V_SLOPE, ADC_V_OFFSET

command_df = utils.get_command_configs()
telem_df   = utils.get_telem_configs()

class CommandHandler(socketserver.BaseRequestHandler):
    def handle(self):
        id = int.from_bytes(self.request.recv(1))
        cmd_row = command_df[command_df['ID'] == id].squeeze()

        print(cmd_row['Name'])
        nums = [int.from_bytes(self.request.recv(4), byteorder='little') for _ in range(cmd_row['Num Args'])]

        tlm_row = telem_df[telem_df['Name'] == cmd_row['Calibration Reference']].squeeze()

        for n in nums:
            if tlm_row.get('Slope', None) is not None:
                n = (((n * ADC_V_SLOPE) - ADC_V_OFFSET) * tlm_row['Slope']) + tlm_row['Offset'] + tlm_row['Zeroing Offset']
                print(n)


class CommandServer(socketserver.TCPServer):
    allow_reuse_address = True

if __name__ == '__main__':
    with CommandServer(('localhost', 1234), CommandHandler) as server:
        try:
            server.serve_forever()
        except KeyboardInterrupt:
            pass
        finally:
            server.server_close()
