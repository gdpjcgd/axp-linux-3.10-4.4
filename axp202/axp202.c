#include "../axp202/axp202.h"

#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/reboot.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/mfd/core.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/err.h>
#include <linux/power/aw_pm.h>
#include "../axp-core.h"
#include "../axp-charger.h"

static struct axp_dev *axp202_pm_power;
struct axp_config_info axp202_config;
struct wakeup_source *axp202_ws;
static int axp202_pmu_num;

static struct axp_regmap_irq_chip axp202_regmap_irq_chip = {
	.name           = "axp202_irq_chip",
	.status_base    = AXP202_INTSTS1,
	.enable_base    = AXP202_INTEN1,
	.num_regs       = 5,
};

static struct resource axp202_pek_resources[] = {
	{AXP202_IRQ_PEKRE,  AXP202_IRQ_PEKRE,  "PEK_DBR",    IORESOURCE_IRQ,},
	{AXP202_IRQ_PEKFE,  AXP202_IRQ_PEKFE,  "PEK_DBF",    IORESOURCE_IRQ,},
};

static struct resource axp202_charger_resources[] = {
	{AXP202_IRQ_USBIN,  AXP202_IRQ_USBIN,  "usb in",        IORESOURCE_IRQ,},
	{AXP202_IRQ_USBRE,  AXP202_IRQ_USBRE,  "usb out",       IORESOURCE_IRQ,},
	{AXP202_IRQ_ACIN,   AXP202_IRQ_ACIN,   "ac in",         IORESOURCE_IRQ,},
	{AXP202_IRQ_ACRE,   AXP202_IRQ_ACRE,   "ac out",        IORESOURCE_IRQ,},
	{AXP202_IRQ_BATIN,  AXP202_IRQ_BATIN,  "bat in",        IORESOURCE_IRQ,},
	{AXP202_IRQ_BATRE,  AXP202_IRQ_BATRE,  "bat out",       IORESOURCE_IRQ,},
	{AXP202_IRQ_TEMLO,  AXP202_IRQ_TEMLO,  "bat temp low",  IORESOURCE_IRQ,},
	{AXP202_IRQ_TEMOV,  AXP202_IRQ_TEMOV,  "bat temp over", IORESOURCE_IRQ,},
	{AXP202_IRQ_CHAST,  AXP202_IRQ_CHAST,  "charging",      IORESOURCE_IRQ,},
	{AXP202_IRQ_CHAOV,  AXP202_IRQ_CHAOV,  "charge over",   IORESOURCE_IRQ,},
};

static struct mfd_cell axp202_cells[] = {
	{
		.name          = "axp202-powerkey",
		.num_resources = ARRAY_SIZE(axp202_pek_resources),
		.resources     = axp202_pek_resources,
	},
	{
		.name          = "axp202-regulator",
	},
	{
		.name          = "axp202-charger",
		.num_resources = ARRAY_SIZE(axp202_charger_resources),
		.resources     = axp202_charger_resources,
	},
	{
		.name          = "axp202-gpio",
	},
};

void axp202_power_off(void)
{
	uint8_t val;

	if (axp202_config.pmu_pwroff_vol >= 2600
		&& axp202_config.pmu_pwroff_vol <= 3300) {
		if (axp202_config.pmu_pwroff_vol > 3200)
			val = 0x7;
		else if (axp202_config.pmu_pwroff_vol > 3100)
			val = 0x6;
		else if (axp202_config.pmu_pwroff_vol > 3000)
			val = 0x5;
		else if (axp202_config.pmu_pwroff_vol > 2900)
			val = 0x4;
		else if (axp202_config.pmu_pwroff_vol > 2800)
			val = 0x3;
		else if (axp202_config.pmu_pwroff_vol > 2700)
			val = 0x2;
		else if (axp202_config.pmu_pwroff_vol > 2600)
			val = 0x1;
		else
			val = 0x0;

		axp_regmap_update(axp202_pm_power->regmap,
				AXP202_VOFF_SET, val, 0x7);
	}

	/* led auto */
	axp_regmap_clr_bits(axp202_pm_power->regmap, 0x32, 0x38);
	axp_regmap_clr_bits(axp202_pm_power->regmap, 0xb9, 0x80);

	pr_info("[axp] send power-off command!\n");
	mdelay(20);

	if (axp202_config.power_start != 1) {
		axp_regmap_write(axp202_pm_power->regmap, AXP202_INTSTS3, 0x03);
		axp_regmap_read(axp202_pm_power->regmap, AXP202_STATUS, &val);
		if (val & 0xF0) {
			axp_regmap_read(axp202_pm_power->regmap,
					AXP202_MODE_CHGSTATUS, &val);
			if (val & 0x20) {
				pr_info("[axp] set flag!\n");
				axp_regmap_write(axp202_pm_power->regmap,
					AXP202_DATA_BUFFERC, 0x0f);
				mdelay(20);
				pr_info("[axp] reboot!\n");
				machine_restart(NULL);
				pr_warn("[axp] warning!!! arch can't reboot,\"\
					\" maybe some error happend!\n");
			}
		}
	}

	axp_regmap_write(axp202_pm_power->regmap, AXP202_DATA_BUFFERC, 0x00);
	axp_regmap_set_bits(axp202_pm_power->regmap, AXP202_OFF_CTL, 0x80);
	mdelay(20);
	pr_warn("[axp] warning!!! axp can't power-off,\"\
		\" maybe some error happend!\n");
}

