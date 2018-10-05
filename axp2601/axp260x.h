#ifndef _AXP260X_H_
#define _AXP260X_H_

#include "GenericTypeDefs.h"

#define AXP260X_MANUFACTURER "X-POWERS"

#define AXP260X_SLAVEADDR 0xC4

/* AXP260X Regsiter addr
#define AXP260X_REG_ID 		(0x00) 
#define AXP260X_REG_BROM	(0x01)
#define AXP260X_REG_MODE	(0x02)
#define AXP260X_REG_CONFIG	(0x03)
#define AXP260X_REG_VBAT	(0x04)
#define AXP260X_REG_TM		(0x06)
#define AXP260X_REG_SOC		(0x08)
#define AXP260X_REG_T2E		(0x0A)
#define AXP260X_REG_T2F		(0x0C)
#define AXP260X_REG_LOWSOC	(0x0E)
#define AXP260X_REG_IRQ		(0x20)
#define AXP260X_REG_IRQMASK	(0x21)
*/

/* AXP260X params */
#define AXP2601_VERSION		(0x18)
#define AXP2601_INVALID_REGADDR (0x22)

typedef union {
	UINT8 byte;
	struct {
		unsigned version:2;
		unsigned type:6;
	};
} reg_deviceid_t;

typedef union {
	UINT8 byte;
	struct {
		unsigned :2;
		unsigned reset:1;
		unsigned por:1;
		unsigned :3;
		unsigned sleep:1;
	};
} reg_mode_t;

typedef union {
	UINT8 byte;
	struct {
		unsigned :2;
		unsigned enwdt:1;
		unsigned rom_sel:1;
		unsigned :3;
		unsigned bromupen:1;
	};
} reg_config_t;

typedef union {
	UINT8 byte;
	struct {
		unsigned :4;
		unsigned wdt:1;
		unsigned ot:1;
		unsigned newsoc;
		unsigned lowsoc;
	};
} reg_irq_t;

typedef union {
	UINT8 byte;
	struct {
		unsigned :4;
		unsigned wdt:1;
		unsigned ot:1;
		unsigned newsoc:1;
		unsigned lowsoc:1;
	};
} reg_irqmask_t;

struct axp260x_reg_t {
	reg_deviceid_t 	deviceid;
	UINT8		 	brom;
	reg_mode_t	 	mode;
	reg_config_t	config;
	UINT16		 	vbat;
	INT8		 	temp;
	UINT8		 	soc;
	UINT16		 	t2e;
	UINT16		 	t2f;
	UINT8		 	lowsocth;
	reg_irq_t		irq;
	reg_irqmask_t	irqmask;
};

enum axp260x_regaddr_index {
	AXP260X_REG_ID = 0,
	AXP260X_REG_BROM,
	AXP260X_REG_MODE,
	AXP260X_REG_CONFIG,
	AXP260X_REG_VBAT,
	AXP260X_REG_TM,
	AXP260X_REG_SOC,
	AXP260X_REG_T2E,
	AXP260X_REG_T2F,
	AXP260X_REG_LOWSOC,
	AXP260X_REG_IRQ,
	AXP260X_REG_IRQMASK,
	AXP260X_REG_MAX,
};


enum axp260x_chip {
	AXP2601 = 0,
	AXP2602 = 1,  
};

struct axp260x_model_data {
	UINT8 *model;
	size_t model_size;
};

enum axp260x_irq {
	AXP260x_IRQ_WDT = 0x08,
	AXP260X_IRQ_OT = 0x04,
	AXP260X_IRQ_NEWSOC = 0x02,
	AXP260X_IRQ_LOWSOC = 0x01,
	AXP260X_IRQ_ALL = 0x0F,
};

struct axp260x_device_info {
	struct device *dev;
	int id;
	const INT8 *name;
	UINT8 batnum;
	enum axp260x_chip chip;
	UINT8 *regaddrs;
	struct axp260x_reg_t regcache;
	struct axp260x_model_data data;
	
	struct power_supply *bat;	
	int (*read)(struct axp260x_device_info *di, UINT8 regaddr, UINT8 *regdata, UINT8 bytenum);
	int (*write)(struct axp260x_device_info *di, UINT8 regaddr, UINT8 *regdata, UINT8 bytenum);	
};
int axp260x_model_update(struct axp260x_device_info *di);
int axp260x_reg_update(struct axp260x_device_info *di);
int axp260x_setup(struct axp260x_device_info *di);
void axp260x_teardown(struct axp260x_device_info *di);

#endif