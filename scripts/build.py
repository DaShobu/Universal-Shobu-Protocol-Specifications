"""Build native examples with installed compilers; no downloads or packages."""
import argparse
import os
import shutil
import subprocess
import sys
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
BUILD=ROOT/'build'


def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('--zig',help='Path to a Zig 0.15.2 executable')
    parser.add_argument('--require-all',action='store_true')
    args=parser.parse_args()
    BUILD.mkdir(exist_ok=True)
    compiler_tmp=BUILD/'compiler-tmp'
    compiler_tmp.mkdir(exist_ok=True)
    # Keep compiler temporary files alongside the ignored build outputs.
    for variable in ('TMP','TEMP','TMPDIR'):
        os.environ[variable]=str(compiler_tmp)
    suffix='.exe' if sys.platform=='win32' else ''
    zig=args.zig or shutil.which('zig')
    if not zig:
        candidates=list((BUILD/'zig-toolchain').glob('*/zig.exe'))
        if candidates: zig=str(candidates[0])
    configs=[
        ('c',shutil.which('gcc'),['-std=c99','-O2','-Wall','-Wextra','-Werror','examples/c/main.c','examples/c/shobu.c','-o',str(BUILD/f'shobu-c{suffix}')]),
        ('cpp',shutil.which('g++'),['-std=c++17','-O2','-Wall','-Wextra','-Werror','examples/cpp/main.cpp','-o',str(BUILD/f'shobu-cpp{suffix}')]),
        ('rust',shutil.which('rustc'),['--edition=2021','-D','warnings','examples/rust/engine.rs','-o',str(BUILD/f'shobu-rust{suffix}')]),
        ('zig',zig,['build-exe','examples/zig/main.zig','examples/c/shobu.c','-I','examples/c','-lc','-O','ReleaseSafe',f'-femit-bin={BUILD/f"shobu-zig{suffix}"}',
                    '--cache-dir',str(BUILD/'zig-local-cache'),'--global-cache-dir',str(BUILD/'zig-global-cache')]),
    ]
    missing=[name for name,compiler,_ in configs if not compiler]
    if missing and args.require_all: parser.error('missing compilers: '+', '.join(missing))
    for name,compiler,options in configs:
        if not compiler:
            print('SKIP '+name+': compiler unavailable');continue
        if name=='zig':
            version=subprocess.check_output([compiler,'version'],text=True).strip()
            if version!='0.15.2': parser.error('Zig 0.15.2 required; found '+version)
        subprocess.run([compiler,*options],cwd=ROOT,check=True)
        print('BUILT '+name,flush=True)
        if name=='c':
            transport_test=BUILD/f'c-transport-test{suffix}'
            subprocess.run([compiler,'-std=c99','-O2','-Wall','-Wextra','-Werror','tests/c_transport.c','-o',str(transport_test)],cwd=ROOT,check=True)
            subprocess.run([str(transport_test)],cwd=ROOT,check=True)
        if name=='cpp':
            api_test=BUILD/f'cpp-api-test{suffix}'
            subprocess.run([compiler,'-std=c++17','-O2','-Wall','-Wextra','-Werror','tests/cpp_api.cpp','-o',str(api_test)],cwd=ROOT,check=True)
            subprocess.run([str(api_test)],cwd=ROOT,check=True)
    if zig:
        subprocess.run([zig,'test','examples/zig/main.zig','examples/c/shobu.c','-I','examples/c','-lc',
                        '--cache-dir',str(BUILD/'zig-local-cache'),'--global-cache-dir',str(BUILD/'zig-global-cache')],cwd=ROOT,check=True)


if __name__=='__main__': main()
