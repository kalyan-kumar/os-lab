all: train manager Manager producer consumer

train: train.c
	gcc train.c -o train

manager: manager.c
	gcc manager.c -o manager

Manager: Manager.c
	gcc Manager.c -o Manager

producer: producer.c
	gcc producer.c -o producer

consumer: consumer.c
	gcc consumer.c -o consumer

clean:
	rm manager train Manager producer consumer