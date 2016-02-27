all: generator process scheduler

generator: gen.c
	gcc gen.c -o generator

process: process.c
	gcc process.c -o process

scheduler: sched.c
	gcc sched.c -o scheduler

clean:
	rm generator process scheduler