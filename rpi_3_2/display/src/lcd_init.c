#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/gpio.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/device/class.h>
#include "../include/lcd_init.h"
#include "../include/user_api.h"
#include "../include/user_api.h"
#include "../include/fonts.h"

#define debug_tag "RPI LCD DEBUG: "
#define DATA_SIZE 90
#define PIXEL_SIZE sizeof(pixel_t)
#define SINGLEDEVICE 0
#define DEVICENUM 1
#define MODNAME "lcd_display_mod"
#define DEVNAME "display"
#define DEVCLASS "rpi_lcd"

//#define DEBUG

MODULE_LICENSE("GPL");
MODULE_VERSION("1.0.0");
MODULE_AUTHOR("Maksym Bilyk <makkusu.shiroi@gmail.com>");
MODULE_DESCRIPTION("RPI 3.2 LCD Display Driver");

#define count_data(...)                                                        \
	{                                                                      \
		sizeof((u8[]){ __VA_ARGS__ }), __VA_ARGS__                     \
	}

static struct spi_device *rpi_lcd_spi_device;
static struct window curw = {
	.x0 = 0,
	.x1 = LCD_WIDTH - 1,
	.y0 = 0,
	.y1 = LCD_HEIGHT - 1,
	.w = LCD_WIDTH,
	.h = LCD_HEIGHT,
};

static lcd_frame frame = {
	.size = LCD_HEIGHT * LCD_WIDTH,
	.n_rows = LCD_HEIGHT,
	.n_cols = LCD_WIDTH,
};

enum lcd_commands lcd_command = NONE;

static int major;
static ssize_t data_write(struct file *file, const char *buf, size_t count,
			  loff_t *ppos);
static struct cdev hcdev;
static struct class *devclass;
static const struct file_operations dev_fops = {
	.owner = THIS_MODULE,
	.write = data_write,
};

int dev_init(void)
{
	int ret = 0;
	dev_t dev;
	ret = alloc_chrdev_region(&dev, SINGLEDEVICE, DEVICENUM, MODNAME);
	major = MAJOR(dev);
	if (ret < 0) {
		pr_info("Failed to register char device region\n");
		goto end;
	}
	cdev_init(&hcdev, &dev_fops);
	hcdev.owner = THIS_MODULE;
	ret = cdev_add(&hcdev, dev, DEVICENUM);
	if (ret < 0) {
		unregister_chrdev_region(MKDEV(major, SINGLEDEVICE), DEVICENUM);
		pr_info("=== Can not add char device\n");
		goto end;
	}
	devclass = class_create(THIS_MODULE, DEVCLASS);
	dev = MKDEV(major, SINGLEDEVICE);
	device_create(devclass, NULL, dev, NULL, "%s", DEVNAME);
	pr_info("======== device installed %d:[%d-%d] ===========\n",
		MAJOR(dev), SINGLEDEVICE, MINOR(dev));
end:
	return ret;
}

void dev_exit(void)
{
	dev_t dev;
	dev = MKDEV(major, SINGLEDEVICE);
	device_destroy(devclass, dev);
	class_destroy(devclass);
	cdev_del(&hcdev);
	unregister_chrdev_region(MKDEV(major, SINGLEDEVICE), DEVICENUM);
	pr_info("=============== module removed ==================\n");
}

static void rpi_lcd_write_command(u8 cmd)
{
	gpio_set_value(LCD_PIN_DC, 0);
	spi_write(rpi_lcd_spi_device, &cmd, sizeof(cmd));
}

inline static void rpi_lcd_reset(void)
{
	gpio_set_value(LCD_PIN_RESET, 0);
	gpio_set_value(LCD_PIN_RESET, 1);
}

static void rpi_lcd_write_data(u8 *buff, size_t buff_size)
{
	size_t i = 0;
	gpio_set_value(LCD_PIN_DC, 1);
	while (buff_size > DATA_SIZE) {
		spi_write(rpi_lcd_spi_device, buff + i, DATA_SIZE);
		i += DATA_SIZE;
		buff_size -= DATA_SIZE;
	}
	spi_write(rpi_lcd_spi_device, buff + i, buff_size);
}

void rpi_lcd_fast_update(void)
{
	size_t row;
	for (row = curw.y0; row <= curw.y1; ++row) {
		memcpy(frame.unraveled + (row - curw.y0) * curw.w,
		       frame.buffer + row * frame.n_cols + curw.x0,
		       curw.w * PIXEL_SIZE);
	}

	pr_info("w: %u, h: %u\n", curw.w, curw.h);
	rpi_lcd_write_data((u8 *)frame.unraveled, curw.w * curw.h * PIXEL_SIZE);
}

