DEBUG = 0
DIR = release

ifeq ($(DEBUG), 1)
	CFLAGS += -g -O0
	DIR = debug
endif

$(shell mkdir -p obj/$(DIR))
$(shell mkdir -p bin/$(DIR))

SRCS = $(wildcard src/*.c)
OBJS = $(patsubst src/%.c, obj/$(DIR)/%.o, $(SRCS))

TARGET_SERVER = bin/$(DIR)/server
TARGET_CLIENT = bin/$(DIR)/client

server: $(TARGET_SERVER)
client: $(TARGET_CLIENT)

$(TARGET_SERVER): $(OBJS)
	$(CC) -o $@ $(CFLAGS) $^ $(LDFLAGS)

$(TARGET_CLIENT): client.c
	$(CC) -o obj/$(DIR)/client.o $(CFLAGS) -c $<
	$(CC) -o $@ $(CFLAGS) obj/$(DIR)/client.o $(LDFLAGS)

obj/$(DIR)/%.o: src/%.c
	$(CC) -o $@ $(CFLAGS) -c $<

clean:
	rm -f bin/debug/* bin/release/* obj/debug/* obj/release/*

erase:
	rm -rf bin obj