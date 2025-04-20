import synnax as sy
from configs.constants import SYNNAX_IP, SYNNAX_PORT, SYNNAX_USERNAME, SYNNAX_PASSWORD

import logging
from logging import NOTSET, DEBUG, INFO, WARN, WARNING, ERROR, FATAL, CRITICAL

import pandas as pd

FORMAT = '[%(levelname)s] %(name)s: %(message)s'


def get_logger(name: str, level: int = logging.INFO):
    log = logging.getLogger(name)
    log.setLevel(level)

    # only add handler if it doesn't already have one
    if not log.handlers:
        formatter = logging.Formatter(FORMAT)
        ch = logging.StreamHandler()
        ch.setFormatter(formatter)
        log.addHandler(ch)

    # prevent double‑logging via root
    log.propagate = False

    return log

# Common Synnax client
sy_client = sy.Synnax(
    host=SYNNAX_IP,
    port=SYNNAX_PORT,
    username=SYNNAX_USERNAME,
    password=SYNNAX_PASSWORD,
    secure=False,
)

def get_synnax_client():
    return sy_client

telem_config_df = pd.read_excel('gses/configs/CMS_Avionics_Channels.xlsx', sheet_name='telem_channels')
def get_telem_configs():
    return telem_config_df

command_config_df = pd.read_excel('gses/configs/CMS_Avionics_Channels.xlsx', sheet_name='command_channels')
def get_command_configs():
    return command_config_df