static int axp202_init_chip(struct axp_dev *axp202)
{
	uint8_t chip_id, dcdc2_ctl;
	int err;

	err = axp_regmap_read(axp202->regmap, AXP202_IC_TYPE, &chip_id);
	if (err) {
		pr_err("[%s] try to read chip id failed!\n",
				axp_name[axp202_pmu_num]);
		return err;
	}

	if ((chip_id & 0x0f) == 0x1)
		pr_info("[%s] chip id detect 0x%x !\n",
				axp_name[axp202_pmu_num], chip_id);
	else
		pr_info("[%s] chip id not detect 0x%x !\n",
				axp_name[axp202_pmu_num], chip_id);

	/* enable dcdc2 dvm */
	err = axp_regmap_read(axp202->regmap, AXP202_LDO3_DC2_DVM, &dcdc2_ctl);
	if (err) {
		pr_err("[%s] try to read dcdc2 dvm failed!\n",
				axp_name[axp202_pmu_num]);
		return err;
	}

	dcdc2_ctl |= (0x1<<2);
	err = axp_regmap_write(axp202->regmap, AXP202_LDO3_DC2_DVM, dcdc2_ctl);
	if (err) {
		pr_err("[%s] try to enable dcdc2 dvm failed!\n",
				axp_name[axp202_pmu_num]);
		return err;
	}
	pr_info("[%s] enable dcdc2 dvm.\n", axp_name[axp202_pmu_num]);

	/*init 16's reset pmu en */
	if (axp202_config.pmu_reset)
		axp_regmap_set_bits(axp202->regmap, AXP202_HOTOVER_CTL, 0x08);
	else
		axp_regmap_clr_bits(axp202->regmap, AXP202_HOTOVER_CTL, 0x08);

	/*init irq wakeup en*/
	if (axp202_config.pmu_irq_wakeup)
		axp_regmap_set_bits(axp202->regmap, AXP202_HOTOVER_CTL, 0x80);
	else
		axp_regmap_clr_bits(axp202->regmap, AXP202_HOTOVER_CTL, 0x80);

	/*init pmu over temperature protection*/
	if (axp202_config.pmu_hot_shutdown)
		axp_regmap_set_bits(axp202->regmap, AXP202_HOTOVER_CTL, 0x04);
	else
		axp_regmap_clr_bits(axp202->regmap, AXP202_HOTOVER_CTL, 0x04);

	/*init inshort status*/
	if (axp202_config.pmu_inshort)
		axp_regmap_set_bits(axp202->regmap, AXP202_HOTOVER_CTL, 0x60);
	else
		axp_regmap_clr_bits(axp202->regmap, AXP202_HOTOVER_CTL, 0x60);

	return 0;
}

static void axp202_wakeup_event(void)
{
	__pm_wakeup_event(axp202_ws, 0);
}

static s32 axp202_usb_det(void)
{
	u8 value = 0;
	axp_regmap_read(axp202_pm_power->regmap, AXP202_STATUS, &value);
	return (value & 0x10) ? 1 : 0;
}

static s32 axp202_usb_vbus_output(int high)
{
	return 0;
}

static int axp202_cfg_pmux_para(int num, struct aw_pm_info *api, int *pmu_id)
{
	char name[8];
	struct device_node *np;

	sprintf(name, "pmu%d", num);

	np = of_find_node_by_type(NULL, name);
	if (NULL == np) {
		pr_err("can not find device_type for %s\n", name);
		return -1;
	}

	if (!of_device_is_available(np)) {
		pr_err("can not find node for %s\n", name);
		return -1;
	}

	api->pmu_arg.twi_port = axp202_pm_power->regmap->client->adapter->nr;
	api->pmu_arg.dev_addr = axp202_pm_power->regmap->client->addr;
	*pmu_id = axp202_config.pmu_id;

	return 0;
}

static const char *axp202_get_pmu_name(void)
{
	return axp_name[axp202_pmu_num];
}

static struct axp_dev *axp202_get_pmu_dev(void)
{
	return axp202_pm_power;
}

