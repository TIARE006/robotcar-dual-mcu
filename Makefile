.PHONY: all f407 h750 clean

all: f407 h750

f407:
	$(MAKE) -C firmware/f407_chassis

h750:
	$(MAKE) -C firmware/h750_wifi_bridge

clean:
	$(MAKE) -C firmware/f407_chassis clean
	$(MAKE) -C firmware/h750_wifi_bridge clean
