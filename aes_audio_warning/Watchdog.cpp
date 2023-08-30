
#include <ESP.h>
#include <Ticker.h>
#include "Watchdog.h"

static volatile uint32_t TimerTickCnt = 0;
static uint32_t SoftWdg_Timeout = TIMMER_CNT_150S;

static inline void IRAM_ATTR onTimer1ISR()
{
	TimerTickCnt++;
	if (TimerTickCnt >= SoftWdg_Timeout)
	{
		Serial.printf("Reset by software watchdog: %dms", SoftWdg_Timeout);
		ESP.reset();
	}
}
// Software watchdog timer
void SoftWdg_Enable(uint32_t timeout)
{
	SoftWdg_Timeout = timeout;
	timer1_attachInterrupt(onTimer1ISR);
	timer1_enable(TIM_DIV16, TIM_EDGE, TIM_LOOP);
	timer1_write(TIMMER_TICK_1000MS);
}
void SoftWdg_Feed()
{
	TimerTickCnt = 0;
}
void SoftWdg_Disable()
{
	timer1_detachInterrupt();
	timer1_disable();
}

// Hardware watchdog timer
void Wdg_Enable(uint32_t WdtTime)
{
	ESP.wdtEnable(WdtTime);
}

void Wdg_Disable()
{
	ESP.wdtDisable();
}

void Wdg_Feed()
{
	ESP.wdtFeed();
}