void rpi_lcd_fill_rectangle(u16 x, u16 y, u16 w, u16 h, u16 color)
{
	u16 j;
	u16 k;

	if ((x >= LCD_WIDTH) || (y >= LCD_HEIGHT)) {
		return;
	}

	if ((x + w - 1) > LCD_WIDTH) {
		w = LCD_WIDTH - x;
	}

	if ((y + h - 1) > LCD_HEIGHT) {
		h = LCD_HEIGHT - y;
	}

	for (j = 0; j < h; j++) {
		for (k = 0; k < w; k++) {
			frame.buffer[(x + LCD_WIDTH * y) + (k + LCD_WIDTH * j)] =
				(color >> 8) | (color << 8);
		}
	}
}

void rpi_lcd_fill_screen(u16 color)
{
	rpi_lcd_fill_rectangle(0, 0, LCD_WIDTH, LCD_HEIGHT, color);
}

void rpi_lcd_set_address_window(u16 x0, u16 y0, u16 w, u16 h)
{
	curw.x0 = x0;
	curw.x1 = x0 + w - 1;
	curw.y0 = y0;
	curw.y1 = y0 + h - 1;
	curw.w = w;
	curw.h = h;

	rpi_lcd_write_command(LCD_CASET);
	{
		uint8_t data[] = { (curw.x0 >> 8) & 0xFF, curw.x0 & 0xFF,
				   (curw.x1 >> 8) & 0xFF, curw.x1 & 0xFF };
		rpi_lcd_write_data(data, sizeof(data));
	}

	rpi_lcd_write_command(LCD_RASET);
	{
		uint8_t data[] = { (curw.y0 >> 8) & 0xFF, curw.y0 & 0xFF,
				   (curw.y1 >> 8) & 0xFF, curw.y1 & 0xFF };
		rpi_lcd_write_data(data, sizeof(data));
	}

	rpi_lcd_write_command(LCD_RAMWR);
}

static bool is_full_screen(struct window *w)
{
	return w->x0 == 0 && w->y0 == 0 && w->w == LCD_WIDTH &&
	       w->h == LCD_HEIGHT;
}

void swap_full_screen(void)
{
	static struct window swapw = { .x0 = 0,
				       .x1 = LCD_WIDTH - 1,
				       .y0 = 0,
				       .y1 = LCD_HEIGHT - 1,
				       .w = LCD_WIDTH,
				       .h = LCD_HEIGHT };
	struct window tempw;

	if (is_full_screen(&curw) && is_full_screen(&swapw))
		return;

	tempw = curw;
	curw = swapw;
	swapw = tempw;
	rpi_lcd_set_address_window(curw.x0, curw.y0, curw.w, curw.h);
}

void rpi_lcd_full_update(void)
{
	swap_full_screen();
	rpi_lcd_write_data((u8 *)frame.buffer, frame.size * PIXEL_SIZE);
	swap_full_screen();
}

void rpi_lcd_put_char(u16 x, u16 y, char ch, FontDef font, u16 color,
		      u16 bgcolor)
{
	u32 i, b, j;
	for (i = 0; i < font.height; i++) {
		b = font.data[(ch - 32) * font.height + i];
		for (j = 0; j < font.width; j++) {
			if ((b << j) & 0x8000) {
				frame.buffer[(x + LCD_WIDTH * y) +
					     (j + LCD_WIDTH * i)] =
					(color >> 8) | (color << 8);
			} else {
				frame.buffer[(x + LCD_WIDTH * y) +
					     (j + LCD_WIDTH * i)] =
					(bgcolor >> 8) | (bgcolor << 8);
			}
		}
	}
}

void rpi_lcd_put_str(u16 x, u16 y, const char *str, FontDef font, u16 color,
		     u16 bgcolor)
{
	while (*str) {
		if (x + font.width >= LCD_WIDTH) {
			x = 0;
			y += font.height;
			if (y + font.height >= LCD_HEIGHT) {
				break;
			}

			if (*str == ' ') {
				// skip spaces in the beginning of the new line
				str++;
				continue;
			}
		}
		rpi_lcd_put_char(x, y, *str, font, color, bgcolor);
		x += font.width;
		str++;
	}
}

