# PyInstaller spec (WSword's PC build, from LAS Recomp's lasgen.spec, MIT): the game code generator (tools/gen.py and
# its modules) as a standalone onedir helper, so the start screen's setup can build the game code on the player's PC
# without Python (runtime/gamebuild.c). `make gen-helper` -> build/helper/botwgen/ (botwgen.exe + _internal/). Same
# command line as gen.py.
import glob
import importlib.util
import os
import sys

from PyInstaller.utils.hooks import collect_dynamic_libs, collect_submodules

TOOLS = os.path.normpath(SPECPATH)

# collect_dynamic_libs only picks unversioned names: unicorn's library is versioned (unicorn.dll on Windows), so add
# it by hand where the package's loader looks for it (importlib.resources files('unicorn')/'lib')
_uc_lib_dir = os.path.join(os.path.dirname(importlib.util.find_spec('unicorn').origin), 'lib')
_uc_versioned = (glob.glob(os.path.join(_uc_lib_dir, 'libunicorn.so*'))
                 + glob.glob(os.path.join(_uc_lib_dir, 'libunicorn.*.dylib'))
                 + glob.glob(os.path.join(_uc_lib_dir, 'unicorn.dll')))

a = Analysis(
    [os.path.join(TOOLS, 'gen.py')],
    pathex=[TOOLS],
    binaries=collect_dynamic_libs('capstone') + collect_dynamic_libs('unicorn')
             + [(p, 'unicorn/lib') for p in _uc_versioned],
    datas=[],
    hiddenimports=collect_submodules('unicorn') + ['patches', 'passes', 'simd_c', 'analyze', 'lifter', 'nso'],
    excludes=['tkinter', 'pydoc', 'doctest', 'unittest', 'pdb', 'IPython', 'numpy', 'setuptools', '_distutils_hack',
              'pkg_resources', 'ssl', '_ssl', '_hashlib', '_multibytecodec', '_codecs_cn', '_codecs_hk',
              '_codecs_iso2022', '_codecs_jp', '_codecs_kr', '_codecs_tw', '_decimal'],
    noarchive=False,
)
pyz = PYZ(a.pure)
exe = EXE(
    pyz,
    a.scripts,
    [('hash_seed=0', None, 'OPTION')],  # (the same output every run)
    exclude_binaries=True,
    name='botwgen',
    console=True,  # stderr = the progress lines the runtime reads (no window: CREATE_NO_WINDOW)
    strip=False,
    upx=False,
)
coll = COLLECT(exe, a.binaries, a.datas, strip=False, upx=False, name='botwgen')
