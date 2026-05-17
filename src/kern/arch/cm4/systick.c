#include <cm4.h>
#include <sys_clock.h>

static volatile uint32_t g_msTick = 0U;

void __SysTick_init(uint32_t tick_hz) {
  uint32_t reload = (__AHB_CLK() / tick_hz) - 1U;
  // if (reload > (SysTick_LOAD_RELOAD_Msk)) {
  //   reload = SysTick_LOAD_RELOAD_Msk;
  // }
  SYSTICK->LOAD = reload;
  SYSTICK->VAL = 0U;
  NVIC_SetPriority(SysTick_IRQn, 0xFFU);
  SYSTICK->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk |
                  SysTick_CTRL_ENABLE_Msk;
}

void SysTickIntDisable(void) { SYSTICK->CTRL &= ~SysTick_CTRL_TICKINT_Msk; }

void SysTickIntEnable(void) { SYSTICK->CTRL |= SysTick_CTRL_TICKINT_Msk; }

void __SysTick_enable(void) { SYSTICK->CTRL |= SysTick_CTRL_ENABLE_Msk; }

void __sysTick_disable(void) { SYSTICK->CTRL &= ~SysTick_CTRL_ENABLE_Msk; }

uint32_t __getSysTickCount(void) { return SYSTICK->VAL; }

void __updateSysTick(uint32_t tick_hz) {
  uint32_t reload = (__AHB_CLK() / tick_hz) - 1U;
  if (reload > (SysTick_LOAD_RELOAD_Msk)) {
    reload = SysTick_LOAD_RELOAD_Msk;
  }
  SYSTICK->LOAD = reload;
  SYSTICK->VAL = 0U;
}

uint32_t __getTime(void) { return g_msTick; }

uint32_t __get__Second(void) { return g_msTick / 1000U; }

uint32_t __get__Minute(void) { return g_msTick / (60U * 1000U); }

uint32_t __get__Hour(void) { return g_msTick / (3600U * 1000U); }

void SysTick_Handler(void) {
  static uint8_t tick10 = 0U;
  g_msTick++;
  tick10++;
  if (tick10 >= 10U) {
    tick10 = 0U;
    SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
  }
}

uint8_t ms_delay(uint32_t delay) {
  uint32_t start = g_msTick;
  while ((g_msTick - start) < delay) {
    __WFI();
  }
  return 0U;
}

uint32_t getmsTick(void) { return g_msTick; }

uint32_t wait_until(uint32_t deadline) {
  while ((int32_t)(deadline - g_msTick) > 0) {
    __WFI();
  }
  return g_msTick;
}
