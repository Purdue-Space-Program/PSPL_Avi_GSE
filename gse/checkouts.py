from synnax.framer import Frame
import synnax as sy
import gse.configs.constants as constants
from gse.utils import get_logger, get_synnax_client
from time import sleep
import pandas as pd
log = get_logger('Checkouts')

import socket

df = pd.read_excel('gse/configs/CMS_Checkouts.xlsx', sheet_name='sensors')

channels = df[pd.notna(df['Default Value'])].squeeze()

try:
    client = get_synnax_client()
    log.info(f'Connected to Synnax at {constants.SYNNAX_IP}:{constants.SYNNAX_PORT}')
except:
    log.error(f'Unable to connect to Synnax at {constants.SYNNAX_IP}:{constants.SYNNAX_PORT}')

def check_avi_ports() -> bool:
    val = True
    for port in avi_ports:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            try:
                s.connect((constants.AVI_IP, port))
                log.info(f'Connected to Avi port: {port}')
            except:
                log.error(f'Unable to connect to Avi port: {port}')
                val = False
    return val

def check_default_states() -> bool:
    val = True
    names = channels['Name']
    sums = {ch: [0.0, 0] for ch in names}
    with client.open_streamer(list(names)) as streamer:
        for frame in streamer:
            frame = Frame(frame)
            for ch in names:
                for f in frame[ch]:
                    if sums[ch][1] < 500:
                        sums[ch][0] += f
                        sums[ch][1] += 1

            if all(sums[ch][1] >= 500 for ch in names):
                break

    for _, row in channels.iterrows():
        name = row['Name']
        def_value = row['Default Value']
        tolerance = row['Tolerance (+/-)']

        avg_val = float(sums[name][0]) / sums[name][1]

        if avg_val <= (def_value + tolerance) and avg_val >= (def_value - tolerance):
            log.info(f'{name} reading correctly, {avg_val:.1f}')
        else: 
            log.error(f'BAD {name} reading at, {avg_val:.1f}!!')
            val = False
    return val

# def check_sv_toggle():
#     with client.open_writer(
#         start=sy.TimeStamp.now(),
#         channels=cms_valves_cmd,
#         enable_auto_commit=True,
#     ) as writer:
#         for (cmd_name, pos_name) in zip(cms_valves_cmd, cms_valves_pos):
#             writer.write({
#                 cmd_name: 1,
#             })
#             # experimentally determined to be enough time
#             sleep(0.2)
#
#             if client.read_latest(pos_name) == 1:
#                 log.info(f'{cmd_name.strip('-CMD')} toggled!')
#             else:
#                 log.error(f'{cmd_name.strip('-CMD')} DID NOT toggle!')
#
#             writer.write({
#                 cmd_name: 0,
#             })

if __name__ == '__main__':
    # check_avi_ports()
    check_default_states()
    # check_sv_toggle()
