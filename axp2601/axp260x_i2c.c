#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <asm/unaligned.h>

#include "GenericTypeDefs.h"
#include "axp260x.h"

static DEFINE_IDR(battery_id);
static DEFINE_MUTEX(battery_mutex);

static irqreturn_t axp260x_irq_handler_thread(int irq, void *data)
{
	struct axp260x_device_info *di = data;
	axp260x_reg_update(di);
	if (di->regcache.irq.wdt){
		axp260x_model_update(di);
		/* inform sys */
	}
	else if (di->regcache.irq.ot){
		/*inform sys */
	}
	else if (di->regcache.irq.lowsoc) {
		/*inform sys */
	}
	else if (di->regcache.irq.newsoc) {
		/*inform sys */
	}
	return IRQ_HANDLED;
}

static int axp260x_i2c_read(struct axp260x_device_info *di, UINT8 regaddr, UINT8 *regdata, UINT8 bytenum)
{
	struct i2c_client *client = to_i2c_client(di->dev);
	struct i2c_msg msg[2];
	int ret;

	if (!client->adapter)
		return -ENODEV;

	msg[0].addr = client->addr;
	msg[0].flags = 0;
	msg[0].buf = &regaddr;
	msg[0].len = sizeof(regaddr);
	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].buf = regdata;
	msg[1].len = bytenum;

	ret = i2c_transfer(client->adapter, msg, ARRAY_SIZE(msg));
	if (ret < 0)
		return ret;
	else return 0;
}

static int axp260x_i2c_write(struct axp260x_device_info *di, UINT8 regaddr, UINT8 *regdata, UINT8 bytenum)
{
	struct i2c_client *client = to_i2c_client(di->dev);
	struct i2c_msg msg;
	UINT8 data[bytenum + 1];
	UINT8 i;
	int ret;

	if (!client->adapter)
		return -ENODEV;
	
	data[0] = regaddr;
	for (i = 1; i <= bytenum; i++)
		data[i] = regdata[i];
	msg.addr = client->addr;
	msg.flags = 0;
	msg.buf = data;
	msg.len = bytenum + 1;\

	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret < 0)
		return ret;
	else return 0;
}

static int axp260x_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct axp260x_device_info *di;
	int ret;
	char *name;
	UINT32 num;
	
	printk(KERN_ALERT "AXP2601  %s - probe start: Client structure is NULL!\n", __func__);
	
	/* get new id for the new battery device */
	mutex_lock(&battery_mutex);
	num = idr_alloc(&battery_id, client, 0, 0, GFP_KERNEL);
	mutex_unlock(&battery_mutex);

	if (num < 0)
		return num;
	name = devm_kasprintf(&client->dev, GFP_KERNEL, "%s-%d", id->name, num);
	if (!name)
		goto err_mem;

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		goto err_mem;
	di->id = num;
	di->dev = &client->dev;
	di->name = name;
	di->chip = id->driver_data;
	
	di->read = axp260x_i2c_read;
	di->write = axp260x_i2c_write;
	
	

	i2c_set_clientdata(client, di);
	if (client->irq){
		ret = devm_request_threaded_irq(&client->dev, client->irq, NULL, 
			axp260x_irq_handler_thread, IRQF_ONESHOT | IRQF_TRIGGER_LOW, di->name, di);
		if (ret){
			dev_err(&client->dev, "Unable to register irq %d, err %d \n", client->irq, ret);
			return ret;
		}
			
	}

	ret = axp260x_setup(di);

	if (ret)
		return ret;

	return 0;
err_mem:

	ret = -ENOMEM;
	mutex_lock(&battery_mutex);
	idr_remove(&battery_id, num);
	mutex_unlock(&battery_mutex);

	return ret;
}


static int axp260x_i2c_remove(struct i2c_client *client)
{
	struct axp260x_device_info *di = i2c_get_clientdata(client);

	axp260x_teardown(di);

	mutex_lock(&battery_mutex);
	idr_remove(&battery_id, di->id);
	mutex_unlock(&battery_mutex);
	
	return 0;
}

static const struct i2c_device_id axp260x_i2c_id_table[] = {
	{"axp2601", AXP2601},
	{"axp2602", AXP2602},
	{},
};
MODULE_DEVICE_TABLE(i2c, axp260x_i2c_id_table);

#ifdef CONFIG_OF
static const struct of_device_id axp260x_i2c_of_match_table[] = {
	{.compatible = "x-powers, axp2601"},
	{.compatible = "x-powers, axp2602"},
	{},
};
MODULE_DEVICE_TABLE(of, axp260x_i2c_of_match_table);
#endif

static struct i2c_driver axp260x_i2c_driver = {
	.driver = {
		.name = "battery_axp260x",
		.of_match_table = of_match_ptr(axp260x_i2c_of_match_table),
	},
	.probe = axp260x_i2c_probe,
	.remove = axp260x_i2c_remove,
	.id_table = axp260x_i2c_id_table,
};

module_i2c_driver(axp260x_i2c_driver);
MODULE_AUTHOR("wangxiaoliang <wangxiaoliang@x-powers.com>");
MODULE_DESCRIPTION("axp260x i2c driver");
MODULE_LICENSE("GPL");
	


