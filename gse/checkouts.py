from synnax.framer import Frame
import synnax as sy
import gse.configs.constants as constants
from gse.utils import get_logger, get_synnax_client
from time import sleep
log = get_logger('Checkouts')

import socket

PT_AVERAGING_WINDOW = 100
PT_AMBIENT_TOLERANCE = 0.7
pt_channels = [
    constants.LOX_CHANNEL_NAME,
    constants.FUEL_CHANNEL_NAME,
    constants.HELIUM_CHANNEL_NAME,
]

avi_ports = [
    constants.AVI_TELEM_PORT,
    constants.AVI_CMD_PORT,
]

cms_valves_cmd = [
    constants.FUEL_SOLENOID_NAME + '-CMD',
    constants.LOX_SOLENOID_NAME  + '-CMD',
]

cms_valves_pos = [
    constants.FUEL_SOLENOID_NAME + '-position',
    constants.LOX_SOLENOID_NAME  + '-position',
]

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

def check_pt_values() -> bool:
    val = True
    sums = {ch: [0.0, 0] for ch in pt_channels}
    with client.open_streamer(pt_channels) as streamer:
        for frame in streamer:
            frame = Frame(frame)
            for ch in pt_channels:
                for f in frame[ch]:
                    if sums[ch][1] < 1000:
                        sums[ch][0] += f
                        sums[ch][1] += 1

            if all(sums[ch][1] >= 1000 for ch in pt_channels):
                break

    for ch in sums.keys():
        avg_val = float(sums[ch][0]) / sums[ch][1]
        if avg_val < (14.7 + PT_AMBIENT_TOLERANCE) and avg_val > (14.7-PT_AMBIENT_TOLERANCE):
            log.info(f'{ch} reading correctly, {avg_val:.1f} psi')
        else: 
            log.error(f'BAD {ch} reading at, {avg_val:.1f} psi!!')
            val = False
    return val

def check_sv_toggle():
    with client.open_writer(
        start=sy.TimeStamp.now(),
        channels=cms_valves_cmd,
        enable_auto_commit=True,
    ) as writer:
        for (cmd_name, pos_name) in zip(cms_valves_cmd, cms_valves_pos):
            writer.write({
                cmd_name: 1,
            })
            # experimentally determined to be enough time
            sleep(0.2)

            if client.read_latest(pos_name) == 1:
                log.info(f'{cmd_name.strip('-CMD')} toggled!')
            else:
                log.error(f'{cmd_name.strip('-CMD')} DID NOT toggle!')

            writer.write({
                cmd_name: 0,
            })

if __name__ == '__main__':
    # check_avi_ports()
    check_pt_values()
    check_sv_toggle()
