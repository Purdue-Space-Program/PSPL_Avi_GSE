import struct

PT_ZEROING_TARGET = 14.7

PT_OX_201_ZERO_OFFSET = 0
PT_FU_201_ZERO_OFFSET = 0
PT_HE_201_ZERO_OFFSET = 0

ADC_V_SLOPE  =  0.00000000235714724017
ADC_V_OFFSET = -0.01390133824020600000

##################
TEST_MODE = True
##################

AVI_CMD_PORT   = 1234
AVI_TELEM_PORT = 25565
SYNNAX_PORT    = 9090

if TEST_MODE:
    SYNNAX_IP = 'localhost'
    AVI_IP    = 'localhost'
else:
    SYNNAX_IP = '128.46.118.59'
    AVI_IP    = '128.46.118.59'

SYNNAX_USERNAME = "Bill"
SYNNAX_PASSWORD = "Bill"

TELEM_FORMAT = '<QQQ'
TELEM_STRUCT = struct.Struct(TELEM_FORMAT)
TELEM_SIZE   = TELEM_STRUCT.size

LOX_CHANNEL_NAME     = 'PT-OX-201'
FUEL_CHANNEL_NAME    = 'PT-FU-201'
HELIUM_CHANNEL_NAME  = 'PT-HE-201'
BB_OPEN_CHANNEL_NAME = 'BB_OPEN_ALL_CMD'
BB_ISO_CHANNEL_NAME  = 'BB_ISO_ALL_CMD'
BB_REG_CHANNEL_NAME  = 'BB_REG_ALL_CMD'
BB_FU_UPPER_SETP_NAME = 'BB_SET_FU_UPPER_SETP'
BB_OX_UPPER_SETP_NAME = 'BB_SET_OX_UPPER_SETP'
BB_FU_LOWER_SETP_NAME = 'BB_SET_FU_LOWER_SETP'
BB_OX_LOWER_SETP_NAME = 'BB_SET_OX_LOWER_SETP'
BB_FU_UPPER_REDLINE_NAME = 'BB_SET_FU_UPPER_REDLINE'
BB_OX_UPPER_REDLINE_NAME = 'BB_SET_OX_UPPER_REDLINE'
BB_FU_LOWER_REDLINE_NAME = 'BB_SET_FU_LOWER_REDLINE'
BB_OX_LOWER_REDLINE_NAME = 'BB_SET_OX_LOWER_REDLINE'

FUEL_SOLENOID_NAME = 'SV-HE-202'
LOX_SOLENOID_NAME  = 'SV-HE-201'
