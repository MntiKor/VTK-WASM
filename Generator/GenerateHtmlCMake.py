import argparse
import json
import shutil
import glob
import subprocess
import os
import errno

def GetParameters():
    parser = argparse.ArgumentParser(description='', epilog='')
    parser.add_argument('source_path')
    parser.add_argument('dest_path')
    parser.add_argument('vtk_source_path')
    args = parser.parse_args()
    return args.source_path, args.dest_path, args.vtk_source_path

def GenerateExample(example_name, source_path, dest_path, vtk_source_path):
    shutil.copyfile('index.html.template', os.path.join(dest_path, 'index.html'))
    with open(os.path.join(dest_path, 'index.html'), 'r') as index:
        data = index.read()
    data = data.replace('XXX', example_name)
    with open(os.path.join(dest_path, 'index.html'), 'w') as index:
        index.write(data)
    shutil.copyfile('CMakeLists.txt.template', os.path.join(dest_path, 'CMakeLists.txt'))
    with open(os.path.join(dest_path, 'CMakeLists.txt'), 'r') as cmake:
        data = cmake.read()
    data = data.replace('XXX', example_name)
    try:
        process = subprocess.run('python3 WhatModulesVTK.py ' + vtk_source_path + ' ' + source_path, shell=True, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        result = process.stdout.decode('utf-8')
    except subprocess.CalledProcessError as err:
        print('ERROR:', err)
        if err.returncode != 0:
            print('returncode:', err.returncode)
            print('Have {} bytes in stdout:\n{}'.format(
                    len(err.stdout),
                    err.stdout.decode('utf-8'))
                        )
            print('Have {} bytes in stderr:\n{}'.format(
                    len(err.stderr),
                    err.stderr.decode('utf-8'))
                        )
        result = f"# The following error occurred running {err.cmd}\n"
        for line in err.stderr.decode('utf-8').split('\n'):
            result += f"# {line}\n"
    data = data.replace('ZZZ', result)
    with open(os.path.join(dest_path, 'CMakeLists.txt'), 'w') as cmake:
        cmake.write(data)

def GenerateExampleArgs(example_name, source_path, dest_path, vtk_source_path, args_data):
    shutil.copyfile('index_arguments.html.template', os.path.join(dest_path, 'index.html'))
    with open(os.path.join(dest_path, 'index.html'), 'r') as index:
        data = index.read()
    data = data.replace('XXX', example_name)
    module_arguments = []
    for arg in args_data.get('args'):
        module_arguments.append(arg)
    data = data.replace('YYY', '\', \''.join(module_arguments))

    script_lines = []
    for file in args_data.get('files'):
        script_lines.append('<script type="text/javascript" src="https://vtk-wasm-examples.s3.fr-par.scw.cloud/data/' + file + '.js"></script>')
    data = data.replace('ZZZ', '\n'.join(script_lines))

    with open(os.path.join(dest_path, 'index.html'), 'w') as index:
        index.write(data)
    shutil.copyfile('CMakeLists_arguments.txt.template', os.path.join(dest_path, 'CMakeLists.txt'))
    with open(os.path.join(dest_path, 'CMakeLists.txt'), 'r') as cmake:
        data = cmake.read()
    data = data.replace('XXX', example_name)
    try:
        process = subprocess.run('python3 WhatModulesVTK.py ' + vtk_source_path + ' ' + source_path, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        result = process.stdout.decode('utf-8')
    except subprocess.CalledProcessError as err:
        print('ERROR:', err)
        if err.returncode != 0:
            print('returncode:', err.returncode)
            print('Have {} bytes in stdout:\n{}'.format(
                    len(err.stdout),
                    err.stdout.decode('utf-8'))
                        )
            print('Have {} bytes in stderr:\n{}'.format(
                    len(err.stderr),
                    err.stderr.decode('utf-8'))
                        )
        result = f"# The following error occurred running {err.cmd}\n"
        for line in err.stderr.decode('utf-8').split('\n'):
            result += f"# {line}\n"
    data = data.replace('ZZZ', result)

    with open(os.path.join(dest_path, 'CMakeLists.txt'), 'w') as cmake:
        cmake.write(data)


def main():
    source_path, dest_path, vtk_source_path = GetParameters()
    example_name = os.path.splitext(os.path.basename(source_path))[0]
    with open('ArgsNeeded.json') as f:
        data = json.load(f)
    if data.get(example_name, None):
        print('arguments found')
        GenerateExampleArgs(example_name, source_path, dest_path, vtk_source_path, data.get(example_name))
    else:
        print('no arguments found')
        GenerateExample(example_name, source_path, dest_path, vtk_source_path)

if __name__ == '__main__':
    main()
