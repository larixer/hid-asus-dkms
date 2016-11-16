/*
 *   FocalTech i2c HID TouchPad - FocalTech FTE5436
 *
 *   Copyright (c) 2016 Brendan McGrath <redmcg@redmandi.dyndns.org>
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
MODULE_DESCRIPTION("FocalTech TouchPad");
MODULE_LICENSE("GPL");


#define VENDOR_ID 0x0b05
#define DEVICE_ID 0x0101

#define FEATURE_REPORT_ID 0x0d
#define INPUT_REPORT_ID 0x5d

#define INPUT_REPORT_SIZE 27

#define MAX_CONTACTS 5

#define MAX_X 0x0aea
#define MAX_Y 0x06de
#define MAX_TOUCH_MAJOR 8
#define MAX_PRESSURE 0x80

#define CONTACT_SIZE 5

/**
 * struct focaltech_t - Tracks FocalTech TouchPad data.
 * @input: Input device through which we report events.
 */
struct focaltech_t {
		struct input_dev *input;
};

static int focaltech_raw_event(struct hid_device *hdev,
				struct hid_report *report, u8 *data, int size)
{
	int i, contactNum = 0;
	struct focaltech_t *ft = hid_get_drvdata(hdev);
	struct input_dev *input = ft->input;

	for (i = 0; i < MAX_CONTACTS; i++) {
		int down = data[1] & (0x08 << i);
		int toolType = data[5 + contactNum*CONTACT_SIZE] & 0x80 ? MT_TOOL_PALM : MT_TOOL_FINGER;
		int x = ((data[2 + contactNum*CONTACT_SIZE] >> 4) << 8) | data[3 + contactNum*CONTACT_SIZE];
		int y = MAX_Y - (((data[2 + contactNum*CONTACT_SIZE] & 0x0f) << 8) | data[4 + contactNum*CONTACT_SIZE]);
		int touch_major = toolType == MT_TOOL_FINGER ? (data[5 + contactNum*CONTACT_SIZE] >> 4) & 0x07 : MAX_TOUCH_MAJOR;
		int pressure = toolType == MT_TOOL_FINGER ? data[6 + contactNum*CONTACT_SIZE] & 0x7f : MAX_PRESSURE;

		input_mt_slot(input, i);
		input_mt_report_slot_state(input, toolType, down);

		input_report_abs(input, ABS_MT_POSITION_X, x);
		input_report_abs(input, ABS_MT_POSITION_Y, y);
		input_report_abs(input, ABS_MT_TOUCH_MAJOR, touch_major);
		input_report_abs(input, ABS_MT_PRESSURE, pressure);

		if (down)
			contactNum++;
	}

	input_mt_report_pointer_emulation(input, true);
	input_report_key(input, BTN_LEFT, data[1] & 1);

	input_sync(input);
	return 1;
}

static int focaltech_setup_input(struct hid_device *hdev, struct input_dev *input)
{
	int ret = input_mt_init_slots(input, MAX_CONTACTS, 0);
	struct focaltech_t *ft = hid_get_drvdata(hdev);

	if (ret) {
		hid_err(hdev, "focaltech input mt init slots failed: %d\n", ret);
		return ret;
	}

	ft->input = input;

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

static int focaltech_input_configured(struct hid_device *hdev,
				struct hid_input *hi)
{
	int ret = focaltech_setup_input(hdev, hi->input);
	if (ret)
		return ret;

	return 0;
}

static int focaltech_input_mapping(struct hid_device *hdev,
				struct hid_input *hi, struct hid_field *field,
				struct hid_usage *usage, unsigned long **bit, int *max)
{
	/* TrackPad does not give relative data after switching to MT */
	if (field->flags & HID_MAIN_ITEM_RELATIVE)
		return -1;

	return 0;
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

static int focaltech_probe(struct hid_device *hdev,
		const struct hid_device_id *id)
{
	int ret;
	struct hid_report *report;
	struct focaltech_t *ft;

	ft = devm_kzalloc(&hdev->dev, sizeof(*ft), GFP_KERNEL);
	if (ft == NULL) {
		hid_err(hdev, "can't alloc focaltech descriptor\n");
		return -ENOMEM;
	}

	hid_set_drvdata(hdev, ft);

	hdev->quirks = HID_QUIRK_NO_INIT_REPORTS;

	ret = hid_parse(hdev);
	if (ret) {
		hid_err(hdev, "focaltech hid parse failed: %d\n", ret);
		return ret;
	}

	ret = hid_hw_start(hdev, HID_CONNECT_DEFAULT);
	if (ret) {
		hid_err(hdev, "focaltech hw start failed: %d\n", ret);
		return ret;
	}

	if (!ft->input) {
		hid_err(hdev, "focaltech input not registered\n");
		ret = -ENOMEM;
		goto err_stop_hw;
	}

	report = hid_register_report(hdev, HID_INPUT_REPORT, INPUT_REPORT_ID);

	if (!report) {
		hid_err(hdev, "unable to register input report\n");
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

static const struct hid_device_id focaltech_touchpad[] = {
	{ HID_I2C_DEVICE(VENDOR_ID,
		DEVICE_ID), .driver_data = 0 },
	{ }
};
MODULE_DEVICE_TABLE(hid, focaltech_touchpad);

static struct hid_driver focaltech_driver = {
	.name = "hid-focaltech",
	.id_table = focaltech_touchpad,
	.probe = focaltech_probe,
	.input_mapping = focaltech_input_mapping,
	.input_configured = focaltech_input_configured,
	.raw_event = focaltech_raw_event
};

module_hid_driver(focaltech_driver);
