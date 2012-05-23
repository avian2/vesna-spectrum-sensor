#include <stdio.h>
#include <stdlib.h>
#include <libopencm3/stm32/f1/gpio.h>
#include <libopencm3/stm32/f1/rcc.h>
#include <libopencm3/stm32/f1/rtc.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/systick.h>

#include "dev-cc.h"
#include "spectrum.h"

static void setup_stm32f1_peripherals(void)
{
	rcc_peripheral_enable_clock(&RCC_APB1ENR,
			RCC_APB1ENR_SPI2EN);

	gpio_set_mode(CC_GPIO, GPIO_MODE_OUTPUT_10_MHZ, 
			GPIO_CNF_OUTPUT_PUSHPULL, CC_PIN_NSS);
	gpio_set_mode(CC_GPIO, GPIO_MODE_OUTPUT_10_MHZ, 
			GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, CC_PIN_SCK);
	gpio_set_mode(CC_GPIO, GPIO_MODE_OUTPUT_10_MHZ, 
			GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, CC_PIN_MISO);
	gpio_set_mode(CC_GPIO, GPIO_MODE_OUTPUT_10_MHZ, 
			GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, CC_PIN_MOSI);


	spi_init_master(SPI2,
			SPI_CR1_BAUDRATE_FPCLK_DIV_4, 
			SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE, 
			SPI_CR1_CPHA_CLK_TRANSITION_1,
			SPI_CR1_DFF_8BIT,
			SPI_CR1_MSBFIRST);
	spi_set_unidirectional_mode(SPI2);
	spi_set_full_duplex_mode(SPI2);
	spi_enable_software_slave_management(SPI2);
	spi_set_nss_high(SPI2);

	spi_enable(SPI2);


	systick_set_reload(0x00ffffff);
	systick_set_clocksource(STK_CTRL_CLKSOURCE_AHB_DIV8);
	systick_counter_enable();
}

static void cc_wait_while_miso_high(void)
{
	while(gpio_get(CC_GPIO, CC_PIN_MISO));
}

static const uint32_t systick_udelay_calibration = 6;

static void systick_udelay(uint32_t usecs)
{
	uint32_t val = (STK_VAL - systick_udelay_calibration * usecs) 
		& 0x00ffffff;
	while(!((STK_VAL - val) & 0x00800000));
}

static void cc_reset() 
{
	gpio_clear(CC_GPIO, CC_PIN_NSS);

	gpio_set(CC_GPIO, CC_PIN_SCK);
	gpio_clear(CC_GPIO, CC_PIN_MOSI);

	gpio_set(CC_GPIO, CC_PIN_NSS);
	systick_udelay(10);
	gpio_clear(CC_GPIO, CC_PIN_NSS);
	systick_udelay(10);
	gpio_set(CC_GPIO, CC_PIN_NSS);
	systick_udelay(200);
	gpio_clear(CC_GPIO, CC_PIN_NSS);

	cc_wait_while_miso_high();

	systick_udelay(300);

	spi_send(SPI2, CC_STROBE_SRES);

	cc_wait_while_miso_high();

	systick_udelay(300);

	gpio_set(CC_GPIO, CC_PIN_NSS);
}

uint16_t cc_read_reg(uint8_t reg)
{
	gpio_clear(CC_GPIO, CC_PIN_NSS);
	cc_wait_while_miso_high();

	spi_send(SPI2, reg|0x80);
	spi_read(SPI2);

	spi_send(SPI2, 0);
	uint16_t value = spi_read(SPI2);

	gpio_set(CC_GPIO,CC_PIN_NSS);

	return value;
}

void cc_write_reg(uint8_t reg, uint8_t value) 
{
	gpio_clear(CC_GPIO,CC_PIN_NSS);
	cc_wait_while_miso_high();

	spi_send(SPI2, reg);
	spi_read(SPI2);

	spi_send(SPI2, value);
	spi_read(SPI2);

	gpio_set(CC_GPIO,CC_PIN_NSS);
}

uint16_t cc_strobe(uint8_t strobe) 
{
	gpio_clear(CC_GPIO, CC_PIN_NSS);
	cc_wait_while_miso_high();

	spi_send(SPI2, strobe);
	uint16_t value = spi_read(SPI2);

	gpio_set(CC_GPIO, CC_PIN_NSS);

	return value;
}

void cc_wait_state(uint8_t state)
{
	while(cc_read_reg(CC_REG_MARCSTATE) != state);
}

int dev_cc_reset(void* priv) 
{
	setup_stm32f1_peripherals();
	cc_reset();
	return E_SPECTRUM_OK;
}

int dev_cc_setup(void* priv, const struct spectrum_sweep_config* sweep_config) 
{
	uint8_t *init_seq = (uint8_t*) sweep_config->dev_config->priv;

	cc_strobe(CC_STROBE_SIDLE);
	cc_wait_state(CC_MARCSTATE_IDLE);

	int n;
	for(n = 0; init_seq[n] != 0xff; n += 2) {
		uint8_t reg = init_seq[n];
		uint8_t value = init_seq[n+1];
		cc_write_reg(reg, value);
	}

	return E_SPECTRUM_OK;
}

