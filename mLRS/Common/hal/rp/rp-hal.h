//*******************************************************
// Copyright (c) MLRS project
// GPL3
// https://www.gnu.org/licenses/gpl-3.0.de.html
//*******************************************************
#ifndef RP_HAL_H
#define RP_HAL_H

#if defined RX_DIY_2400_RP2040 || defined RX_DIY_2400_RP2350
    #include "rx-hal-diy-2400-rp.h"
#endif

#if defined TX_DIY_2400_RP2040 || defined TX_DIY_2400_RP2350
    #include "tx-hal-diy-2400-rp.h"
#endif

#if defined RX_WAVESHARE_RP2040
    #include "rx-hal-waveshare-rp.h"
#endif

#if defined TX_WAVESHARE_RP2040
    #include "tx-hal-waveshare-rp.h"
#endif

#if defined RX_WAVESHARE_RP2040_433
    #include "rx-hal-waveshare-rp2040-433.h"
#endif

#if defined TX_WAVESHARE_RP2040_433
    #include "tx-hal-waveshare-rp2040-433.h"
#endif

#endif // RP_HAL_H
