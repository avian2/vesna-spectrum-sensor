#!/usr/bin/python

from setuptools import setup

setup(name='vesna-spectrumsensor',
      version='1.0.0',
      description='Tools for talking the VESNA almost-like-HTTP protocol',
      license='GPL',
      long_description=open("README").read(),
      author='Tomaz Solc',
      author_email='tomaz.solc@tablix.org',

      packages = [ 'vesna', 'vesna.rftest' ],

      install_requires = [ 'pyserial' ],

      namespace_packages = [ 'vesna' ],

      scripts = [
	      'scripts/vesna_rftest',
	      'scripts/vesna_rftest_plot',
	      'scripts/vesna_log',
      ],
      test_suite = 'tests',
)