struct axp_platform_ops axp202_plat_ops = {
	.usb_det = axp202_usb_det,
	.usb_vbus_output = axp202_usb_vbus_output,
	.cfg_pmux_para = axp202_cfg_pmux_para,
	.get_pmu_name = axp202_get_pmu_name,
	.get_pmu_dev  = axp202_get_pmu_dev,
	.powerkey_name = {
		"axp2029-powerkey"
	},
	.charger_name = {
		"axp2029-charger"
	},
	.regulator_name = {
		"axp2029-regulator"
	},
	.gpio_name = {
		"axp2029-gpio"
	},
};

static const struct i2c_device_id axp202_id_table[] = {
	{ "axp202", 0 },
	{}
};

static const struct of_device_id axp202_dt_ids[] = {
	{ .compatible = "axp202", },
	{},
};
MODULE_DEVICE_TABLE(of, axp202_dt_ids);

static int axp202_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	int ret;
	struct axp_dev *axp202;
	struct device_node *node = client->dev.of_node;

	axp202_pmu_num = axp_get_pmu_num(axp202_dt_ids, ARRAY_SIZE(axp20_dt_ids));
	if (axp202_pmu_num < 0) {
		pr_err("%s get pmu num failed\n", __func__);
		return axp202_pmu_num;
	}

	if (node) {
		/* get dt and sysconfig */
		if (!of_device_is_available(node)) {
			axp202_config.pmu_used = 0;
			pr_err("%s: pmu_used = %u\n", __func__,
					axp202_config.pmu_used);
			return -EPERM;
		} else {
			axp202_config.pmu_used = 1;
			ret = axp_dt_parse(node, axp202_pmu_num, &axp202_config);
			if (ret) {
				pr_err("%s parse device tree err\n", __func__);
				return -EINVAL;
			}
		}
	} else {
		pr_err("AXP20x device tree err!\n");
		return -EBUSY;
	}

	axp202 = devm_kzalloc(&client->dev, sizeof(*axp202), GFP_KERNEL);
	if (!axp202)
		return -ENOMEM;

	axp202->dev = &client->dev;
	axp202->nr_cells = ARRAY_SIZE(axp202_cells);
	axp202->cells = axp202_cells;
	axp202->pmu_num = axp202_pmu_num;

	ret = axp_mfd_cell_name_init(&axp202_plat_ops,
				ARRAY_SIZE(axp202_dt_ids), axp202->pmu_num,
				axp202->nr_cells, axp202->cells);
	if (ret)
		return ret;

	axp202->regmap = axp_regmap_init_i2c(&client->dev);
	if (IS_ERR(axp202->regmap)) {
		ret = PTR_ERR(axp202->regmap);
		dev_err(&client->dev, "regmap init failed: %d\n", ret);
		return ret;
	}

	ret = axp202_init_chip(axp202);
	if (ret)
		return ret;

	ret = axp_mfd_add_devices(axp202);
	if (ret) {
		dev_err(axp202->dev, "failed to add MFD devices: %d\n", ret);
		return ret;
	}

	axp202->irq_data = axp_irq_chip_register(axp202->regmap, client->irq,
						IRQF_SHARED
						| IRQF_DISABLED
						| IRQF_NO_SUSPEND,
						&axp202_regmap_irq_chip,
						axp202_wakeup_event);
	if (IS_ERR(axp202->irq_data)) {
		ret = PTR_ERR(axp202->irq_data);
		dev_err(&client->dev, "axp init irq failed: %d\n", ret);
		return ret;
	}

	axp202_pm_power = axp202;

	if (!pm_power_off)
		pm_power_off = axp202_power_off;

	axp_platform_ops_set(axp202->pmu_num, &axp202_plat_ops);

	axp202_ws = wakeup_source_register("axp202_wakeup_source");

	return 0;
}

static int axp202_remove(struct i2c_client *client)
{
	struct axp_dev *axp202 = i2c_get_clientdata(client);

	if (axp202 == axp202_pm_power) {
		axp202_pm_power = NULL;
		pm_power_off = NULL;
	}

	axp_mfd_remove_devices(axp202);
	axp_irq_chip_unregister(client->irq, axp202->irq_data);

	return 0;
}

static struct i2c_driver axp202_driver = {
	.driver = {
		.name   = "axp202",
		.owner  = THIS_MODULE,
		.of_match_table = axp202_dt_ids,
	},
	.probe      = axp202_probe,
	.remove     = axp202_remove,
	.id_table   = axp202_id_table,
};

static int __init axp202_i2c_init(void)
{
	int ret;
	ret = i2c_add_driver(&axp202_driver);
	if (ret != 0)
		pr_err("Failed to register axp202 I2C driver: %d\n", ret);
	return ret;
}
subsys_initcall(axp202_i2c_init);

static void __exit axp202_i2c_exit(void)
{
	i2c_del_driver(&axp202_driver);
}
module_exit(axp202_i2c_exit);

MODULE_DESCRIPTION("PMIC Driver for AXP20X");
MODULE_AUTHOR("Qin <qinyongshen@allwinnertech.com>");
MODULE_LICENSE("GPL");
