#include <linux/module.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/param.h>
#include <linux/jiffies.h>
#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/types.h>

#include "GenericTypeDefs.h"
#include "axp260x.h"

#define AXP260X_MASK_WDT	(0x1 << 3)
#define AXP260X_MASK_OT		(0x1 << 2)
#define AXP260X_MASK_NEWSOC	(0x1 << 1)
#define AXP260X_MASK_LOWSOC	(0x1 << 0)

#define AXP260X_MODE_RSTMCU	(0x1 << 5)
#define AXP260X_MODE_POR	(0x1 << 4)
#define AXP260X_MODE_SLEEP	(0x1 << 0)

#define AXP260X_CFG_ENWDT	(0x1 << 5)
#define AXP260X_CFG_ROMSEL	(0x1 << 4)
#define AXP260X_CFG_BROMUP	(0x1 << 0)

#define AXP260X_VBAT_MAX	(4500)
#define AXP260X_VBAT_MIN	(2000)

struct axp260x_reg_t axp2601_regdata;

static enum power_supply_property axp2601_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_CAPACITY_ALERT_MIN,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_TEMP_ALERT_MIN,
	POWER_SUPPLY_PROP_TIME_TO_EMPTY_NOW,
	POWER_SUPPLY_PROP_TIME_TO_FULL_NOW,
	POWER_SUPPLY_PROP_SERIAL_NUMBER,
	POWER_SUPPLY_PROP_MANUFACTURER,
};
/* not test yelt
static enum power_supply_property axp2602_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	...
}; 
*/
static UINT8 axp2601_model[] = {
	0x01,0x7C,0x00,0x00,0xFB,0x00,0x00,0xFB,0x00,0x1E,0x32,0x01,0x14,0x04,0xD8,0x04,
	0x74,0xFC,0xF4,0x0D,0xA7,0x10,0x04,0xFB,0xC8,0x00,0xBE,0x0D,0x1B,0x06,0x4B,0x06,
	0x04,0x0A,0xD5,0x0F,0x62,0x0F,0x14,0x09,0xBF,0x0E,0x41,0x0E,0x32,0x04,0x1B,0x04,
	0x06,0x08,0xF2,0x0D,0xD7,0x0D,0xC9,0x08,0xB1,0x0D,0x88,0x0D,0x70,0x03,0x48,0x03,
	0x1C,0x08,0x07,0x0C,0xEF,0x0C,0xBC,0x06,0xC2,0xB9,0x9E,0x9D,0x91,0x73,0x40,0x29,
};

static struct axp260x_model_data axp2601_model_data = {
	.model = axp2601_model,
	.model_size = ARRAY_SIZE(axp2601_model),
};

static UINT8 axp2601_regaddrs[AXP260X_REG_MAX] = {
	[AXP260X_REG_ID] = 0x00,
	[AXP260X_REG_BROM] = 0x01,
	[AXP260X_REG_MODE] = 0x02,
	[AXP260X_REG_CONFIG] = 0x03,
	[AXP260X_REG_VBAT] = 0x04,
	[AXP260X_REG_TM] = 0x06,
	[AXP260X_REG_SOC] = 0x08,
	[AXP260X_REG_T2E] = 0x0A,
	[AXP260X_REG_T2F] = 0x0C,
	[AXP260X_REG_LOWSOC] = 0x0E,
	[AXP260X_REG_IRQ] = 0x20,
	[AXP260X_REG_IRQMASK] = 0x21,
};

/*
static UINT8 axp2602_regaddrs[AXP260x_REG_MAX] = {
	[AXP260x_REG_ID] = 0x00,
	[AXP260x_REG_BROM] = 0x01,
	[AXP260x_REG_MODE] = 0x02,
	[AXP260x_REG_CONFIG] = 0x03,
	[AXP260x_REG_VBAT] = 0x04,
	[AXP260x_REG_TM] = 0x06,
	[AXP260x_REG_SOC] = 0x08,
	[AXP260x_REG_T2E] = 0x0A,
	[AXP260x_REG_T2F] = 0x0C,
	[AXP260x_REG_LOWSOC] = 0x0E,
	[AXP260x_REG_IRQ] = 0x20,
	[AXP260x_REG_IRQMASK] = 0x21,	
}; */ //not test yelt


