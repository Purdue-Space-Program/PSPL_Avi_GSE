import configs.constants as constants
from utils import get_logger, get_telem_configs, get_synnax_client
log = get_logger('Zeroer')
telem_df = get_telem_configs()

NUM_SAMPLES = 1000

client = get_synnax_client()
log.info(f'Connected to Synnax at {constants.SYNNAX_IP}:{constants.SYNNAX_PORT}')

channel_list = list(telem_df['Name'])

def main():
    # key : [count, sum]
    channel_avgs = { ch : [0, 0.0] for ch in channel_list }
    with client.open_streamer(channel_list) as s:
        for frame in s:
            for ch in channel_list:
                for f in frame[ch]:
                    channel_avgs[ch][0] += 1 # count
                    channel_avgs[ch][1] += f # sum

    for ch in channel_list:
        vals = channel_avgs[ch]
        log.info(f'{ch}: {vals[1] / vals[0]}')

if __name__ == '__main__':
    main()
