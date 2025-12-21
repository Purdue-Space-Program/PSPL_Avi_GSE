import gse.configs.constants as constants
from gse.utils import get_logger, get_telem_configs, get_synnax_client
import synnax as sy
from collections import deque
import itertools

log = get_logger('Zeroer')
telem_df = get_telem_configs()

# The size of the rolling window in seconds
WINDOW_DURATION = 10 
CHANNEL_RATE = 100 # samples per second
WINDOW_SIZE = CHANNEL_RATE * WINDOW_DURATION

try:
    client = get_synnax_client()
    log.info(f'Connected to Synnax at {constants.SYNNAX_IP}:{constants.SYNNAX_PORT}')
except Exception as e:
    log.error(f'Unable to connect to Synnax at {constants.SYNNAX_IP}:{constants.SYNNAX_PORT}: {e}')
    exit(1)

channels = [
    'PT_HE_01',
]
rate_channels = [f'{ch}_rate' for ch in channels]

synnax_rate_channels = []
synnax_rate_channel_indexes = []

for ch in rate_channels:
    time_ch = client.channels.create(
        name=f'{ch}_time',
        data_type=sy.DataType.TIMESTAMP,
        is_index=True,
        retrieve_if_name_exists=True,
    )
    synnax_rate_channel_indexes.append(time_ch)

    ch_obj = client.channels.create(
        name=ch,
        data_type=sy.DataType.FLOAT64,
        index=time_ch.key,
        retrieve_if_name_exists=True,
    )
    synnax_rate_channels.append(ch_obj.key)
    log.info(f'Added rate channel: {ch_obj.name}\t')

with client.open_streamer(channels) as streamer, \
        client.open_writer(
            start=sy.TimeStamp.now(),
            channels=synnax_rate_channels + [ch.key for ch in synnax_rate_channel_indexes],
            enable_auto_commit=True
        ) as writer:
    
    # data now uses the calculated WINDOW_SIZE for the deque maxlen
    data = {
        ch: deque(maxlen=WINDOW_SIZE) for ch in rate_channels
    }
    
    while True:
        frame = streamer.read(5 * sy.TimeSpan.MILLISECOND)
        if frame is not None:
            for ch, rate_ch in zip(channels, rate_channels):
                for v in frame[ch]:
                    data[rate_ch].append(v)

        for ch, ch_index in zip(rate_channels, [ch.key for ch in synnax_rate_channel_indexes]):
            window_data = data[ch]
            
            if len(window_data) == window_data.maxlen:
                
                midpoint = window_data.maxlen // 2
                len_second_half = window_data.maxlen - midpoint

                avg_first_half = sum(itertools.islice(window_data, 0, midpoint)) / midpoint
                
                avg_second_half = sum(itertools.islice(window_data, midpoint, None)) / len_second_half
                delta = avg_second_half - avg_first_half
                
                print(f'writing delta for {ch}: {delta:.4f}')
                writer.write({
                    ch: delta,
                    ch_index: sy.TimeStamp.now(),
                })
