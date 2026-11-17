"""Shared fixtures for ops_translator unit tests."""
from argparse import Namespace
from pathlib import Path

import pytest

import config


@pytest.fixture(autouse=True)
def global_args(tmp_path, monkeypatch):
    """Provide a default `config.global_args` for every test.

    Several translator passes read `config.global_args` (normally set by
    __main__.main from the CLI). Tests get a sane default here and can
    override individual fields, e.g. `global_args.hls = True`.
    Runs inside tmp_path so any dumped graphs/images don't litter the repo.
    """
    args = Namespace(
        verbose=False,
        debug=False,
        dump=False,
        out=str(tmp_path),
        config={},
        force_soa=False,
        ast_dump=False,
        suffix="_ops",
        I=[],
        i=[],
        D=[],
        target=[],
        hls=False,
        tapa=False,
        df_img_format="png",
    )
    monkeypatch.setattr(config, "global_args", args)
    monkeypatch.chdir(tmp_path)
    return args


@pytest.fixture
def write_cpp(tmp_path):
    """Write a C++ snippet to a temp file and return its Path."""
    def _write(source: str, name: str = "snippet.cpp") -> Path:
        path = tmp_path / name
        path.write_text(source)
        return path
    return _write