import synnax as sy
import pandas as pd
import constants

import logging
log = logging.getLogger(' Data Redux')
logging.basicConfig(level=logging.INFO)

RANGE_NAME = 'Cold Flow'

REED_TIME_PREFIX = 'Dev5_di_time_'
FU_REED_NAME = 'REED-FU-MAROTTA'
OX_REED_NAME = 'REED-OX-MAROTTA'

# rocket is ... seconds ahead (baseline for rocket side data)
ROCKET_GROUND_TIME_OFFSET = 12.3319096

channels = [
    'PT-FU-201',
    'PT-OX-201',
    'PT-HE-201',
    'SV-HE-202-position',
    'SV-HE-201-position',
]

time_channels = [ch + '_time' for ch in channels] + [REED_TIME_PREFIX + FU_REED_NAME, REED_TIME_PREFIX + OX_REED_NAME]
print(time_channels)

client = sy.Synnax(
    host=constants.SYNNAX_IP,
    port=constants.SYNNAX_PORT,
    username="Bill",
    password="Bill",
    secure=False,
)
log.info(f' Connected to Synnax at {constants.SYNNAX_IP}:{constants.SYNNAX_PORT}')
rng = client.ranges.retrieve(name=RANGE_NAME)

df = pd.read_excel('tools/CMS_Avionics_Channels.xlsx', 'channels')

data = dict()
for _, row in df.iterrows():
    time = f'{row['Name']}_time'
    name = row['Name']

    time_data   = rng[time]
    actual_data = rng[name]

    if not bool(pd.isna(row['Slope'])):
        actual_data = (((actual_data * constants.ADC_V_SLOPE) - constants.ADC_V_OFFSET) * row['Slope']) + row['Offset']
    match row['Name']:
        case 'PT-FU-201' | 'FU_UPPER_SETP' | 'FU_LOWER_SETP' | 'FU_UPPER_REDLINE' | 'FU_LOWER_REDLINE':
            actual_data += 14.7 - 46.0258
        case 'PT-OX-201' | 'OX_UPPER_SETP' | 'OX_LOWER_SETP' | 'OX_UPPER_REDLINE' | 'OX_LOWER_REDLINE':
            actual_data += 14.7 - 47.0573
        case 'PT-HE-201':
            actual_data += 14.7 - 293.2778

    data[name] = (time_data, actual_data)

if __name__ == '__main__':
    df = pd.DataFrame()

    df[FU_REED_NAME] = pd.Series(rng[FU_REED_NAME])
    df[REED_TIME_PREFIX + FU_REED_NAME] = pd.Series(rng[REED_TIME_PREFIX + FU_REED_NAME]) + ROCKET_GROUND_TIME_OFFSET * 1e9
    df[OX_REED_NAME] = pd.Series(rng[OX_REED_NAME])
    df[REED_TIME_PREFIX + OX_REED_NAME] = pd.Series(rng[REED_TIME_PREFIX + OX_REED_NAME]) + ROCKET_GROUND_TIME_OFFSET * 1e9

    for ch in channels:
        df[ch] = pd.Series(data[ch][1])
        df[ch + '_time'] = pd.Series(data[ch][0])
        log.info(f' Processed channel: {ch}')

    start_time = min([df[tch][0] for tch in time_channels])

    for tch in time_channels:
        # start time and convert to milliseconds
        df[tch] = (df[tch] - start_time) / 1e6

    df.to_csv('cf4_rocket_size.csv', index=False)
