/*
 * drivers/power/axp/AXP2585/AXP2585.h
 * (C) Copyright 2010-2016
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Pannan <pannan@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */
#ifndef AXP2585_H_
#define AXP2585_H_

/* For AXP2585 */
#define AXP2585_STATUS1              (0x00)
#define AXP2585_STATUS2              (0x01)
#define AXP2585_STATUS3             (0x02)
#define AXP2585_IC_TYPE             (0x03)
#define AXP2585_STATUS4              (0x04)
#define AXP2585_STATUS5             (0x05)
#define AXP2585_PWR                     (0x06)
#define AXP2585_IHOLD                  (0x10)
#define AXP2585_VINDPM              (0x11)
#define AXP2585_VSYSMIN              (0x12)
#define AXP2585_BSTCTRL              (0x13)
#define AXP2585_WDTCTRL              (0x14)
#define AXP2585_POKEY              (0x15)
#define AXP2585_THER_CTRL           (0x18)
#define AXP2585_BC_CTRL1           (0x20)
#define AXP2585_BC_CTRL2           (0x21)
#define AXP2585_BC_CTRL3          (0x22)
#define AXP2585_DPDM           (0x23)

#define AXP2585_POK_SET             (0x15)
#define AXP2585_OFF_CTL             (0x28)
#define AXP2585_INTEN1              (0x40)
#define AXP2585_INTEN2              (0x41)
#define AXP2585_INTEN3              (0x42)
#define AXP2585_INTEN4              (0x43)
#define AXP2585_INTEN5              (0x44)
#define AXP2585_INTEN6              (0x45)
#define AXP2585_INTSTS1             (0x48)
#define AXP2585_INTSTS2             (0x49)
#define AXP2585_INTSTS3             (0x4A)
#define AXP2585_INTSTS4             (0x4B)
#define AXP2585_INTSTS5             (0x4C)
#define AXP2585_INTSTS6             (0x4D)

#define AXP2585_DIETEMP         (0x56)
#define AXP2585_TSADC                     (0x58)
#define AXP2585_GPADC         (0x5A)
#define AXP2585_BATVOL         (0x78)
#define AXP2585_BATICHG      (0x7A)
#define AXP2585_BATIDIS      (0x7C)
#define AXP2585_ADCEN       (0x80)
#define AXP2585_TS_PIN_CONTROL      (0x81)
#define AXP2585_HYSL2H                   (0x82)
#define AXP2585_HYSH2L                (0x83)
#define AXP2585_VLTF_CHG            (0x84)
#define AXP2585_VHTF_CHG            (0x85)
#define AXP2585_VLTF_WORK            (0x86)
#define AXP2585_VHTF_WORK           (0x87)
#define AXP2585_VLTF_GPADC            (0x88)
#define AXP2585_VHTF_GPADC           (0x89)
#define AXP2585_CHG_CTRL1             (0x8A)
#define AXP2585_CHG_CTRL2             (0x8B)
#define AXP2585_CHG_CTRL3             (0x8C)
#define AXP2585_CHG_CTRL4             (0x8D)
#define AXP2585_CHG_CTRL5             (0x8E)
#define AXP2585_CHGLED             (0x90)
#define AXP2585_GAUGE_CTRL   (0xB8)
#define AXP2585_CAP_PERCENT             (0xB9)

#define AXP2585_OCV1                            (0xBC)
#define AXP2585_MAX_CAP                   (0xE0)
#define AXP2585_COUL_COUNTER       (0xE2)
#define AXP2585_OCV_PERCENT           (0xE4)
#define AXP2585_COUL_PERCENT       (0xE5)
#define AXP2585_WARNING_LEVEL       (0xE6)
#define AXP2585_OCV_CURVE                 (0xE7)
#define AXP2585_GAUGE_TUNE0            (0xE8)
#define AXP2585_GAUGE_TUNE1            (0xE9)
#define AXP2585_GAUGE_TUNE2            (0xEA)
#define AXP2585_GAUGE_TUNE3            (0xEB)
#define AXP2585_GAUGE_TUNE4            (0xEC)
#define AXP2585_GAUGE_TUNE5           (0xED)
#define AXP2585_GAUGE_TUNE6            (0xEE)
#define AXP2585_GAUGE_TUNE7            (0xEF)
#define AXP2585_ADDR_EXTENSION      (0xFF)


