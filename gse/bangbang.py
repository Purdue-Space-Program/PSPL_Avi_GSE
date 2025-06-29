from synnax.framer import Frame
import gse.command as cmd
import gse.configs.constants as constants
from gse.utils import get_logger, get_synnax_client
log = get_logger('Command Proxy')

client = get_synnax_client()
log.info(f'Connected to Synnax at {constants.SYNNAX_IP}:{constants.SYNNAX_PORT}')

channels = [
    constants.FUEL_SOLENOID_NAME + '-CMD',
    constants.LOX_SOLENOID_NAME  + '-CMD',
    constants.BB_OPEN_CHANNEL_NAME,
    constants.BB_ISO_CHANNEL_NAME,
    constants.BB_REG_CHANNEL_NAME,
    constants.BB_FU_UPPER_SETP_NAME,
    constants.BB_OX_UPPER_SETP_NAME,
    constants.BB_FU_LOWER_SETP_NAME,
    constants.BB_OX_LOWER_SETP_NAME,
    constants.BB_FU_UPPER_REDLINE_NAME,
    constants.BB_OX_UPPER_REDLINE_NAME,
    constants.BB_FU_LOWER_REDLINE_NAME,
    constants.BB_OX_LOWER_REDLINE_NAME,
]

def main():
    with client.open_streamer(channels) as s:
        for frame in s:
            frame = Frame(frame)
            for f in frame[constants.FUEL_SOLENOID_NAME + '-CMD']:
                if f == 1:
                    cmd.send_command(cmd.Command.SET_FU_STATE_OPEN.name)
                    log.info('Fuel: OPEN')
                if f == 0:
                    cmd.send_command(cmd.Command.SET_FU_STATE_ISOLATE.name)
                    log.info('Fuel: CLOSE')

            for f in frame[constants.LOX_SOLENOID_NAME + '-CMD']:
                if f == 1:
                    cmd.send_command(cmd.Command.SET_OX_STATE_OPEN.name)
                    log.info('Lox: OPEN')
                if f == 0:
                    cmd.send_command(cmd.Command.SET_OX_STATE_ISOLATE.name)
                    log.info('Lox: CLOSE')

            for f in frame[constants.BB_OPEN_CHANNEL_NAME]:
                if f == 1:
                    cmd.send_command(cmd.Command.SET_BB_STATE_OPEN.name)
                    log.info('Open all')

            for f in frame[constants.BB_ISO_CHANNEL_NAME]:
                if f == 1:
                    cmd.send_command(cmd.Command.SET_BB_STATE_ISOLATE.name)
                    log.info('Isolate all')

            for f in frame[constants.BB_REG_CHANNEL_NAME]:
                if f == 1:
                    cmd.send_command(cmd.Command.SET_BB_STATE_REGULATE.name)
                    log.info('Regulate all')

            for f in frame[constants.BB_FU_UPPER_SETP_NAME]:
                cmd.send_command(cmd.Command.SET_FU_UPPER_SETP.name, [float(f)])
                log.info(f'Set upper fuel setpoint to {f}')

            for f in frame[constants.BB_OX_UPPER_SETP_NAME]:
                cmd.send_command(cmd.Command.SET_OX_UPPER_SETP.name, [float(f)])
                log.info(f'Set upper lox setpoint to {f}')

            for f in frame[constants.BB_FU_LOWER_SETP_NAME]:
                cmd.send_command(cmd.Command.SET_FU_LOWER_SETP.name, [float(f)])
                log.info(f'Set upper fuel setpoint to {f}')

            for f in frame[constants.BB_OX_LOWER_SETP_NAME]:
                cmd.send_command(cmd.Command.SET_OX_LOWER_SETP.name, [float(f)])
                log.info(f'Set upper lox setpoint to {f}')

            for f in frame[constants.BB_FU_UPPER_REDLINE_NAME]:
                cmd.send_command(cmd.Command.SET_FU_UPPER_REDLINE.name, [float(f)])
                log.info(f'Set upper fuel setpoint to {f}')

            for f in frame[constants.BB_OX_UPPER_REDLINE_NAME]:
                cmd.send_command(cmd.Command.SET_OX_UPPER_REDLINE.name, [float(f)])
                log.info(f'Set upper lox setpoint to {f}')

            for f in frame[constants.BB_FU_LOWER_REDLINE_NAME]:
                cmd.send_command(cmd.Command.SET_FU_LOWER_REDLINE.name, [float(f)])
                log.info(f'Set upper fuel setpoint to {f}')

            for f in frame[constants.BB_OX_LOWER_REDLINE_NAME]:
                cmd.send_command(cmd.Command.SET_OX_LOWER_REDLINE.name, [float(f)])
                log.info(f'Set upper lox setpoint to {f}')

if __name__ == '__main__':
    main()
