import socketserver
import pandas as pd
import gse.utils as utils
from gse.configs.constants import ADC_V_SLOPE, ADC_V_OFFSET
log = utils.get_logger('Command Mock Server')

command_df = utils.get_command_configs()
telem_df   = utils.get_telem_configs()

class CommandHandler(socketserver.BaseRequestHandler):
    def handle(self):
        id = int.from_bytes(self.request.recv(1))
        cmd_row = command_df[command_df['ID'] == id].squeeze()
        tlm_row = telem_df[telem_df['Name'] == cmd_row['Calibration Reference']].squeeze()
        nums = [int.from_bytes(self.request.recv(4), byteorder='little') for _ in range(cmd_row['Num Args'])]

        if not isinstance(tlm_row['Slope'], pd.Series):
            for n in nums:
                n = (((n * ADC_V_SLOPE) + ADC_V_OFFSET) * tlm_row['Slope']) + tlm_row['Offset'] + tlm_row['Zeroing Offset']
                log.info(f'[Command] {cmd_row['Name']}: {n:.1f}')
        else:
            log.info(f'[Command] {cmd_row['Name']}')

class CommandServer(socketserver.TCPServer):
    allow_reuse_address = True

if __name__ == '__main__':
    with CommandServer(('localhost', 1234), CommandHandler) as server:
        log.info('Listening on localhost:1234')
        try:
            server.serve_forever()
        except KeyboardInterrupt:
            pass
        finally:
            server.server_close()
