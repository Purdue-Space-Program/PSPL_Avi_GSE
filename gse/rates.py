import gse.configs.constants as constants
from gse.utils import get_logger, get_telem_configs, get_synnax_client
import synnax as sy
from collections import deque
log = get_logger('Zeroer')
telem_df = get_telem_configs()

CHANNEL_RATE = 100 # samples per second

try:
    client = get_synnax_client()
    log.info(f'Connected to Synnax at {constants.SYNNAX_IP}:{constants.SYNNAX_PORT}')
except:
    log.error(f'Unable to connect to Synnax at {constants.SYNNAX_IP}:{constants.SYNNAX_PORT}')
    exit(1)

channels = [
    'PT-HE-01',
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

    ch = client.channels.create(
        name=f'{ch}',
        data_type=sy.DataType.FLOAT64,
        index=time_ch.key,
        retrieve_if_name_exists=True,
    )
    synnax_rate_channels.append(ch)
    log.info(f' Added rate channel: {ch}\t')

with client.open_streamer(channels) as streamer, \
        client.open_writer(
            start=sy.TimeStamp.now(), 
            channels=synnax_rate_channels + synnax_rate_channel_indexes, 
            enable_auto_commit=True
        ) as writer:
    data = {
        ch: deque(maxlen=CHANNEL_RATE * 10) for ch in rate_channels
    }
    
    while True:
        frame = streamer.read(5 * sy.TimeSpan.MILLISECOND)
        if frame is not None:
            for ch, rate_ch in zip(channels, rate_channels):
                for v in frame[ch]:
                    data[rate_ch].append(v)

        for ch, ch_index in zip(rate_channels, synnax_rate_channel_indexes):
            print(len(data[ch]))
            if len(data[ch]) >= CHANNEL_RATE*10:
                print(f'writing {data[ch][-1] - data[ch][0]}')
                writer.write({
                    ch: data[ch][-1] - data[ch][0],
                    ch_index: sy.TimeStamp.now(),
                })
