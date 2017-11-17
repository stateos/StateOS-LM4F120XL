#include <lm4f_120xl.h>
#include <os.h>

int main()
{
	unsigned i;

    LED_Init();

    for (i = 0; true; i++)
	{
		LEDR = ((i + 1) / 3) % 2;
		LEDG = ((i + 3) / 3) % 2;
		LEDB = ((i + 5) / 3) % 2;

		tsk_delay(200);
    }
}