static ssize_t data_write(struct file *file, const char *buf, size_t count,
			  loff_t *ppos)
{
	int res;
	int command = -1;
	u16 x0, y0, w, h, color;
	char command_buff[100] = { '\0' };
	copy_from_user(command_buff, buf, count);

	pr_info("command:: %s\n", command_buff);
	sscanf(command_buff, "%d:", &command);

	switch (command) {
	case DRAW_RECTANGLE: {
		pr_info("DRAW_RECTANGLE\n");
		res = sscanf(command_buff, "%d:%hu|%hu|%hu|%hu|%hu", &command,
			     &x0, &y0, &w, &h, &color);
		pr_info("%s[]%d\n", command_buff, res);
		if (res == 5 + 1)
			pr_info("x0: %hu, y0: %hu, w: %hu, h: %hu, c: %hu", x0,
				y0, w, h, color);
		rpi_lcd_fill_rectangle(x0, y0, w, h, color);
		break;
	}
	case DRAW_BALL: {
		pr_info("DRAW_BALL\n");
		res = sscanf(command_buff, "%d:%hu|%hu|%hu", &command, &x0, &y0,
			     &color);
		if (res == 3 + 1) {
			rpi_lcd_put_char(x0, y0, ' ', Ball_16x16, color,
					 COLOR_BLACK);
		}
		break;
	}

	case SET_WINDOW: {
		pr_info("SET_WINDOW\n");
		res = sscanf(command_buff, "%d:%hu|%hu|%hu|%hu", &command, &x0,
			     &y0, &w, &h);
		if (res == 4 + 1)
			rpi_lcd_set_address_window(x0, y0, w, h);
		break;
	}

	case RENDER: {
		pr_info("RENDER\n");
		res = sscanf(command_buff, "%d:%hu", &command, &x0);
		if (res == 1 + 1) {
			if (x0 == 0)
				rpi_lcd_full_update();
			else
				rpi_lcd_fast_update();
		}
		break;
	}
	default:
		pr_info("Wrong command  %d\n", lcd_command);
	}
	return count;
}

static init_step software_reset = { .cmd = 0x01,
				    .delay = 1000,
				    .data = count_data() };

static init_step power_control_a = { .cmd = 0xCB,
				     .delay = 0,
				     .data = count_data(0x39, 0x2C, 0x00, 0x34,
							0x02) };

static init_step power_control_b = { .cmd = 0xCF,
				     .delay = 0,
				     .data = count_data(0x00, 0xC1, 0x30) };

static init_step driver_timing_control_a = { .cmd = 0xE8,
					     .delay = 0,
					     .data = count_data(0x85, 0x00,
								0x78) };

static init_step driver_timing_control_b = { .cmd = 0xEA,
					     .delay = 0,
					     .data = count_data(0x00, 0x00) };

static init_step power_on_sequence_control = { .cmd = 0xED,
					       .delay = 0,
					       .data = count_data(0x64, 0x03,
								  0x12, 0x81) };

static init_step pump_ratio_control = { .cmd = 0xF7,
					.delay = 0,
					.data = count_data(0x20) };

static init_step power_control_vrh = { .cmd = 0xC0,
				       .delay = 0,
				       .data = count_data(0x23) };

static init_step power_control_sap = { .cmd = 0xC1,
				       .delay = 0,
				       .data = count_data(0x10) };

static init_step vcm_control = { .cmd = 0xC5,
				 .delay = 0,
				 .data = count_data(0x3E, 0x28) };

static init_step vcm_control_2 = { .cmd = 0xC7,
				   .delay = 0,
				   .data = count_data(0x86) };

static init_step pixel_format = { .cmd = 0x3A,
				  .delay = 0,
				  .data = count_data(0x55) };

static init_step frame_ratio_control_standard_rgb_color = {
	.cmd = 0xB1,
	.delay = 0,
	.data = count_data(0x00, 0x18)
};

static init_step display_function_control = { .cmd = 0xB6,
					      .delay = 0,
					      .data = count_data(0x08, 0x82,
								 0x27) };

static init_step gamma_function_disable = { .cmd = 0xF2,
					    .delay = 0,
					    .data = count_data(0x00) };

static init_step gamma_curve_selected = { .cmd = 0x26,
					  .delay = 0,
					  .data = count_data(0x01) };

static init_step positive_gamma_correction = {
	.cmd = 0xE0,
	.delay = 0,
	.data = count_data(0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1, 0x37,
			   0x07, 0x10, 0x03, 0x0E, 0x09, 0x00)
};

static init_step negative_gamma_correction = {
	.cmd = 0xE1,
	.delay = 0,
	.data = count_data(0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1, 0x48,
			   0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F)
};

static init_step exit_sleep = { .cmd = 0x11,
				.delay = 120,
				.data = count_data() };

