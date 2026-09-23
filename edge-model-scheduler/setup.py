#!/usr/bin/env python3
import sys
import os
import sysconfig
from setuptools import setup, Extension

try:
    from pybind11.setup_helpers import Pybind11Extension, build_ext
    USE_PYBIND11_HELPERS = True
except ImportError:
    from setuptools.command.build_ext import build_ext
    Pybind11Extension = Extension
    USE_PYBIND11_HELPERS = False

__version__ = "3.3.0"

include_dirs = [
    "include",
    os.path.join(os.path.dirname(__file__), "include"),
    "/usr/include/pybind11",
    sysconfig.get_path("include"),
]

extra_compile_args = [
    "-std=c++17",
    "-O3",
    "-fvisibility=hidden",
    "-Wno-unused-parameter",
    "-Wno-unused-variable"
]

libraries = ["pthread"]
if sys.platform.startswith("linux"):
    libraries.append("rt")

ext_modules = [
    Pybind11Extension(
        "edge_scheduler_py",
        [
            "python/bindings.cpp",
            "src/ResourceMonitor.cpp",
            "src/Scheduler.cpp",
            "src/AdvancedScheduler.cpp",
            "src/TensorRTRunner.cpp",
        ],
        include_dirs=include_dirs,
        extra_compile_args=extra_compile_args,
        libraries=libraries,
        cxx_std=17,
    ),
]

setup(
    name="edge-model-scheduler",
    version=__version__,
    author="Vikas Samvel",
    author_email="engineering@vikassamvel.dev",
    url="https://github.com/VIKASSAMVEL/edge-model-scheduler",
    description="Deterministic Low-Latency AI Co-Scheduler for Edge SoCs",
    long_description=open("README.md", "r", encoding="utf-8").read() if os.path.exists("README.md") else "",
    long_description_content_type="text/markdown",
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
    python_requires=">=3.8",
)
