import os
import platform
import re
import subprocess
import sys
import sysconfig
from distutils.version import LooseVersion
from typing import Any, Dict

from setuptools.command.build_ext import build_ext
from setuptools.extension import Extension


class CMakeExtension(Extension):
    name: str  # exists, even though IDE doesn't find it

    def __init__(self, name: str, sourcedir: str="") -> None:
        super().__init__(name, sources=[])
        self.sourcedir = os.path.abspath(sourcedir)
        self.sourcedir_rel = sourcedir


class ExtensionBuilder(build_ext):
    def run(self) -> None:
        self.validate_cmake()
        super().run()

    def build_extension(self, ext: Extension) -> None:
        if isinstance(ext, CMakeExtension):
            self.build_cmake_extension(ext)
        else:
            super().build_extension(ext)

    def validate_cmake(self) -> None:
        cmake_extensions = [x for x in self.extensions if isinstance(x, CMakeExtension)]
        if len(cmake_extensions) > 0:
            try:
                out = subprocess.check_output(["cmake", "--version"])
            except OSError:
                raise RuntimeErrorPython_CORIOLISDIR(
                    "CMake must be installed to build the following extensions: "
                    + ", ".join(e.name for e in cmake_extensions)
                )
            if platform.system() == "Windows":
                cmake_version = LooseVersion(re.search(r"version\s*([\d.]+)", out.decode()).group(1))  # type: ignore
                if cmake_version < "3.1.0":
                    raise RuntimeError("CMake >= 3.1.0 is required on Windows")

    def build_cmake_extension(self, ext: CMakeExtension) -> None:
        extdir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))
        cmake_args = []

        cfg = "Debug" if self.debug else "Release"
        # cfg = 'Debug'
        build_args = ["--config", cfg]
        install_args = ["--config", cfg]

        if platform.system() == "Windows":
            if sys.maxsize > 2 ** 32:
                cmake_args += ["-A", "x64"]
            build_args += ["--", "/m"]
        else:
            cmake_args += ["-DCMAKE_BUILD_TYPE=" + cfg]
            build_args += ["--", "-j4", "VERBOSE=1"]
        cmake_args += ["-DPYTHON_INCLUDE_DIR={}".format(sysconfig.get_path("include"))]

        env = os.environ.copy()
        env["CXXFLAGS"] = '{} -DVERSION_INFO=\\"{}\\"'.format(env.get("CXXFLAGS", ""), self.distribution.get_version())

        build_dir = os.path.join(self.build_temp, ext.sourcedir_rel)
        os.makedirs(build_dir,exist_ok=True)

        cmake_args += [f"-DCMAKE_MODULE_PATH={os.path.abspath('bootstrap/cmake_modules')};"
                       f"{os.path.join(extdir, 'share/cmake/Modules')}"]
        cmake_args += [f"-DCMAKE_INSTALL_PREFIX={extdir}"]
        cmake_args += [f"-DPython_CORIOLISLIB={extdir}"]
        cmake_args += [f"-DPython_CORIOLISARCH={extdir}"]
        cmake_args += [f"-DSYS_CONF_DIR={extdir}"]
        cmake_args += [f"-DCORIOLIS_TOP={extdir}"]
        cmake_args += [f"-DCORIOLIS_USER_TOP={extdir}"]
        cmake_args += [f"-DPOETRY=1"]
        cmake_args += [f"-DWITH_QT5=1"]
    

        subprocess.check_call(["cmake", "--debug-find", "--trace-redirect=build.cmake.trace", "--trace-expand",  ext.sourcedir] + cmake_args, cwd=build_dir, env=env)
        subprocess.check_call(["cmake", "--build", "."] + build_args, cwd=build_dir)
        subprocess.check_call(["cmake", "--install", ".", "--prefix", extdir] + install_args, cwd=build_dir)


def build(setup_kwargs: Dict[str, Any]) -> None:
    cmake_modules = [
                     CMakeExtension("Coriolis.coloquinte", sourcedir="coloquinte"),
                     CMakeExtension("Coriolis.hurricane", sourcedir="hurricane"),
                     CMakeExtension("Coriolis.crlcore", sourcedir="crlcore"),
                     CMakeExtension("Coriolis.flute", sourcedir="flute"),
                     CMakeExtension("Coriolis.etesian", sourcedir="etesian"),
                     CMakeExtension("Coriolis.anabatic", sourcedir="anabatic"),
                     CMakeExtension("Coriolis.katana", sourcedir="katana"),
                     CMakeExtension("Coriolis.equinox", sourcedir="equinox"),
                     CMakeExtension("Coriolis.solstice", sourcedir="solstice"),
                     CMakeExtension("Coriolis.oroshi", sourcedir="oroshi"),
                     CMakeExtension("Coriolis.bora", sourcedir="bora"),
                     CMakeExtension("Coriolis.karakaze", sourcedir="karakaze"),
                     #CMakeExtension("Coriolis.knik", sourcedir="knik"),
                     #CMakeExtension("Coriolis.unicorn", sourcedir="unicorn"),
                     CMakeExtension("Coriolis.tutorial", sourcedir="tutorial"),
                     CMakeExtension("Coriolis.cumulus", sourcedir="cumulus"),
                     CMakeExtension("Coriolis.stratus1", sourcedir="stratus1"),
                     CMakeExtension("Coriolis.documentation", sourcedir="documentation"),
                     CMakeExtension("Coriolis.unittests", sourcedir="unittests")
                     ]
 
    ext_modules = cmake_modules
    setup_kwargs.update(
        {
            "ext_modules": ext_modules,
            "cmdclass": dict(build_ext=ExtensionBuilder),
            "zip_safe": False,
        }
    )