static int axp260x_read_vbat(struct axp260x_device_info *di, union power_supply_propval *val)
{
	UINT8 data[2];
	UINT16 vtemp[3], tempv;
	int ret;
	UINT8 i;

	for (i = 0; i<3; i++){
		ret = di->read(di,di->regaddrs[AXP260X_REG_VBAT], data, 2);
		if (ret)
			return ret;
		vtemp[i] = ((data[0] << 0x08) | (data[1]));
	}
	if (vtemp[0] > vtemp[1]){
		tempv = vtemp[0];
		vtemp[0] = vtemp[1];
		vtemp[1] = tempv;
	}
	if (vtemp[1] > vtemp[2]){
		tempv = vtemp[1];
		vtemp[1] = vtemp[2];
		vtemp[2] = tempv;
	}
	if (vtemp[0] > vtemp[1]){
		tempv = vtemp[0];
		vtemp[0] = vtemp[1];
		vtemp[1] = tempv;
	}
	/*incase vtemp[1] exceed AXP260X_VBAT_MAX */
	if ((vtemp[1] > AXP260X_VBAT_MAX) || (vtemp[1] < AXP260X_VBAT_MIN)){
		val->intval = di->regcache.vbat;
		return 0;
	}
	di->regcache.vbat = vtemp[1];
	val->intval = vtemp[1];
	return 0;	
}

/* read temperature */
static int axp260x_read_temp(struct axp260x_device_info *di, union power_supply_propval *val)
{
	UINT8 data;
	int ret;
	ret = di->read(di,di->regaddrs[AXP260X_REG_TM], &data, 1);
	if (ret)
		return ret;	
	di->regcache.temp = data;
	val->intval = data;	
	return 0;
}

static int axp260x_read_soc(struct axp260x_device_info *di, union power_supply_propval *val)
{
	UINT8 data;
	int ret;
	ret = di->read(di,di->regaddrs[AXP260X_REG_SOC], &data, 1);
	if (ret)
		return ret;	
	di->regcache.vbat = data;
	val->intval = data;	
	return 0;
}

static int axp260x_read_time2empty(struct axp260x_device_info *di, union power_supply_propval *val)
{
	UINT8 data[2];
	UINT16 ttemp[3], tempt;
	int ret;
	UINT8 i;

	for (i = 0; i<3; i++){
		ret = di->read(di,di->regaddrs[AXP260X_REG_T2E], data, 2);
		if (ret)
			return ret;
		ttemp[i] = ((data[0] << 0x08) | (data[1]));
	}
	if (ttemp[0] > ttemp[1]){
		tempt = ttemp[0];
		ttemp[0] = ttemp[1];
		ttemp[1] = tempt;
	}
	if (ttemp[1] > ttemp[2]){
		tempt = ttemp[1];
		ttemp[1] = ttemp[2];
		ttemp[2] = tempt;
	}
	if (ttemp[0] > ttemp[1]){
		tempt = ttemp[0];
		ttemp[0] = ttemp[1];
		ttemp[1] = tempt;
	}
	di->regcache.t2e = ttemp[1];
	val->intval = ttemp[1];
	return 0;	
}

static int axp260x_read_time2full(struct axp260x_device_info *di, union power_supply_propval *val)
{
	UINT8 data[2];
	UINT16 ttemp[3], tempt;
	int ret;
	UINT8 i;

	for (i = 0; i<3; i++){
		ret = di->read(di,di->regaddrs[AXP260X_REG_T2F], data, 2);
		if (ret)
			return ret;
		ttemp[i] = ((data[0] << 0x08) | (data[1]));
	}
	if (ttemp[0] > ttemp[1]){
		tempt = ttemp[0];
		ttemp[0] = ttemp[1];
		ttemp[1] = tempt;
	}
	if (ttemp[1] > ttemp[2]){
		tempt = ttemp[1];
		ttemp[1] = ttemp[2];
		ttemp[2] = tempt;
	}
	if (ttemp[0] > ttemp[1]){
		tempt = ttemp[0];
		ttemp[0] = ttemp[1];
		ttemp[1] = tempt;
	}
	di->regcache.t2f = ttemp[1];
	val->intval = ttemp[1];
	return 0;	
}

