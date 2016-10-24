#!/usr/bin/python

import os
from setuptools import setup

def get_long_description():
	return open(os.path.join(os.path.dirname(__file__), "README.rst")).read()

setup(name='vesna-spectrumsensor',
      version='1.0.5',
      description='Tools for talking to the VESNA spectrum sensor applicaiton',
      license='GPL',
      long_description=get_long_description(),
      author='Tomaz Solc',
      author_email='tomaz.solc@ijs.si',
      url='https://github.com/avian2/vesna-spectrum-sensor',

      packages = [ 'vesna', 'vesna.rftest', 'vesna.spectrumsensor' ],

      install_requires = [ 'pyserial' ],

      namespace_packages = [ 'vesna' ],

      scripts = [
	      'scripts/vesna_rftest',
	      'scripts/vesna_rftest_plot',
	      'scripts/vesna_log',
      ],
      test_suite = 'test',
)