/* bit definitions for AXP events ,irq event */
/* AXP2585 */
//Enable bits:Reg 40H[0:7]<------------->Status bits:Reg 48H[0:7]
#define AXP2585_IRQ_BFETOCP       (0)//BATFET OCP
#define AXP2585_IRQ_BSTOCP        (1)//Boost OCP
#define AXP2585_IRQ_BSTOVP         (2)//Boost OVP
#define AXP2585_IRQ_BATDET          (3)//Battery detcted done
#define AXP2585_IRQ_GAUGECALDONE          (4)//Gauge calculation done
#define AXP2585_IRQ_BATPERCHANGE          (5)//bat capacity percent change
#define AXP2585_IRQ_LOWN1         (6)//Battery capacity percentage drop to warning level1
#define AXP2585_IRQ_LOWN2         (7)//Battery capacity percentage drop to warning level2

//Enable bits:Reg 41H[0:7]<------------->Status bits:Reg 49H[0:7]
#define AXP2585_IRQ_QBWUT         (8)//Quit Battery under temperature in work mode
#define AXP2585_IRQ_BWUT          (9)//Battery under temperature in work mode
#define AXP2585_IRQ_QBWOT         (10)//Quit Battery over temperature in work mode
#define AXP2585_IRQ_BWOT          (11)//Battery over temperature in work mode
#define AXP2585_IRQ_QBCUT         (12)//Quit Battery under temperature in charge mode
#define AXP2585_IRQ_BCUT          (13)//Battery under temperature in charge mode
#define AXP2585_IRQ_QBCOT         (14)//Quit Battery over temperature in charge mode
#define AXP2585_IRQ_BCOT          (15)//Battery over temperature in charge mode

//Enable bits:Reg 42H[0:7]<------------->Status bits:Reg 4AH[0:7]
#define AXP2585_IRQ_VBUSOVP                (16)//VBUS over voltage protection IRQ
#define AXP2585_IRQ_SAFEMODE              (17)//Charger safety timer1/2 timeout and battery enters safe mode IRQ
#define AXP2585_IRQ_DIEOT                        (18)//Die over temperature IRQ enable
#define AXP2585_IRQ_DBCNORMAL          (19)//Dead/Weak Battery charge to normal IRQ
#define AXP2585_IRQ_BATRE         (20)//Battery removal IRQ
#define AXP2585_IRQ_BATIN         (21)//Battery insert IRQ
#define AXP2585_IRQ_ACRE          (22)//VBUS removal IRQ
#define AXP2585_IRQ_ACIN          (23)//VBUS insertion IRQ

//Enable bits:Reg 43H[0:7]<------------->Status bits:Reg 4BH[0:7]
#define AXP2585_IRQ_QGPADCUT         (24)//Quit GPADC under temperature IRQ
#define AXP2585_IRQ_GPADCUT        (25)//GPADC under temperature IRQ
#define AXP2585_IRQ_QGPADCOT        (26)//Quit GPADC over temperature IRQ
#define AXP2585_IRQ_GPADCOT         (27)//GPADC over temperature IRQ
#define AXP2585_IRQ_PEKRE         (28)//PWRON positive edge IRQ enable(PWRON from low go high)
#define AXP2585_IRQ_PEKFE         (29)//PWRON negative edge IRQ enable(PWRON from high go low)
#define AXP2585_IRQ_PEKLP         (30)//PRWON long press IRQ
#define AXP2585_IRQ_PEKSP    (31)//PWRON short press IRQ

//Enable bits:Reg 44H[0:7]<------------->Status bits:Reg 4CH[0:7]
#define AXP2585_IRQ_BATOVP        (34)//BAT over voltage protection IRQ
#define AXP2585_IRQ_RID              (35)//RID detect result change IRQ
#define AXP2585_IRQ_BCCHANGE         (36)//BC1.2 detect result change IRQ
#define AXP2585_IRQ_BCDONE         (37)//BC1.2 detect finished IRQ
#define AXP2585_IRQ_CHAOV         (38)//Battery charge done IRQ
#define AXP2585_IRQ_CHAST         (39)//Charger begin charging IRQ

//Enable bits:Reg 45H[0:7]<------------->Status bits:Reg 4DH[0:7]
#define AXP2585_IRQ_TCPWRCHANGE          (41)//Type-C power state changed IRQ enable
#define AXP2585_IRQ_TCERR         (42)//Type-C error generated IRQ
#define AXP2585_IRQ_TCIN		  (46)//Type-C device insert and detection finished IRQ
#define AXP2585_IRQ_TCRE         (47)//Type-C device removed (unattached) IRQ

extern int axp_debug;
extern struct axp_config_info axp2585_config;

#endif /* AXP2585_H_ */