static int axp260x_read_irq(struct axp260x_device_info *di, union power_supply_propval *val)
{
	UINT8 data;
	int ret;
	ret = di->read(di,di->regaddrs[AXP260X_REG_IRQ], &data, 1);
	if (ret)
		return ret;	
	di->regcache.irq.byte = data;
	val->intval = data;	
	return 0;
}

static int axp260x_read_lowsocth(struct axp260x_device_info *di, union power_supply_propval *val)
{
	UINT8 data;
	int ret;
	ret = di->read(di,di->regaddrs[AXP260X_REG_LOWSOC], &data, 1);
	if (ret)
		return ret;	
	di->regcache.lowsocth = data;
	val->intval = data;	
	return 0;
}

static int axp260x_clear_irq(struct axp260x_device_info *di)
{
	int ret;
	UINT8 data;
	
	data = 0x0F;
	ret = di->write(di,di->regaddrs[AXP260X_REG_IRQ], &data, 1);
	if (ret)
		return ret;
	data = 0x00;
	ret = di->write(di,di->regaddrs[AXP260X_REG_IRQ], &data, 1);
	if (ret)
		return ret;
	return 0;
}

static int axp260x_mask_irq(struct axp260x_device_info *di, enum axp260x_irq irq)
{
	int ret;
	UINT8 data;

	ret = di->read(di,di->regaddrs[AXP260X_REG_IRQMASK], &data, 1);
	if (ret)
		return ret;	
	data |= (UINT8) irq;
	ret = di->write(di,di->regaddrs[AXP260X_REG_IRQMASK], &data, 1);
	if (ret)
		return ret;
	return 0;	
}

static int axp260x_unmask_irq(struct axp260x_device_info *di, enum axp260x_irq irq)
{
	int ret;
	UINT8 data;

	ret = di->read(di,di->regaddrs[AXP260X_REG_IRQMASK], &data, 1);
	if (ret)
		return ret;	
	data &=~(0xF0 | (UINT8) irq);
	ret = di->write(di,di->regaddrs[AXP260X_REG_IRQMASK], &data, 1);
	if (ret)
		return ret;
	return 0;	
}

static int axp260x_set_lowsocth(struct axp260x_device_info *di, UINT8 data)
{
	int ret;
	
	if (data > 63)
		return 1;
	ret = di->write(di,di->regaddrs[AXP260X_REG_LOWSOC], &data, 1);
	if (ret)
		return ret;
	return 0;
}

static int axp260x_reset_mcu(struct axp260x_device_info *di)
{
	int ret;
	UINT8 data;
	
	ret = di->read(di,di->regaddrs[AXP260X_REG_MODE], &data, 1);
	if (ret)
		return ret;	
	data |= AXP260X_MODE_RSTMCU;
	ret = di->write(di,di->regaddrs[AXP260X_REG_MODE], &data, 1);
	if (ret)
		return ret;
	data &= ~AXP260X_MODE_RSTMCU;
	ret = di->write(di,di->regaddrs[AXP260X_REG_MODE], &data, 1);
	if (ret)
		return ret;	
	
	return 0;
}

static int axp260x_softpor(struct axp260x_device_info *di)
{
	int ret;
	UINT8 data;
	
	ret = di->read(di,di->regaddrs[AXP260X_REG_MODE], &data, 1);
	if (ret)
		return ret;	
	data |= AXP260X_MODE_POR;
	ret = di->write(di,di->regaddrs[AXP260X_REG_MODE], &data, 1);
	if (ret)
		return ret;
	data &= ~AXP260X_MODE_POR;
	ret = di->write(di,di->regaddrs[AXP260X_REG_MODE], &data, 1);
	if (ret)
		return ret;	
	return 0;
}

static int axp260x_enter_sleep(struct axp260x_device_info *di)
{
	int ret;
	UINT8 data;
	
	ret = di->read(di,di->regaddrs[AXP260X_REG_MODE], &data, 1);
	if (ret)
		return ret;	
	data |= AXP260X_MODE_SLEEP;
	ret = di->write(di,di->regaddrs[AXP260X_REG_MODE], &data, 1);
	if (ret)
		return ret;
	return 0;
}

static int axp260x_exit_sleep(struct axp260x_device_info *di)
{
	int ret;
	UINT8 data;
	
	ret = di->read(di,di->regaddrs[AXP260X_REG_MODE], &data, 1);
	if (ret)
		return ret;	
	data &= ~AXP260X_MODE_SLEEP;
	ret = di->write(di,di->regaddrs[AXP260X_REG_MODE], &data, 1);
	if (ret)
		return ret;
	return 0;
}

