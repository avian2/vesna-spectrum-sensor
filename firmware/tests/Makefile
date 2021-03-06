UNITY ?= ../../../Unity

INCLUDES += -I$(UNITY)/src -I.. -I../platform

CFLAGS += -g -Wall $(INCLUDES) -lm

UNITY_OBJ = $(UNITY)/src/unity.o

TESTS =	test_buffer_run \
	test_device_run \
	test_task_run \
	test_calibration_run \
	test_average_run \
	test_eeprom_run \
	test_base64_run

DEPS =	$(TESTS:test_%_run=%.d) \
	$(TESTS:test_%_run=test_%.d) \
	$(TESTS:test_%_run=test_%_runner.d)

all: test

test: $(TESTS)

test_task_runner: test_task_runner.o test_task.o task.o buffer.o device.o $(UNITY_OBJ)

test_eeprom_runner: test_eeprom_runner.o test_eeprom.o eeprom.o $(UNITY_OBJ)

test_%_runner: test_%_runner.o test_%.o %.o $(UNITY_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.d: %.c
	@set -e; rm -f $@; \
	$(CC) -M $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

%_run: %_runner
	./$^

%_runner.c: %.c
	ruby $(UNITY)/auto/generate_test_runner.rb $^ $@

clean:
	rm -f *.o *.d
	rm -f *_runner *_runner.c

include $(DEPS)

vpath %.c ..

.PHONY: clean test
