# PSPL Avi Ground Software
This repo is in the middle of a major refactor, reference the `refactor` branch for
the latest changes.

## Getting started
### Initialize Virtual Environment
#### uv
```
uv sync
source .venv/bin/activate
```

#### pyenv
```
python3 -m venv .venv
source .venv/bin/activate
```

### Startup
Run `./run` or `./tools/run_mock` depending on whether you are using the mock servers.

If you do not have tmux installed, you must start each process individually:
```
python3 -m gse.mock.test_tlm_server
python3 -m gse.mock.test_cmd_server
python3 -m gse.telem
python3 -m gse.command
``
