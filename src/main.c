#include <lm4f_120xl.h>
#include <os.h>

int main()
{
    LED_Init();

    for (unsigned i = 0; true; i++)
	{
		LEDR = ((i + 1) / 3) % 2;
		LEDG = ((i + 3) / 3) % 2;
		LEDB = ((i + 5) / 3) % 2;

		tsk_delay(200);
    }
}
