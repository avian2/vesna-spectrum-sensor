.. vim:sw=3 ts=3 expandtab tw=78

Introduction
============

This Python module abstracts the serial line protocol used by the VESNA
spectrum sensor application and provides a high-level object-oriented
Python interface.


Installation
============

To install system-wide, run::

   $ python setup.py install

To run provided unit tests, run::

   $ python setup.py test


Usage
=====

The minimal application using this module looks like the following::

   # instantiate the SpectrumSensor class using the path
   # to the serial device.
   spectrumsensor = SpectrumSensor("/dev/ttyUSB0")

   # query the attached hardware for supported configurations.
   config_list = spectrumsensor.get_config_list()

   # get required frequency sweep configuration.
   sweep_config = config_list.get_sweep_config(...)

   # define callback function that does something with
   # measurements.
   def callback(sweep_config, sweep):
      ...

   # start spectrum sensing
   spectrumsensor.run(sweep_config, callback)

Please refer to docstring documentation for details.

The package also installs vesna_rftest script that performs a series of
automated hardware tests using a USBTMC attached RF signal generator. Run
"vesna_rftest --help" to get a list of available options.


License
=======

Copyright (C) 2015 SensorLab, Jozef Stefan Institute
http://sensorlab.ijs.si

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

Authors:	Tomaz Solc, <tomaz.solc@ijs.si>
