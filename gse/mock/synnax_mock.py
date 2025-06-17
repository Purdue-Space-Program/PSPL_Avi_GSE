from gse.utils import get_logger

log = get_logger('Synnax Mock')

class SynnaxMockWriter():
    def __init__(self) -> None:
        self.count = 0
        pass

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        pass

    def write(self, *args) -> None:
        self.count += 1
        if self.count % 10000 == 0:
            log.info(self.count)

class SynnaxMockStreamer():
    def __init__(self) -> None:
        pass

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        pass

    def __iter__(self):
        return self

    def __next__(self):
        pass

class SynnaxMock():
    def __init__(self) -> None:
        pass

    def open_writer(self, start, channels, enable_auto_commit) -> SynnaxMockWriter:
        return SynnaxMockWriter()

    def open_streamer(self, channels) -> SynnaxMockStreamer:
        return SynnaxMockStreamer()
