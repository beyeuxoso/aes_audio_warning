
#ifndef __WATCH_DOG_H_
#define __WATCH_DOG_H_

// Software watchdog
#define TIMMER_TICK_1MS				5000U
#define TIMMER_TICK_10MS			50000U
#define TIMMER_TICK_100MS			500000U
#define TIMMER_TICK_1000MS			5000000U

#define TIMMER_CNT_1S				1U				// 1s
#define TIMMER_CNT_2S				2U				// 2s
#define TIMMER_CNT_5S				5U				// 5s
#define TIMMER_CNT_10S				10U				// 10s
#define TIMMER_CNT_100S				100U			// 100s
#define TIMMER_CNT_150S				150U			// 150s

// Hardware watchdog
#define TIMMER_CNT_6000MS			6000U

extern void Wdg_Enable(uint32_t WdtTime);

extern void Wdg_Disable();

extern void Wdg_Feed();

extern void SoftWdg_Enable(uint32_t timeout);

extern void SoftWdg_Disable();

extern void SoftWdg_Feed();

#endif /* __WATCH_DOG_H_ */



