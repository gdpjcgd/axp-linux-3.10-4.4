#include "axp15060-gpio.h"

#include <linux/io.h>
#include <linux/clk.h>
#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/pinctrl/consumer.h>
#include <linux/pinctrl/machine.h>
#include <linux/pinctrl/pinctrl.h>
#include <linux/pinctrl/pinconf-generic.h>
#include <linux/pinctrl/pinmux.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include "../sunxi-gpio.h"
#include <linux/mfd/core.h>
#include <linux/seq_file.h>
#include <linux/i2c.h>
#include "../../../pinctrl/core.h"

#include "../axp-core.h"
#include "../axp-gpio.h"

static int axp15060_gpio_get_data(struct axp_dev *axp_dev, int gpio)
{
	u8 ret;
	struct axp_regmap *map;

	map = axp_dev->regmap;
	if (NULL == map) {
		pr_err("%s: axp regmap is null\n", __func__);
		return -ENXIO;
	}

	switch (gpio) {
	case 1:
		axp_regmap_read(map, AXP_GPIO1_CFG, &ret); ret &= 0x80; break;
	case 2:
	default:
		pr_err("This IO is not an input\n");
		return -ENXIO;
	}

	return ret;
}

static int axp15060_gpio_set_data(struct axp_dev *axp_dev, int gpio, int value)
{
	struct axp_regmap *map;

	if ((0 < gpio) && (3 > gpio)) {
		map = axp_dev->regmap;
		if (NULL == map) {
			pr_err("%s: %d axp regmap is null\n",
					__func__, __LINE__);
			return -ENXIO;
		}
	} else {
		return -ENXIO;
	}

	if (value) {
		/* high */
		switch (gpio) {
		case 1:
			return axp_regmap_update_sync(map,
				AXP_GPIO1_CFG, 0x40, 0x60);
		case 2:
			return axp_regmap_update_sync(map,
				AXP_GPIO2_CFG, 0x02, 0x07);
		default:
			break;
		}
	} else {
		/* low */
		switch (gpio) {
		case 1:
			return axp_regmap_update_sync(map,
				AXP_GPIO1_CFG, 0x20, 0x60);
		case 2:
			return axp_regmap_update_sync(map,
				AXP_GPIO2_CFG, 0x01, 0x07);
		default:
			break;
		}
	}

	return -ENXIO;
}

static int axp15060_pmx_set(struct axp_dev *axp_dev, int gpio, int mux)
{
	struct axp_regmap *map;

	map = axp_dev->regmap;
	if (NULL == map) {
		pr_err("%s: %d axp regmap is null\n", __func__, __LINE__);
		return -ENXIO;
	}

	if (mux == 1) {
		/* output */
		switch (gpio) {
		case 1:
			return axp_regmap_update_sync(map,
				AXP_GPIO1_CFG, 0x20, 0x60);
		case 2:
			return axp_regmap_update_sync(map,
				AXP_GPIO2_CFG, 0x02, 0x07);
		default:
			return -ENXIO;
		}
	} else if (mux == 0) {
		/* input */
		switch (gpio) {
		case 1:
			return axp_regmap_update_sync(map,
				AXP_GPIO1_CFG, 0x60, 0x60);
		case 2:
			return axp_regmap_update_sync(map,
				AXP_GPIO2_CFG, 0x03, 0x07);
		default:
			pr_err("This IO can not config as input!");
			return -ENXIO;
		}
	}

	return -ENXIO;
}

static int axp15060_pmx_get(struct axp_dev *axp_dev, int gpio)
{
	u8 ret;
	struct axp_regmap *map;

	map = axp_dev->regmap;
	if (NULL == map) {
		pr_err("%s: %d axp regmap is null\n",
				__func__, __LINE__);
		return -ENXIO;
	}

	switch (gpio) {
	case 1:
		axp_regmap_read(map, AXP_GPIO1_CFG, &ret);
		if (0x40 == (ret & 0x60))
			return 1;
		else if (0x60 == (ret & 0x60))
			return 0;
		else
			return -ENXIO;
	case 2:
		axp_regmap_read(map, AXP_GPIO2_CFG, &ret);
		if (0x02 == (ret & 0x07))
			return 1;
		else if (0x03 == (ret & 0x07))
			return 0;
		else
			return -ENXIO;
	default:
		return -ENXIO;
	}
}

static const struct axp_desc_pin axp15060_pins[] = {
	AXP_PIN_DESC(AXP_PINCTRL_GPIO(1),
			 AXP_FUNCTION(0x0, "gpio_in"),
			 AXP_FUNCTION(0x1, "gpio_out")),
	AXP_PIN_DESC(AXP_PINCTRL_GPIO(2),
			 AXP_FUNCTION(0x0, "gpio_in"),
			 AXP_FUNCTION(0x1, "gpio_out")),
};

static struct axp_pinctrl_desc axp15060_pinctrl_pins_desc = {
	.pins  = axp15060_pins,
	.npins = ARRAY_SIZE(axp15060_pins),
};

static struct axp_gpio_ops axp15060_gpio_ops = {
	.gpio_get_data = axp15060_gpio_get_data,
	.gpio_set_data = axp15060_gpio_set_data,
	.pmx_set = axp15060_pmx_set,
	.pmx_get = axp15060_pmx_get,
};

static int axp15060_gpio_probe(struct platform_device *pdev)
{

	struct axp_dev *axp_dev = dev_get_drvdata(pdev->dev.parent);
	struct axp_pinctrl *axp15060_pin;

	axp15060_pin = axp_pinctrl_register(&pdev->dev,
				axp_dev, &axp15060_pinctrl_pins_desc,
				&axp15060_gpio_ops);
	if (IS_ERR_OR_NULL(axp15060_pin))
		goto fail;

	platform_set_drvdata(pdev, axp15060_pin);

	return 0;

fail:
	return -1;
}

static int axp15060_gpio_remove(struct platform_device *pdev)
{
	struct axp_pinctrl *axp15060_pin = platform_get_drvdata(pdev);
	return axp_pinctrl_unregister(axp15060_pin);
}

static const struct of_device_id axp15060_gpio_dt_ids[] = {
	{ .compatible = "axp15060-gpio", },
	{},
};
MODULE_DEVICE_TABLE(of, axp15060_gpio_dt_ids);

static struct platform_driver axp15060_gpio_driver = {
	.driver = {
		.name = "axp15060-gpio",
		.of_match_table = axp15060_gpio_dt_ids,
	},
	.probe  = axp15060_gpio_probe,
	.remove = axp15060_gpio_remove,
};

static int __init axp15060_gpio_initcall(void)
{
	int ret;

	ret = platform_driver_register(&axp15060_gpio_driver);
	if (IS_ERR_VALUE(ret)) {
		pr_err("%s: failed, errno %d\n", __func__, ret);
		return -EINVAL;
	}

	return 0;
}
fs_initcall(axp15060_gpio_initcall);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("gpio Driver for axp15060");
MODULE_AUTHOR("Qin <qinyongshen@allwinnertech.com>");
