class SynnaxMockWriter():
    def __init__(self) -> None:
        pass

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        pass

    def write(self, *args) -> None:
        print(*args)

class SynnaxMock():
    def __init__(self) -> None:
        pass

    def open_writer(self, start, channels, enable_auto_commit) -> SynnaxMockWriter:
        return SynnaxMockWriter()
