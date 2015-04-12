
MAN=

.include <bsd.own.mk>

PROG=beastie_ssd1351_128x128
SRCS=main.c
CFLAGS+=-I../../lib/liblcd
LDFLAGS+=-L../../lib/liblcd
LDADD+= -llcd -lgpio

.include <bsd.prog.mk>
