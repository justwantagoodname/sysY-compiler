#!/usr/bin/env python3

import logging
import os
from tqdm import tqdm
from time import sleep
import subprocess
import filecmp



logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)
testcases = []

class Helper:
    @staticmethod
    def search_testcase():
        # for root, dirs, files in os.path('testcases'):
        #     for file in files:
        #         if file.endswith('.sy'):
        #             name, _ = file.split('.')
        #             abs_path = os.path.abspath(root)
        #             testcases.append(TestCase(name, abs_path))
        for f in os.scandir('testcases'):
            if not f.is_dir():
                exit(-1)
            
            name = 'long_code2'
            test_path = os.path.join('testcases', name)
            testcase_num = len([g for g in os.scandir(test_path) if (g.is_file() and g.name.endswith('.out'))])
            testcases.append(TestCase(name, test_path, testcase_num))


    @staticmethod
    def build_compiler():
        logger.info('Building compiler')
        build = subprocess.Popen(['make', 'dev-compiler', '-j12'], cwd='..')
        ret = build.wait()
        if ret != 0:
            exit(ret)

class TestCase:
    def __init__(self, name, abs_path, testcase_num):
        self.name = name
        self.path = abs_path
        self.source = os.path.join(self.path, f'{name}.sy')
        # self.input = os.path.join(self.path, f'{name}.in')
        # self.output = os.path.join(self.path, f'{name}.out')
        self.testcase_num = testcase_num
        self.with_input = True if len([g for g in os.scandir(self.path) if (g.is_file() and g.name.endswith('.in'))]) != 0 else False
        self.check_valid() 
    
    def check_valid(self):
        if not os.path.exists(self.source):
            raise FileNotFoundError(f'{self.source} not found')
        if self.testcase_num == 0:
            # print('no testcase found in', self.name)
            # exit(-1)
            pass
        
        in_num = len([g for g in os.scandir(self.path) if (g.is_file() and g.name.endswith('.out'))])
        if in_num != 0 and in_num != self.testcase_num:
            print('invalid input files')
            exit(-1)
            # self.input = 'stdin'
        # if not os.path.exists(self.output):
        #    raise FileNotFoundError(f'{self.output} not found')

class Runner:
    def run(self, testcase):
        logger.info(f'Running testcase: {testcase.name}')

        with open('test-run/compile_log', 'w') as f:
            compile = subprocess.Popen(['compiler', testcase.source, '-o', '../test/output.s'], executable='../build-dev/compiler', stdout=f)
            ret = compile.wait()
            if ret != 0:
                print('compile failed.')
                exit(ret)

        for i in range(testcase.testcase_num):
            out_file = os.path.join(testcase.path, str(i) + '.out')
            if testcase.with_input != 0:
                in_file = os.path.join(testcase.path, str(i) + '.in')
                
            else:
                in_file = '/dev/null'
            
            arm_simulation = subprocess.Popen(['bash', 'run_wrapper.sh', 'arm', in_file, out_file], cwd='.')
            ret = arm_simulation.wait()

            if not filecmp.cmp(out_file, 'output.arm.out', shallow=False):
                print(f'error on {i}.')
                exit(ret)
            else:
                print(f'ok on input {i}.')


if __name__ == '__main__':

    Helper.search_testcase()
    Helper.build_compiler()

    testcases.sort(key=lambda x: x.name)

    logger.info(f'Found {len(testcases)} testcases')

    runner = Runner()

    # for testcase in testcases:
    #     runner.run(testcase)
    runner.run(testcases[0])