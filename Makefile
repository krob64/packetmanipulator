CFLAGS = -O2
LDFLAGS = -ldiscord -lcurl -pthread

packetmanipulator: src/main.c
	gcc $(CFLAGS) -o packetmanipulator src/main.c $(LDFLAGS)
