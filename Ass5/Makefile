all: manager producer consumer

manager: Manager.c
	gcc Manager.c -o manager

producer: producer.c
	gcc producer.c -o producer

consumer: consumer.c
	gcc consumer.c -o consumer

clean:
	rm manager producer consumer