
void kbd_init(volatile unsigned char *kbdport);
int kbd_hit(volatile unsigned char *kbdport);
void kbd_readkey(volatile unsigned char *kbdport,
					unsigned char *row, unsigned char *col);
