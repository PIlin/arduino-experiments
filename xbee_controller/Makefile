BUILD :=ino build -m
INST :=ino upload -m
UNO :=uno
DUE :=atmega328


all:



# uno

bu:
	$(BUILD) $(UNO)

iu:
	$(INST) $(UNO)

biu: bu iu


# due

bd:
	$(BUILD) $(DUE)

id:
	$(INST) $(DUE)

bid: bd id


#



.PHONY: all bu iu biu bd id bid clean