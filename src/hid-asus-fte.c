/*
 *   FocalTech i2c HID TouchPad - FocalTech FTE5436
 *
 *   Copyright (c) 2016 Brendan McGrath <redmcg@redmandi.dyndns.org>
 *   Copyright (c) 2016 Victor Vlasenko <victor.vlasenko@sysgears.com>
 *   Copyright (c) 2016 Frederik Wenigwieser <frederik.wenigwieser@gmail.com>
 */

/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#include <linux/module.h>
#include <linux/hid.h>
#include <linux/input/mt.h>

MODULE_AUTHOR("Brendan McGrath <redmcg@redmandi.dyndns.org>");
MODULE_AUTHOR("Victor Vlasenko <victor.vlasenko@sysgears.com>");
MODULE_DESCRIPTION("ASUS FTE I2C HID TouchPad");
MODULE_LICENSE("GPL");


#define VENDOR_ID 0x0b05
#define DEVICE_ID 0x0101

#define FEATURE_REPORT_ID 0x0d
#define INPUT_REPORT_ID 0x5d

#define INPUT_REPORT_SIZE 28

#define MAX_CONTACTS 5

#define MAX_X 0x0aea
#define MAX_Y 0x06de
#define MAX_TOUCH_MAJOR 8
#define MAX_PRESSURE 0x80

#define CONTACT_SIZE 5

/**
 * struct asus_t - Tracks ASUS FTE I2C HID TouchPad data.
 * @input: Input device through which we report events.
 */
struct asus_t {
	struct input_dev *input;
};

static int asus_raw_event(struct hid_device *hdev, struct hid_report *report, u8 *data, int size)
{
	if (data[0] == INPUT_REPORT_ID && size == INPUT_REPORT_SIZE) {
		int i, contactNum = 0;
		struct asus_t *drvdata = hid_get_drvdata(hdev);
		struct input_dev *input = drvdata->input;

		for (i = 0; i < MAX_CONTACTS; i++) {
			int down = data[1] & (0x08 << i);
			int toolType = data[5 + contactNum*CONTACT_SIZE] & 0x80 ? MT_TOOL_PALM : MT_TOOL_FINGER;
			int x = ((data[2 + contactNum*CONTACT_SIZE] >> 4) << 8) | data[3 + contactNum*CONTACT_SIZE];
			int y = MAX_Y - (((data[2 + contactNum*CONTACT_SIZE] & 0x0f) << 8) | data[4 + contactNum*CONTACT_SIZE]);
			int touch_major = toolType == MT_TOOL_FINGER ? (data[5 + contactNum*CONTACT_SIZE] >> 4) & 0x07 : MAX_TOUCH_MAJOR;
			int pressure = toolType == MT_TOOL_FINGER ? data[6 + contactNum*CONTACT_SIZE] & 0x7f : MAX_PRESSURE;

			input_mt_slot(input, i);
			input_mt_report_slot_state(input, toolType, down);

			if (down) {
				input_report_abs(input, ABS_MT_POSITION_X, x);
				input_report_abs(input, ABS_MT_POSITION_Y, y);
				input_report_abs(input, ABS_MT_TOUCH_MAJOR, touch_major);
				input_report_abs(input, ABS_MT_PRESSURE, pressure);
				contactNum++;
			}
		}

		input_mt_report_pointer_emulation(input, true);
		input_report_key(input, BTN_LEFT, data[1] & 1);

		input_sync(input);
		return 1;
	}

	return 0;
}