static int axp260x_en_wdt(struct axp260x_device_info *di)
{
	int ret;
	UINT8 data;
	
	ret = di->read(di,di->regaddrs[AXP260X_REG_CONFIG], &data, 1);
	if (ret)
		return ret;	
	data |= AXP260X_CFG_ENWDT;
	ret = di->write(di,di->regaddrs[AXP260X_REG_CONFIG], &data, 1);
	if (ret)
		return ret;
	return 0;
}

static int axp260x_dis_wdt(struct axp260x_device_info *di)
{
	int ret;
	UINT8 data;
	
	ret = di->read(di,di->regaddrs[AXP260X_REG_CONFIG], &data, 1);
	if (ret)
		return ret;	
	data &= ~AXP260X_CFG_ENWDT;
	ret = di->write(di,di->regaddrs[AXP260X_REG_CONFIG], &data, 1);
	if (ret)
		return ret;
	return 0;
}


int axp260x_model_update(struct axp260x_device_info *di)
{
	int ret;
	UINT8 val;
	UINT8 para[di->data.model_size];
	UINT8 i;
	
	
	ret = di->read(di,di->regaddrs[AXP260X_REG_CONFIG], &val, 1);
	if (ret)
		return ret;
	val |= AXP260X_CFG_BROMUP;
	ret = di->write(di,di->regaddrs[AXP260X_REG_CONFIG], &val, 1);
	if (ret)
		return ret;
	for (i = 0; i < di->data.model_size; i++){
		ret = di->write(di,di->regaddrs[AXP260X_REG_BROM], &di->data.model[i], 1);
		if (ret)
			return ret;		
	}
	val &= ~AXP260X_CFG_BROMUP;
	ret = di->write(di,di->regaddrs[AXP260X_REG_CONFIG], &val, 1);
	if (ret)
		return ret;
	val |= AXP260X_CFG_BROMUP;
	ret = di->write(di,di->regaddrs[AXP260X_REG_CONFIG], &val, 1);
	if (ret)
		return ret;
	for (i = 0; i < di->data.model_size; i++){
		ret = di->read(di,di->regaddrs[AXP260X_REG_BROM], &para[i], 1);
		if (ret)
			return ret;
		if (para[i] != di->data.model[i]){
			dev_dbg(di->dev, "model [%d] para reading %02x != write %02x\n", i, para[i], di->data.model[i]);
			return 1;
		}
	}
	val &= ~AXP260X_CFG_BROMUP;
	val |= AXP260X_CFG_ROMSEL;
	ret = di->write(di,di->regaddrs[AXP260X_REG_CONFIG], &val, 1);
	if (ret)
		return ret;	
	ret = axp260x_reset_mcu(di);
	if (ret)
		return ret;
	return 0;
}
//EXPORT_SYMBOL(axp260x_model_update)

static inline bool axp260x_model_update_check(struct axp260x_device_info *di)
{
	int ret;
	UINT8 data;
	ret = di->read(di,di->regaddrs[AXP260X_REG_CONFIG], &data, 1);
	return ((data & AXP260X_CFG_ROMSEL) && (!ret));	
}

int axp260x_reg_update(struct axp260x_device_info *di)
{
	int ret;
	UINT8 data[2];

	ret = di->read(di,di->regaddrs[AXP260X_REG_ID], &di->regcache.deviceid.byte, 1);
	if (ret)
		return ret;
	if (di->regcache.deviceid.byte != AXP2601_VERSION ){
		dev_err(di->dev, "Error device version %02x\n", di->regcache.deviceid.byte);
		return 1;
	}
	ret = di->read(di,di->regaddrs[AXP260X_REG_VBAT], data, 2);
	if (ret)
		return ret;
	di->regcache.vbat = (data[0] << 8) + data[1];
	ret = di->read(di,di->regaddrs[AXP260X_REG_TM], &di->regcache.temp, 1);
	if (ret)
		return ret;	
	ret = di->read(di,di->regaddrs[AXP260X_REG_SOC], &di->regcache.soc, 1);
	if (ret)
		return ret;		
	ret = di->read(di,di->regaddrs[AXP260X_REG_T2E], data, 2);
	if (ret)
		return ret;
	di->regcache.vbat = (data[0] << 8) + data[1];
	ret = di->read(di,di->regaddrs[AXP260X_REG_T2F], data, 2);
	if (ret)
		return ret;	
	di->regcache.vbat = (data[0] << 8) + data[1];
	ret = di->read(di,di->regaddrs[AXP260X_REG_LOWSOC], &di->regcache.lowsocth, 1);
	if (ret)
		return ret;	
	ret = di->read(di,di->regaddrs[AXP260X_REG_IRQ], &di->regcache.irq.byte, 1);
	if (ret)
		return ret;	
	return 0;
}
//EXPORT_SYMBOL(axp260x_reg_update)