int dev_cc_run(void* priv, const struct spectrum_sweep_config* sweep_config)
{
	int r;
	short int *data;

	rtc_set_counter_val(0);

	int channel_num = spectrum_sweep_channel_num(sweep_config);
	data = calloc(channel_num, sizeof(*data));
	if (data == NULL) {
		return E_SPECTRUM_TOOMANY;
	}

	do {
		uint32_t rtc_counter = rtc_get_counter_val();
		/* LSE clock is 32768 Hz. Prescaler is set to 16.
		 *
		 *                 rtc_counter * 16
		 * t [ms] = 1000 * ----------------
		 *                       32768
		 */
		int timestamp = ((long long) rtc_counter) * 1000 / 2048;

		int n, ch;
		for(		ch = sweep_config->channel_start, n = 0; 
				ch < sweep_config->channel_stop && n < channel_num; 
				ch += sweep_config->channel_step, n++) {

			cc_strobe(CC_STROBE_SIDLE);
			cc_wait_state(CC_MARCSTATE_IDLE);

			cc_write_reg(CC_REG_CHANNR, ch);

			cc_strobe(CC_STROBE_SRX);
			cc_wait_state(CC_MARCSTATE_RX);

			int8_t reg = cc_read_reg(CC_REG_RSSI);

			int rssi_dbm_100 = -5920 + ((int) reg) * 50;
			data[n] = rssi_dbm_100;
		}
		r = sweep_config->cb(sweep_config, timestamp, data);
	} while(!r);

	free(data);

	if (r == E_SPECTRUM_STOP_SWEEP) {
		return E_SPECTRUM_OK;
	} else {
		return r;
	}
}

static void dev_cc_print_status(void)
{
	printf("Part number : %02x\n", cc_read_reg(CC_REG_PARTNUM));
	printf("Version     : %02x\n", cc_read_reg(CC_REG_VERSION));
}

void dev_cc1101_print_status(void)
{
	printf("IC          : CC1101\n\n");
	dev_cc_print_status();
}

uint8_t dev_cc1101_868mhz_400khz_init_seq[] = {
	CC_REG_IOCFG2,             0x2E,
	CC_REG_IOCFG1,             0x2E,
	CC_REG_IOCFG0,             0x2E,
	CC_REG_FIFOTHR,            0x07,
	CC_REG_SYNC1,              0xD3,
	CC_REG_SYNC0,              0x91,
	CC_REG_PKTLEN,             0xFF,
	CC_REG_PKTCTRL1,           0x04,
	CC_REG_PKTCTRL0,           0x12,
	CC_REG_ADDR,               0x00,
	CC_REG_CHANNR,             0x00,
	CC_REG_FSCTRL1,            0x06,
	CC_REG_FSCTRL0,            0x00,
	CC_REG_FREQ2,              0x21,
	CC_REG_FREQ1,              0x65,
	CC_REG_FREQ0,              0x6A,
	CC_REG_MDMCFG4,            0x49,
	CC_REG_MDMCFG3,            0x93,
	CC_REG_MDMCFG2,            0x70,
	CC_REG_MDMCFG1,            0x22,
	CC_REG_MDMCFG0,            0xF8,
	CC_REG_DEVIATN,            0x34,
	CC_REG_MCSM2,              0x07,
	CC_REG_MCSM1,              0x30,
	CC_REG_MCSM0,              0x18,
	CC_REG_FOCCFG,             0x16,
	CC_REG_BSCFG,              0x6C,
	CC_REG_AGCCTRL2,           0x43,
	CC_REG_AGCCTRL1,           0x40,
	CC_REG_AGCCTRL0,           0x91,
	CC_REG_WOREVT1,            0x87,
	CC_REG_WOREVT0,            0x6B,
	CC_REG_WORCTRL,            0xFB,
	CC_REG_FREND1,             0x56,
	CC_REG_FREND0,             0x10,
	CC_REG_FSCAL3,             0xE9,
	CC_REG_FSCAL2,             0x2A,
	CC_REG_FSCAL1,             0x00,
	CC_REG_FSCAL0,             0x1F,
	CC_REG_RCCTRL1,            0x41,
	CC_REG_RCCTRL0,            0x00,
	CC_REG_FSTEST,             0x59,
	CC_REG_PTEST,              0x7F,
	CC_REG_AGCTEST,            0x3F,
	CC_REG_TEST2,              0x81,
	CC_REG_TEST1,              0x35,
	CC_REG_TEST0,              0x09,
	CC_REG_PARTNUM,            0x00,
	CC_REG_VERSION,            0x04,
	CC_REG_FREQEST,            0x00,
	CC_REG_LQI,                0x00,
	CC_REG_RSSI,               0x00,
	CC_REG_MARCSTATE,          0x00,
	CC_REG_WORTIME1,           0x00,
	CC_REG_WORTIME0,           0x00,
	CC_REG_PKTSTATUS,          0x00,
	CC_REG_VCO_VC_DAC,         0x00,
	CC_REG_TXBYTES,            0x00,
	CC_REG_RXBYTES,            0x00,
	CC_REG_RCCTRL1_STATUS,     0x00,
	CC_REG_RCCTRL0_STATUS,     0x00,
	0xFF,			   0xFF
};