static int asus_setup_input(struct hid_device *hdev, struct input_dev *input)
{
	int ret = input_mt_init_slots(input, MAX_CONTACTS, 0);
	struct asus_t *drvdata = hid_get_drvdata(hdev);

	if (ret) {
		hid_err(hdev, "ASUS FTE input mt init slots failed: %d\n", ret);
		return ret;
	}

	drvdata->input = input;

	__set_bit(EV_KEY, input->evbit);
	__set_bit(BTN_LEFT, input->keybit);
	__set_bit(BTN_TOOL_FINGER, input->keybit);
	__set_bit(BTN_TOUCH, input->keybit);
	__set_bit(BTN_TOOL_DOUBLETAP, input->keybit);
	__set_bit(BTN_TOOL_TRIPLETAP, input->keybit);
	__set_bit(BTN_TOOL_QUADTAP, input->keybit);


	input_set_abs_params(input, ABS_X, 0, MAX_X, 0, 0);

	input_set_abs_params(input, ABS_Y, 0, MAX_Y, 0, 0);

	input_set_abs_params(input, ABS_MT_POSITION_X, 0, MAX_X, 0, 0);

	input_set_abs_params(input, ABS_MT_POSITION_Y, 0, MAX_Y, 0, 0);

	input_set_abs_params(input, ABS_MT_TOOL_TYPE, 0, MT_TOOL_MAX, 0, 0);

	input_set_abs_params(input, ABS_TOOL_WIDTH, 0, MAX_TOUCH_MAJOR, 0, 0);

	input_set_abs_params(input, ABS_MT_TOUCH_MAJOR, 0, MAX_TOUCH_MAJOR, 0, 0);

	input_set_abs_params(input, ABS_PRESSURE, 0, MAX_PRESSURE, 0, 0);

	input_set_abs_params(input, ABS_MT_PRESSURE, 0, MAX_PRESSURE, 0, 0);

	__set_bit(INPUT_PROP_POINTER, input->propbit);
	__set_bit(INPUT_PROP_BUTTONPAD, input->propbit);

	return 0;
}

static int asus_input_configured(struct hid_device *hdev, struct hid_input *hi)
{
	int ret = asus_setup_input(hdev, hi->input);
	if (ret)
		return ret;

	return 0;
}

static int asus_input_mapping(struct hid_device *hdev,
				struct hid_input *hi, struct hid_field *field,
				struct hid_usage *usage, unsigned long **bit, int *max)
{
	/* Don't map anything from the HID report. We do it all manually in asus_setup_input */
	return -1;
}

static int start_multitouch(struct hid_device *hdev) {
	unsigned char buf[] = { FEATURE_REPORT_ID, 0x00, 0x03, 0x01, 0x00 };
	int ret = hid_hw_raw_request(hdev, FEATURE_REPORT_ID, buf, sizeof(buf), HID_FEATURE_REPORT, HID_REQ_SET_REPORT);
	if (ret != sizeof(buf)) {
		hid_err(hdev, "Failed to start multitouch: %d\n", ret);
		return ret;
	}

	return 0;
}

#ifdef CONFIG_PM
static int asus_resume(struct hid_device *hdev) {
	return start_multitouch(hdev);
}
#endif

static int asus_probe(struct hid_device *hdev, const struct hid_device_id *id)
{
	int ret;
	struct hid_report *report;
	struct asus_t *drvdata;

	drvdata = devm_kzalloc(&hdev->dev, sizeof(*drvdata), GFP_KERNEL);
	if (drvdata == NULL) {
		hid_err(hdev, "Can't alloc ASUS FTE descriptor\n");
		return -ENOMEM;
	}

	hid_set_drvdata(hdev, drvdata);

	hdev->quirks = HID_QUIRK_NO_INIT_REPORTS;

	ret = hid_parse(hdev);
	if (ret) {
		hid_err(hdev, "ASUS FTE hid parse failed: %d\n", ret);
		return ret;
	}

	ret = hid_hw_start(hdev, HID_CONNECT_DEFAULT);
	if (ret) {
		hid_err(hdev, "ASUS FTE hw start failed: %d\n", ret);
		return ret;
	}

	if (!drvdata->input) {
		hid_err(hdev, "ASUS FTE input not registered\n");
		ret = -ENOMEM;
		goto err_stop_hw;
	}

	drvdata->input->name = "Asus FTE TouchPad";

	report = hid_register_report(hdev, HID_INPUT_REPORT, INPUT_REPORT_ID);

	if (!report) {
		hid_err(hdev, "Unable to register input report\n");
		ret = -ENOMEM;
		goto err_stop_hw;
	}

	ret = start_multitouch(hdev);
	if (ret)
		goto err_stop_hw;

	return 0;
err_stop_hw:
	hid_hw_stop(hdev);
	return ret;
}

static const struct hid_device_id asus_touchpad[] = {
	{ HID_I2C_DEVICE(VENDOR_ID,
		DEVICE_ID), .driver_data = 0 },
	{ }
};
MODULE_DEVICE_TABLE(hid, asus_touchpad);

static struct hid_driver asus_driver = {
	.name = "hid-asus-fte",
	.id_table = asus_touchpad,
	.probe = asus_probe,
	.input_mapping = asus_input_mapping,
	.input_configured = asus_input_configured,
#ifdef CONFIG_PM
	.resume			= asus_resume,
	.reset_resume		= asus_resume,
#endif
	.raw_event = asus_raw_event
};

module_hid_driver(asus_driver);
