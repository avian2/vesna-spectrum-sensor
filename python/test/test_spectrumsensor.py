import unittest
from textwrap import dedent

from vesna.spectrumsensor import Device, DeviceConfig, SweepConfig, DeviceConfig, ConfigList

class TestDeviceConfig(unittest.TestCase):
	def setUp(self):
		self.d = Device(0, "test")

		self.dc = DeviceConfig(0, "test", self.d)
		self.dc.base = 1000
		self.dc.spacing = 30
		self.dc.num = 1000

	def test_get_full_sweep_config_1(self):
		sc = self.dc.get_full_sweep_config()
		self.assertEquals(sc.step_ch, 1)

	def test_get_full_sweep_config_2(self):
		sc = self.dc.get_full_sweep_config(step_hz=5)
		self.assertEquals(sc.step_ch, 1)

	def test_get_full_sweep_config_3(self):
		sc = self.dc.get_full_sweep_config(step_hz=35)
		self.assertEquals(sc.step_ch, 1)

	def test_get_full_sweep_config_3(self):
		sc = self.dc.get_full_sweep_config(step_hz=45)
		self.assertEquals(sc.step_ch, 2)

class TestSweepConfig(unittest.TestCase):
	def setUp(self):
		self.d = Device(0, "test")

		self.dc = DeviceConfig(0, "test", self.d)
		self.dc.base = 1000
		self.dc.spacing = 1
		self.dc.num = 1000

	def test_stop_hz_1(self):
		# 0, 1, 2, 3, 4, 5, 6, 7, 8, 9
		sc = SweepConfig(self.dc, start_ch=0, stop_ch=10, step_ch=1)

		self.assertEquals(sc.stop_ch, 10)
		self.assertEquals(sc.stop_hz, 1009)

	def test_stop_hz_2(self):
		# 0, 13, 26, 39, 52
		sc = SweepConfig(self.dc, start_ch=0, stop_ch=50, step_ch=13)

		self.assertEquals(sc.stop_ch, 50)
		self.assertEquals(sc.stop_hz, 1039)

class TestConfigList(unittest.TestCase):
	def setUp(self):
		self.cl = ConfigList()

		d = Device(0, "test device")
		self.cl._add_device(d)

		def add_dc(id, name, base):
			dc = DeviceConfig(id, name, d)
			dc.base = base
			dc.spacing = 1
			dc.bw = 1
			dc.num = 1000
			dc.time = 1
			self.cl._add_config(dc)

		add_dc(0, "foo 1", 1000)
		add_dc(1, "foo 2", 2000)
		add_dc(2, "bar 1", 1000)
		add_dc(3, "bar 2", 2000)

	def test_get_config_name(self):

		cl = self.cl

		sc = cl.get_sweep_config(1500, 1600, 1)
		self.assertEquals(0, sc.config.id)

		sc = cl.get_sweep_config(2500, 2600, 1)
		self.assertEquals(1, sc.config.id)

		sc = cl.get_sweep_config(1500, 1600, 1, name="bar")
		self.assertEquals(2, sc.config.id)

		sc = cl.get_sweep_config(2500, 2600, 1, name="bar")
		self.assertEquals(3, sc.config.id)

	def test_str_empty(self):
		cl = ConfigList()

		self.assertEquals('', str(cl))

	def test_str(self):
		self.assertEquals(dedent('''\
			device 0: test device
			  channel config 0,0: foo 1
			    base: 1000 Hz
			    spacing: 1 Hz
			    bw: 1 Hz
			    num: 1000
			    time: 1 ms
			  channel config 0,1: foo 2
			    base: 2000 Hz
			    spacing: 1 Hz
			    bw: 1 Hz
			    num: 1000
			    time: 1 ms
			  channel config 0,2: bar 1
			    base: 1000 Hz
			    spacing: 1 Hz
			    bw: 1 Hz
			    num: 1000
			    time: 1 ms
			  channel config 0,3: bar 2
			    base: 2000 Hz
			    spacing: 1 Hz
			    bw: 1 Hz
			    num: 1000
			    time: 1 ms'''
		), str(self.cl))

from vesna.spectrumsensor import SpectrumSensor, SpectrumSensorBase

class TestSpectrumSensor(unittest.TestCase):

	def test_unser_base64(self):
		f = "TS 16.296 CH 660000 SC 1 BS gBgBgCgBgA BE".split()
		s = SpectrumSensorBase()
		td = s._unser_base64(f)

		self.assertEquals(td.timestamp, 16.296)
		self.assertEquals(td.channel, 660000)
		self.assertEquals(td.data, [2049.0, 2049.0, 2050.0, 2049.0, 2048.0])

	def test_unser_dec(self):
		f = "TS 0.049 CH 660000 SC 100 DS -9782 -9805 -9759 -10016 -9660 DE".split()
		td = SpectrumSensor._unser_dec(f)

		self.assertEquals(td.timestamp, 0.049)
		self.assertEquals(td.channel, 660000)
		self.assertEquals(td.data, [-97.82, -98.05, -97.59, -100.16, -96.60])

	def test_unser_old(self):
		f = "TS 0.049 CH 660000 DS -97.82 -98.05 -97.59 -100.16 -96.60 DE".split()
		td = SpectrumSensor._unser_old(f)

		self.assertEquals(td.timestamp, 0.049)
		self.assertEquals(td.channel, 660000)
		self.assertEquals(td.data, [-97.82, -98.05, -97.59, -100.16, -96.60])

from vesna.rftest import parse_test_kwargs

class TestRFTest(unittest.TestCase):
	def test_parse_test_kwargs_none(self):
		self.assertEquals(parse_test_kwargs(None), {})

	def test_parse_test_kwargs_int(self):
		self.assertEquals(parse_test_kwargs("a=1"), {'a': 1})

	def test_parse_test_kwargs_two_ints(self):
		self.assertEquals(parse_test_kwargs("a=1,b=2"), {'a': 1, 'b': 2})

	def test_parse_test_kwargs_list(self):
		self.assertEquals(parse_test_kwargs("a=[1,2],b=2"), {'a': [1,2], 'b': 2})
