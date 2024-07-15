#!/usr/bin/env python3

import logging
import os
from tqdm import tqdm
from time import sleep
import subprocess

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)
testcases = []

class Helper:
    @staticmethod
    def search_testcase():
        for root, dirs, files in os.walk('testcases'):
            for file in files:
                if file.endswith('.sy'):
                    name, _ = file.split('.')
                    abs_path = os.path.abspath(root)
                    testcases.append(TestCase(name, abs_path))

    @staticmethod
    def build_compiler():
        logger.info('Building compiler')
        build = subprocess.Popen(['make', 'release-compiler'], cwd='..')
        ret = build.wait()
        if ret != 0:
            exit(ret)

class TestCase:
    def __init__(self, name, abs_path):
        self.name = name
        self.path = abs_path
        self.source = os.path.join(self.path, f'{name}.sy')
        self.input = os.path.join(self.path, f'{name}.in')
        self.output = os.path.join(self.path, f'{name}.out')
        self.check_valid() 
    
    def check_valid(self):
        if not os.path.exists(self.source):
            raise FileNotFoundError(f'{self.source} not found')
        # if not os.path.exists(f'{self.path}/{self.input}'):
        #     raise FileNotFoundError(f'{self.input} not found')
        # if not os.path.exists(f'{self.path}/{self.output}'):
        #    raise FileNotFoundError(f'{self.output} not found')

class Runner:

    def run(self, testcase):
        logger.info(f'Running testcase: {testcase.name}')

        compile = subprocess.Popen(['compiler', testcase.source, '-o', '../test/output.s'], executable='../build/compiler')
        ret = compile.wait()
        if ret != 0:
            exit(ret)

        arm_simulation = subprocess.Popen(['make', 'run-arm'], cwd='..')
        ret = arm_simulation.wait()

        if ret != 0:
            exit(ret)


if __name__ == '__main__':

    Helper.search_testcase()
    Helper.build_compiler()

    testcases.sort(key=lambda x: x.name)

    logger.info(f'Found {len(testcases)} testcases')

    runner = Runner()

    for testcase in tqdm(testcases):
        runner.run(testcase)