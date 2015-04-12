#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <err.h>

#include "lcd.h"

struct lcd *
lcd_create(void)
{
	struct lcd *l;

	l = calloc(1, sizeof(*l));
	if (l == NULL) {
		warn("%s: calloc", __func__);
		return (NULL);
	}
	return (l);
}

void
lcd_teardown(struct lcd *l)
{

	/* XXX tell HAL layer */
	free(l);
}