static int axp260x_get_property(struct power_supply *psy, enum power_supply_property psp, 
						union power_supply_propval *val)
{
	int ret = 0;
	struct axp260x_device_info *di = power_supply_get_drvdata(psy);

	switch (psp) {
	case POWER_SUPPLY_PROP_STATUS:
		ret = axp260x_read_irq(di, val);
		val->intval = di->regcache.irq.byte;
		break;
	case POWER_SUPPLY_PROP_PRESENT:
		ret = axp260x_read_vbat(di, val);
		val->intval = di->regcache.irq.byte > 0? 1 : 0;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		ret = axp260x_read_vbat(di, val);
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		ret = axp260x_read_soc(di, val);
		break;
	case POWER_SUPPLY_PROP_CAPACITY_ALERT_MIN:
		ret = axp260x_read_lowsocth(di, val);
		break;
	case POWER_SUPPLY_PROP_TEMP:
		ret = axp260x_read_temp(di, val);
		break;
	case POWER_SUPPLY_PROP_TEMP_ALERT_MIN:
		val->intval = 85;
		break;
	case POWER_SUPPLY_PROP_TIME_TO_EMPTY_NOW:
		ret = axp260x_read_time2empty(di, val);
		break;
	case POWER_SUPPLY_PROP_TIME_TO_FULL_NOW:
		ret = axp260x_read_time2full(di, val);
		break;
	case POWER_SUPPLY_PROP_SERIAL_NUMBER:
		val->intval = di->regcache.deviceid.byte;
		break;
	case POWER_SUPPLY_PROP_MANUFACTURER:
		val->strval = AXP260X_MANUFACTURER;
		break;
	default:
		return -EINVAL;
	}
	return ret;	
}

int axp260x_setup(struct axp260x_device_info *di)
{
	int ret;
	struct power_supply_desc *psy_desc;
	struct power_supply_config psy_cfg = {
		.of_node = di->dev->of_node,
		.drv_data = di,
	};
	
	di->regaddrs = axp2601_regaddrs;
	di->data = axp2601_model_data;
	psy_desc = devm_kzalloc(di->dev, sizeof(*psy_desc), GFP_KERNEL);
	if (!psy_desc)
		return -ENOMEM;
	psy_desc->name = di->name;
	psy_desc->type = POWER_SUPPLY_TYPE_BATTERY;
	psy_desc->properties = axp2601_props;
	psy_desc->num_properties = ARRAY_SIZE(axp2601_props);
	psy_desc->get_property = axp260x_get_property;
	di->bat = power_supply_register_no_ws(di->dev, psy_desc, &psy_cfg);
	if (IS_ERR(di->bat)){
		dev_err(di->dev, "failed to register battery\n");
		return PTR_ERR(di->bat);
	}
	
	ret = axp260x_reg_update(di);
	if (ret)
		return ret;
	if (di->regcache.mode.sleep){
		ret = axp260x_exit_sleep(di);
		if (ret)
			return ret;
	}
	ret = axp260x_reg_update(di);
	if (ret)
		return ret;
	if (!axp260x_model_update_check){
		ret = axp260x_model_update(di);
		if (ret)
			return ret;
	}
	return 0;
}
EXPORT_SYMBOL_GPL(axp260x_setup);

void axp260x_teardown(struct axp260x_device_info *di)
{
	power_supply_unregister(di->bat);
}
EXPORT_SYMBOL_GPL(axp260x_teardown);

MODULE_AUTHOR("x-powers <wangxiaoliang@x-powers.com>");
MODULE_DESCRIPTION("axp260x egauge driver");
MODULE_LICENSE("GPL");



