static init_step turn_on_display = { .cmd = 0x29,
				     .delay = 0,
				     .data = count_data() };

static init_step memory_access_control = { .cmd = 0x36,
					   .delay = 0,
					   .data = count_data(0x28) };

static init_step *init_seq[] = { &software_reset,
				 &power_control_a,
				 &power_control_b,
				 &driver_timing_control_a,
				 &driver_timing_control_b,
				 &power_on_sequence_control,
				 &pump_ratio_control,
				 &power_control_vrh,
				 &power_control_sap,
				 &vcm_control,
				 &vcm_control_2,
				 &pixel_format,
				 &frame_ratio_control_standard_rgb_color,
				 &display_function_control,
				 &gamma_function_disable,
				 &gamma_curve_selected,
				 &positive_gamma_correction,
				 &negative_gamma_correction,
				 &exit_sleep,
				 &turn_on_display,
				 &memory_access_control };

static const u8 CMD_NUM = sizeof(init_seq) / sizeof(init_step *);

static void rpi_lcd_display_init(void)
{
	u8 seq_ind = 0;
	u8 data_ind = 0;
	u8 data_num = 0;

	for (seq_ind = 0; seq_ind < CMD_NUM; ++seq_ind) {
		data_num = init_seq[seq_ind]->data[0];
		rpi_lcd_write_command(init_seq[seq_ind]->cmd);
		if (data_num)
			rpi_lcd_write_data(&init_seq[seq_ind]->data[1],
					   data_num);
		if (init_seq[seq_ind]->delay)
			mdelay(init_seq[seq_ind]->delay);
#ifdef DEBUG
		pr_info("%u. CMD: %x\n", seq_ind + 1, init_seq[seq_ind]->cmd);
		for (data_ind = 1; data_ind < data_num + 1; ++data_ind)
			pr_info("\t%u. DATA: %x\n", data_ind,
				init_seq[seq_ind]->data[data_ind]);
#endif
	}
}

static int rpi_lcd_spi_init(void)
{
	int ret = 0;
	struct spi_master *master = NULL;

	struct spi_board_info lcd_info = {
		.modalias = "LCD",
		.max_speed_hz = 50e6,
		.bus_num = 0,
		.chip_select = 0,
		.mode = SPI_MODE_0,
	};

	if (!(master = spi_busnum_to_master(lcd_info.bus_num))) {
		pr_err("MASTER not found.\n");
		ret = -ENODEV;
		goto end;
	}

	if (!(rpi_lcd_spi_device = spi_new_device(master, &lcd_info))) {
		pr_err("FAILED to create slave.\n");
		ret = -ENODEV;
		goto end;
	}

	rpi_lcd_spi_device->bits_per_word = 8;
	if ((ret = spi_setup(rpi_lcd_spi_device))) {
		pr_err("FAILED to setup slave.\n");
		spi_unregister_device(rpi_lcd_spi_device);
		ret = -ENODEV;
		goto end;
	}

	pr_info("LCD: spi device setup completed\n");
end:
	return ret;
}

static void rpi_lcd_gpio_init(void)
{
	gpio_request(LCD_PIN_RESET, "LCD_PIN_RESET");
	gpio_direction_output(LCD_PIN_RESET, 0);
	gpio_request(LCD_PIN_DC, "LCD_PIN_DC");
	gpio_direction_output(LCD_PIN_DC, 0);
	rpi_lcd_reset();

	pr_info("GPIO: PINS initialized\n");
}

static void rpi_lcd_gpio_free(void)
{
	gpio_free(LCD_PIN_DC);
	gpio_free(LCD_PIN_RESET);
}

static void rpi_lcd_spi_free(void)
{
	if (rpi_lcd_spi_device)
		spi_unregister_device(rpi_lcd_spi_device);
}

int init_module(void)
{
	int ret = 0;

	if ((ret = rpi_lcd_spi_init()))
		goto end;
	if ((ret = dev_init()))
		goto end;
	rpi_lcd_gpio_init();
	rpi_lcd_display_init();

	rpi_lcd_set_address_window(0, 0, LCD_WIDTH, LCD_HEIGHT);
	rpi_lcd_fill_screen(COLOR_RED);
	rpi_lcd_full_update();

	pr_info(debug_tag "Module initialized successfully\n");
end:
	return ret;
}

void cleanup_module(void)
{
	rpi_lcd_fill_screen(COLOR_WHITE);
	rpi_lcd_full_update();
	dev_exit();
	rpi_lcd_gpio_free();
	rpi_lcd_spi_free();
	pr_info(debug_tag "Module cleaned-up\n");
}