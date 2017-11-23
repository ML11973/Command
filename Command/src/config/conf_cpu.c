/*
 * conf_cpu.c
 *
 * Created: 06.11.2017 09:07:51
 *  Author: leemannma
 */ 

#include <asf.h>
#include "conf_cpu.h"

#define PLL			 0
#define MUL			 7
#define DIV			 1
#define OSC			 0
#define LOCK_COUNT	 16
#define PLL_F		 1
#define PLL_DIV2	 1
#define PLL_WBW_DIS	 0

/* sysclk_init (void)
*
* Initializes SYSCLK for a 64 MHz clock.
*
*/
void sysclk_init (void) {
  // PLL formula = PLL_OUT (OSC0 / DIV) * (MUL+1) => (16MHz / 1) * (7+1) = 128MHz
  pm_switch_to_osc0(&AVR32_PM, XTAL_OSC0, OSC0_STARTUP_US);  // Switch main clock to Osc0.
  pm_pll_setup(&AVR32_PM, PLL, MUL, DIV, OSC, LOCK_COUNT);
  /*
   This function will set a PLL option.
   *pm Base address of the Power Manager (i.e. &AVR32_PM)
   pll PLL number 0
   pll_freq Set to 1 for VCO frequency range 80-180MHz, set to 0 for VCO frequency range 160-240Mhz.
   pll_div2 Divide the PLL output frequency by 2 (this settings does not change the FVCO value)
   pll_wbwdisable 1 Disable the Wide-Bandith Mode (Wide-Bandwith mode allow a faster startup time and out-of-lock time). 0 to enable the Wide-Bandith Mode.
  */
  /* PLL output VCO frequency is 128MHz. We divide it by 2 with the pll_div2=1. This enable to get later main clock to 64MHz */
  pm_pll_set_option(&AVR32_PM, PLL, PLL_F, PLL_DIV2, PLL_WBW_DIS);
  pm_pll_enable(&AVR32_PM, PLL);

  /* Wait for PLL0 locked */
  pm_wait_for_pll0_locked(&AVR32_PM) ;
  pm_cksel(&AVR32_PM, 1, 0, 0, 0, 0, 0);		// 1 divides PBA clock by two

  // Set one wait-state (WS) for flash controller. 0 WS access is up to 30MHz for HSB/CPU clock.
  // As we want to have 64MHz on HSB/CPU clock, we need to set 1 WS on flash controller.
  flashc_set_wait_state(1);
  pm_switch_to_clock(&AVR32_PM, AVR32_PM_MCSEL_PLL0); /* Switch main clock to 64MHz */	
}