uint8_t dev_cc1101_868mhz_800khz_init_seq[] = {
	CC_REG_IOCFG2,         0x2E,
	CC_REG_IOCFG1,         0x2E,
	CC_REG_IOCFG0,         0x2E,
	CC_REG_FIFOTHR,        0x07,
	CC_REG_SYNC1,          0xD3,
	CC_REG_SYNC0,          0x91,
	CC_REG_PKTLEN,         0xFF,
	CC_REG_PKTCTRL1,       0x04,
	CC_REG_PKTCTRL0,       0x12,
	CC_REG_ADDR,           0x00,
	CC_REG_CHANNR,         0x00,
	CC_REG_FSCTRL1,        0x06,
	CC_REG_FSCTRL0,        0x00,
	CC_REG_FREQ2,          0x20,
	CC_REG_FREQ1,          0x26,
	CC_REG_FREQ0,          0x98,
	CC_REG_MDMCFG4,        0x09,
	CC_REG_MDMCFG3,        0x84,
	CC_REG_MDMCFG2,        0x70,
	CC_REG_MDMCFG1,        0x22,
	CC_REG_MDMCFG0,        0xE5,
	CC_REG_DEVIATN,        0x67,
	CC_REG_MCSM2,          0x07,
	CC_REG_MCSM1,          0x30,
	CC_REG_MCSM0,          0x18,
	CC_REG_FOCCFG,         0x16,
	CC_REG_BSCFG,          0x6C,
	CC_REG_AGCCTRL2,       0x43,
	CC_REG_AGCCTRL1,       0x40,
	CC_REG_AGCCTRL0,       0x91,
	CC_REG_WOREVT1,        0x87,
	CC_REG_WOREVT0,        0x6B,
	CC_REG_WORCTRL,        0xFB,
	CC_REG_FREND1,         0x56,
	CC_REG_FREND0,         0x10,
	CC_REG_FSCAL3,         0xE9,
	CC_REG_FSCAL2,         0x2A,
	CC_REG_FSCAL1,         0x00,
	CC_REG_FSCAL0,         0x1F,
	CC_REG_RCCTRL1,        0x41,
	CC_REG_RCCTRL0,        0x00,
	CC_REG_FSTEST,         0x59,
	CC_REG_PTEST,          0x7F,
	CC_REG_AGCTEST,        0x3F,
	CC_REG_TEST2,          0x88,
	CC_REG_TEST1,          0x31,
	CC_REG_TEST0,          0x09,
	CC_REG_PARTNUM,        0x00,
	CC_REG_VERSION,        0x04,
	CC_REG_FREQEST,        0x00,
	CC_REG_LQI,            0x00,
	CC_REG_RSSI,           0x00,
	CC_REG_MARCSTATE,      0x00,
	CC_REG_WORTIME1,       0x00,
	CC_REG_WORTIME0,       0x00,
	CC_REG_PKTSTATUS,      0x00,
	CC_REG_VCO_VC_DAC,     0x00,
	CC_REG_TXBYTES,        0x00,
	CC_REG_RXBYTES,        0x00,
	CC_REG_RCCTRL1_STATUS, 0x00,
	CC_REG_RCCTRL0_STATUS, 0x00,
	0xFF,		       0xFF
};

const struct spectrum_dev_config dev_cc1101_868mhz_400khz = {
	.name			= "868 MHz ISM, 400 kHz bandwidth",

	.channel_base_hz 	= 868299866,
	.channel_spacing_hz	= 199951,
	.channel_bw_hz		= 400000,
	.channel_num		= 5,

	.channel_time_ms	= 0,

	.priv			= dev_cc1101_868mhz_400khz_init_seq
};

const struct spectrum_dev_config dev_cc1101_868mhz_800khz = {
	.name			= "868 MHz ISM, 800 kHz bandwidth",

	.channel_base_hz 	= 867999729,
	.channel_spacing_hz	= 199797,
	.channel_bw_hz		= 800000,
	.channel_num		= 3,

	.channel_time_ms	= 0,

	.priv			= dev_cc1101_868mhz_800khz_init_seq
};

const struct spectrum_dev_config* dev_cc1101_config_list[] = {
	&dev_cc1101_868mhz_400khz, 
	&dev_cc1101_868mhz_800khz };

const struct spectrum_dev dev_cc1101 = {
	.name = "cc1101",

	.dev_config_list	= dev_cc1101_config_list,
	.dev_config_num		= 2,

	.dev_reset		= dev_cc_reset,
	.dev_setup		= dev_cc_setup,
	.dev_run		= dev_cc_run,

	.priv 			= NULL
};

int dev_cc_register(void)
{
	return spectrum_add_dev(&dev_cc1101);
}