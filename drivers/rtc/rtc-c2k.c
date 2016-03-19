/*
 * drivers/rtc/rtc-c2k.c
 *
 * Copyright (c) 2010 Mindspeed Technologies Co., Ltd.
 *		http://www.mindspeed.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Copyright 2016
 * Hans Uli Kroll <ulli.krollgooglemail.com
 *
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/rtc.h>
#include <linux/bcd.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/gfp.h>
#include <linux/module.h>


/*
	REGISTER OFFSET_ADDRESS R/W DESCRIPTION RESET VALUE
*/

#define	C2K_RTC_RTCCON 0X00 /* R/W RTC CONTROL REGISTER 0X00 */
#define	C2K_RTC_RTCALM 0X04 /* R/W RTC ALARM CONTROL REGISTER 0X00 */
#define	C2K_RTC_ALMSEC 0X08 /* R/W ALARM SECOND DATA REGISTER 0X00 */
#define	C2K_RTC_ALMMIN 0X0C /* R/W ALARM MINUTE DATA REGISTER 0X00 */
#define	C2K_RTC_ALMHOUR 0X10 /* R/W ALARM HOUR DATA REGISTER 0X00 */
#define	C2K_RTC_ALMDATE 0X14 /* R/W ALARM DATE DATA REGISTER 0X00 */
#define	C2K_RTC_ALMDAY 0X18 /* R/W ALARM DAY OF WEEK DATA REGISTER 0X00 */
#define	C2K_RTC_ALMMON 0X1C /* R/W ALARM MONTH DATA REGISTER 0X00 */
#define	C2K_RTC_ALMYEAR 0X20 /* R/W ALARM YEAR DATA REGISTER 0X00 */
#define	C2K_RTC_BCDSEC 0X24 /* R/W BCD SECOND REGISTER */
#define	C2K_RTC_BCDMIN 0X28 /* R/W BCD MINUTE REGISTER */
#define	C2K_RTC_BCDHOUR 0X2C /* R/W BCD HOUR REGISTER */
#define	C2K_RTC_BCDDATE 0X30 /* R/W BCD DATE REGISTER */
#define	C2K_RTC_BCDDAY 0X34 /* R/W BCD DAY OF WEEK REGISTER */
#define	C2K_RTC_BCDMON 0X38 /* R/W BCD MONTH REGISTER */
#define	C2K_RTC_BCDYEAR 0X3C /* R/W BCD YEAR REGISTER */
#define	C2K_RTC_RTCIM 0X40 /* R/W RTC INTERRUPT MODE REGISTER 0X00 */
#define	C2K_RTC_RTCPEND 0X44 /* R/W RTC INTERRUPT PENDING REGISTER 0X00 */


/* Initial by default values */
#define	YEAR		112 /* 2012 */
#define	MONTH		6 /* 6 */
#define	DATE		12 /* 18 */
#define	DAY		3 //0:SUNDAY 1:MONDAY 2:TUESDAY 3:WEDNESDAY 4:THURSDAY 5:FRIDAY 6:SATURDAY
#define	HOUR		18 //24 hours time format
#define	MIN		25
#define	SEC		1
/*End*/

#define	C2K_RTC_RTCALM_SECEN	0x81
#define	C2K_RTC_RTCALM_MINEN	0x82
#define	C2K_RTC_RTCALM_HOUREN	0x84
#define	C2K_RTC_RTCALM_DATEEN	0x88
#define	C2K_RTC_RTCALM_DAYEN	0x90
#define	C2K_RTC_RTCALM_MONEN	0xA0
#define	C2K_RTC_RTCALM_YEAREN	0xC0
#define	C2K_RTC_RTCALM_GLOBALEN	0x80

#define	C2K_RTC_RTCALM_ALMEN	0xff
#define	DISABLE_ALL_ALAM	0x0

#define	C2K_RTCALM_ALMEN	(0x3<<0)


struct rtc_plat_data {
	struct rtc_device *rtc;
	void __iomem *ioaddr;
	int		irq;
	struct clk	*clk;
};

void rtc_reg_write(u32 data, void *addr)
{
	data &= 0x0000FFFF;

	/* The core APB runs at 250Hz ,where as the RTC APB runs at 50MHz.
	 * So the delay need to be inserted between two APB requests, other
	 * wise transaction gets dropped.
	 * Each Core time unit = 4000ps , RTC time unit = 20000ps. 5 times
	 * core time unit = RTC time unit.
	 */

	writel(data, addr);

	udelay(5);
}

u32 rtc_reg_read(void *addr)
{
	u32 data;

	/* The core APB runs at 250Hz ,where as the RTC APB runs at 50MHz.
	 * So the delay need to be inserted between two APB requests, other
	 * wise transaction gets dropped.
	 * Each Core time unit = 4000ps , RTC time unit = 20000ps. 5 times
	 * core time unit = RTC time unit.
	 */

	data = 0x00010000;
	writel(0x00010000, addr);

	/* The core APB runs at 250Hz ,where as the RTC APB runs at 50MHz.
	 * So the delay need to be inserted between two APB requests, other
	 * wise transaction gets dropped.
	 * Each Core time unit = 4000ps , RTC time unit = 20000ps. 5 times
	 * core time unit = RTC time unit.
	 */

	udelay(5);

	data = readl(addr);

	while ((0x00010000 & data) != 0x10000) 	{
		udelay(5);
		data = readl(addr);
	}

	data %= 0xFFFF;

	return data;
}

void dbg_rtc_time(struct rtc_time *rtc_tm)
{
	printk ("\n%s: \
			\n\trtc_tm->tm_sec=%d \
			\n\trtc_tm->tm_min=%d \
			\n\trtc_tm->tm_hour=%d \
			\n\trtc_tm->tm_mday=%d \
			\n\trtc_tm->tm_wday=%d \
			\n\trtc_tm->tm_mon=%d \
			\n\trtc_tm->tm_year=%d \n\n", __func__, \
			rtc_tm->tm_sec, rtc_tm->tm_min, rtc_tm->tm_hour, rtc_tm->tm_mday,\
			rtc_tm->tm_wday, rtc_tm->tm_mon, rtc_tm->tm_year);
}

/* IRQ Handlers */

static irqreturn_t c2k_rtc_interrupt(int irq, void *data)
{
	struct rtc_plat_data *pdata = data;
	void __iomem *ioaddr = pdata->ioaddr;

	pr_debug ("%s: irq=%d: Alaram Rang ..............!!!\n \
			\n\t*(0x%x)=0x%x \
			\n\t*(0x%x)=0x%x \n", __func__, irq, \
			(unsigned int)(ioaddr + C2K_RTC_RTCALM), rtc_reg_read(ioaddr + C2K_RTC_RTCALM),\
			(unsigned int)(ioaddr + C2K_RTC_RTCIM), rtc_reg_read(ioaddr + C2K_RTC_RTCIM));

	rtc_update_irq(pdata->rtc, 1, RTC_IRQF | RTC_AF);
	/*-------------------------
	 * DISBABLING alarm
	 *------------------------
	 */
	rtc_reg_write(DISABLE_ALL_ALAM, ioaddr + C2K_RTC_RTCALM);

	/*-------------------------
	 * DISBABLING the alarm interrupt
	 *------------------------
	 */
	rtc_reg_write(DISABLE_ALL_ALAM, ioaddr + C2K_RTC_RTCIM);

	/*-------------------------
	 * DISBABLING the alarm PENDING bit
	 *------------------------
	 */
	rtc_reg_write(DISABLE_ALL_ALAM, ioaddr + C2K_RTC_RTCPEND);

	return IRQ_HANDLED;
}

/* Time read/write */
static int c2k_rtc_read_time(struct device *dev, struct rtc_time *tm)
{
	struct rtc_plat_data *pdata = dev_get_drvdata(dev);
	void __iomem *ioaddr = pdata->ioaddr;
	unsigned int year, month, day, hour, minute, second, wday;

	year = rtc_reg_read(ioaddr + C2K_RTC_BCDYEAR);
	month = rtc_reg_read(ioaddr + C2K_RTC_BCDMON);
	day = rtc_reg_read(ioaddr + C2K_RTC_BCDDATE);
	wday = rtc_reg_read(ioaddr + C2K_RTC_BCDDAY);
	hour = rtc_reg_read(ioaddr + C2K_RTC_BCDHOUR);
	minute  = rtc_reg_read(ioaddr + C2K_RTC_BCDMIN);
	second = rtc_reg_read(ioaddr + C2K_RTC_BCDSEC);

	pr_debug("%s: BCD:\n \
			\n\tyear.mon.date.day hr:min:sec\n \
			\n\t0x%x.0x%x.0x%x.0x%x 0x%x:0x%x:0x%x\n",__func__,\
			year, month, day, wday,
			hour, minute, second);

	tm->tm_sec = bcd2bin(second);
	tm->tm_min = bcd2bin(minute);
	tm->tm_hour = bcd2bin(hour);
	tm->tm_mday = bcd2bin(day);
	tm->tm_wday = bcd2bin(wday);
	tm->tm_mon = bcd2bin(month);
	tm->tm_year = bcd2bin(year);

	tm->tm_year += 100;
	tm->tm_mon -= 1;

	return rtc_valid_tm(tm);
}

#define	C2K_RTC_RTCCON_STARTB	(0x1<<0) /* RTC Halt */
#define	C2K_RTC_RTCCON_RTCEN	(0x1<<1) /* RTC Write Enable */
#define	C2K_RTC_RTCCON_CLKRST	(0x1<<2) /* RTC RESET */

static int c2k_rtc_set_time(struct device *dev, struct rtc_time *tm)
{
	struct rtc_plat_data *pdata = dev_get_drvdata(dev);
	void __iomem *ioaddr = pdata->ioaddr;
	int year = tm->tm_year - 100;

	pr_debug("%s: Will set:\
			\n%04d.%02d.%02d %02d:%02d:%02d\n",__func__,\
			tm->tm_year+1900, tm->tm_mon, tm->tm_mday, \
			tm->tm_hour, tm->tm_min, tm->tm_sec);

	if (year < 0 || year >= 100) {
		dev_err(dev, "rtc only supports 100 years\n");
		return -EINVAL;
	}

	rtc_reg_write((C2K_RTC_RTCCON_STARTB | C2K_RTC_RTCCON_RTCEN | C2K_RTC_RTCCON_CLKRST), \
			ioaddr + C2K_RTC_RTCCON);

	rtc_reg_write(bin2bcd(tm->tm_min), ioaddr + C2K_RTC_BCDMIN);
	rtc_reg_write(bin2bcd(tm->tm_hour), ioaddr + C2K_RTC_BCDHOUR);
	rtc_reg_write(bin2bcd(tm->tm_mday), ioaddr + C2K_RTC_BCDDATE);
	rtc_reg_write(bin2bcd(tm->tm_wday), ioaddr + C2K_RTC_BCDDAY);
	rtc_reg_write(bin2bcd(tm->tm_mon+1), ioaddr + C2K_RTC_BCDMON);
	rtc_reg_write(bin2bcd(year), ioaddr + C2K_RTC_BCDYEAR);
	rtc_reg_write(bin2bcd(tm->tm_sec), ioaddr + C2K_RTC_BCDSEC);

	rtc_reg_write((C2K_RTC_RTCCON_RTCEN | C2K_RTC_RTCCON_CLKRST), ioaddr + C2K_RTC_RTCCON);

	return 0;
}

static int c2k_rtc_alarm_irq_enable(struct device *dev, unsigned int enabled)
{
	struct rtc_plat_data *pdata = dev_get_drvdata(dev);
	void __iomem *ioaddr = pdata->ioaddr;

	unsigned int tmp;

	pr_debug ("%s: aie=%d\n", __func__, enabled);

	tmp = rtc_reg_read(ioaddr + C2K_RTC_RTCIM) & ~C2K_RTCALM_ALMEN;

	if (enabled)
		tmp |= C2K_RTCALM_ALMEN;

	rtc_reg_write(tmp, ioaddr + C2K_RTC_RTCIM);

	return 0;
}

static int c2k_rtc_read_alarm(struct device *dev, struct rtc_wkalrm *alrm)
{
	struct rtc_plat_data *pdata = dev_get_drvdata(dev);
	void __iomem *ioaddr = pdata->ioaddr;

	struct rtc_time *alm_tm = &alrm->time;
	unsigned int alm_en;

	alm_tm->tm_sec  = rtc_reg_read(ioaddr + C2K_RTC_ALMSEC);
	alm_tm->tm_min  = rtc_reg_read(ioaddr + C2K_RTC_ALMMIN);
	alm_tm->tm_hour = rtc_reg_read(ioaddr + C2K_RTC_ALMHOUR);
	alm_tm->tm_mon  = rtc_reg_read(ioaddr + C2K_RTC_ALMMON);
	alm_tm->tm_mday = rtc_reg_read(ioaddr + C2K_RTC_ALMDATE);
	alm_tm->tm_wday = rtc_reg_read(ioaddr + C2K_RTC_ALMDAY);
	alm_tm->tm_year = rtc_reg_read(ioaddr + C2K_RTC_ALMYEAR);

	alm_en = rtc_reg_read(ioaddr + C2K_RTC_RTCALM);

	alrm->enabled = (alm_en & C2K_RTC_RTCALM_ALMEN) ? 1 : 0;

	pr_debug("%s: alm_en=%d, %04d.%02d.%02d.%02d  %02d:%02d:%02d\n",__func__,
			alm_en,
			1900 + alm_tm->tm_year, alm_tm->tm_mon, alm_tm->tm_mday,
			alm_tm->tm_wday, alm_tm->tm_hour, alm_tm->tm_min, alm_tm->tm_sec);

	/* decode the alarm enable field */

	if (alm_en & C2K_RTC_RTCALM_SECEN)
		alm_tm->tm_sec = bcd2bin(alm_tm->tm_sec);
	else
		alm_tm->tm_sec = -1;

	if (alm_en & C2K_RTC_RTCALM_MINEN)
		alm_tm->tm_min = bcd2bin(alm_tm->tm_min);
	else
		alm_tm->tm_min = -1;

	if (alm_en & C2K_RTC_RTCALM_HOUREN)
		alm_tm->tm_hour = bcd2bin(alm_tm->tm_hour);
	else
		alm_tm->tm_hour = -1;

	if (alm_en & C2K_RTC_RTCALM_DAYEN)
		alm_tm->tm_mday = bcd2bin(alm_tm->tm_mday);
	else
		alm_tm->tm_mday = -1;

	if (alm_en & C2K_RTC_RTCALM_MONEN) {
		alm_tm->tm_mon = bcd2bin(alm_tm->tm_mon);
		alm_tm->tm_mon -= 1;
	} else {
		alm_tm->tm_mon = -1;
	}

	if (alm_en & C2K_RTC_RTCALM_YEAREN){
		alm_tm->tm_year = bcd2bin(alm_tm->tm_year);
		alm_tm->tm_year += 100;
	}
	else
		alm_tm->tm_year = -1;

	return 0;
}

static int c2k_rtc_set_alarm(struct device *dev, struct rtc_wkalrm *alrm)
{
	struct rtc_plat_data *pdata = dev_get_drvdata(dev);
	void __iomem *ioaddr = pdata->ioaddr;
	struct rtc_time *tm = &alrm->time;

	pr_debug("%s: %d, %04d.%02d.%02d %02d:%02d:%02d\n",__func__,
			alrm->enabled,
			1900 + tm->tm_year, tm->tm_mon + 1, tm->tm_mday,
			tm->tm_hour, tm->tm_min, tm->tm_sec);

	rtc_reg_write(0x3, (ioaddr + C2K_RTC_RTCIM)); //enable alarm interrupt mode
	rtc_reg_write(0xff, (ioaddr + C2K_RTC_RTCALM)); //enable all alarms.

	if (tm->tm_sec < 60 && tm->tm_sec >= 0)
		rtc_reg_write(bin2bcd(tm->tm_sec), ioaddr + C2K_RTC_ALMSEC);

	if (tm->tm_min < 60 && tm->tm_min >= 0)
		rtc_reg_write(bin2bcd(tm->tm_min), ioaddr + C2K_RTC_ALMMIN);

	if (tm->tm_hour < 24 && tm->tm_hour >= 0)
		rtc_reg_write(bin2bcd(tm->tm_hour), ioaddr + C2K_RTC_ALMHOUR);

	if (tm->tm_mday < 32 && tm->tm_mday >= 1)
		rtc_reg_write(bin2bcd(tm->tm_mday), ioaddr + C2K_RTC_ALMDATE);

	if (tm->tm_wday < 7 && tm->tm_wday >= 0) {
		rtc_reg_write(bin2bcd(tm->tm_wday), ioaddr + C2K_RTC_ALMDAY);
	}

	if (tm->tm_mon < 12 && tm->tm_mon >= 0)
		rtc_reg_write(bin2bcd(tm->tm_mon+1), ioaddr + C2K_RTC_ALMMON);

	if (tm->tm_year >= 1) {
		int year = tm->tm_year - 100;

		if (year < 0 || year >= 100) {
			dev_err(dev, "rtc only supports 100 years\n");
			return -EINVAL;
		}

		rtc_reg_write(bin2bcd(year), ioaddr + C2K_RTC_ALMYEAR);
	}

	c2k_rtc_alarm_irq_enable(dev, alrm->enabled);

	return 0;
}

static const struct rtc_class_ops c2k_rtc_ops = {
	.read_time	= c2k_rtc_read_time,
	.set_time	= c2k_rtc_set_time,
};

static const struct rtc_class_ops c2k_rtc_alarm_ops = {
	.read_time	= c2k_rtc_read_time,
	.set_time	= c2k_rtc_set_time,
	.read_alarm	= c2k_rtc_read_alarm,
	.set_alarm	= c2k_rtc_set_alarm,
	.alarm_irq_enable = c2k_rtc_alarm_irq_enable,
};


#if 0
static int __devexit c2k_rtc_remove(struct platform_device *dev)
{
	struct rtc_device *rtc = platform_get_drvdata(dev);

	free_irq(rtc_alarmno, rtc);

	platform_set_drvdata(dev, NULL);
	rtc_device_unregister(rtc);

	c2k_rtc_setaie(&dev->dev, 0);

	iounmap(c2k_rtc_base);
	release_resource(c2k_rtc_mem);
	kfree(c2k_rtc_mem);

	return 0;
}
#endif

static int __exit c2k_rtc_remove(struct platform_device *pdev)
{
	struct rtc_plat_data *pdata = platform_get_drvdata(pdev);

	if (pdata->irq >= 0)
		device_init_wakeup(&pdev->dev, 0);

	if (!IS_ERR(pdata->clk))
		clk_disable_unprepare(pdata->clk);

	return 0;
}

#if 0
static int __devinit c2k_rtc_probe(struct platform_device *pdev)
{
	struct rtc_device *rtc;
	struct rtc_time rtc_tm;
	struct resource *res;
	int ret;

	rtc_alarmno = platform_get_irq(pdev, 0);
	if (rtc_alarmno < 0) {
		dev_err(&pdev->dev, "no irq for alarm\n");
		return -ENOENT;
	}

	/* get the memory region */

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		dev_err(&pdev->dev, "failed to get memory region resource\n");
		return -ENOENT;
	}

	pr_debug ("%s: alarm irq=%d res->start=0x%x res->end=0x%x\n", __func__, \
			rtc_alarmno, res->start, res->end);

	c2k_rtc_mem = request_mem_region(res->start, resource_size(res),
			pdev->name);
	if (c2k_rtc_mem == NULL) {
		printk("%s: failed to reserve memory region\n", __func__);
		ret = -ENOENT;
		goto err_nores;
	}

	c2k_rtc_base = ioremap(res->start, resource_size(res));
	if (c2k_rtc_base == NULL) {
		printk ("%s: failed ioremap()\n", __func__);
		ret = -EINVAL;
		goto err_nomap;
	}

	device_init_wakeup(&pdev->dev, 1);

	/* register RTC and exit */

	rtc = rtc_device_register("c2k", &pdev->dev, &c2k_rtcops,
			THIS_MODULE);
	if (IS_ERR(rtc)) {
		dev_err(&pdev->dev, "cannot attach rtc\n");
		ret = PTR_ERR(rtc);
		goto err_nortc;
	} else
		printk(banner);

	/* Check RTC Time */
	c2k_rtc_gettime(NULL, &rtc_tm);

	if (rtc_valid_tm(&rtc_tm)) {
		rtc_tm.tm_year	= YEAR;
		rtc_tm.tm_mon	= MONTH;
		rtc_tm.tm_mday	= DATE;
		rtc_tm.tm_wday	= DAY;
		rtc_tm.tm_hour	= HOUR;
		rtc_tm.tm_min	= MIN;
		rtc_tm.tm_sec	= SEC;

		c2k_rtc_settime(NULL, &rtc_tm);

		dev_warn(&pdev->dev, "Warning: Invalid RTC value so initializing it\n");
	}

	platform_set_drvdata(pdev, rtc);

	ret = request_irq(rtc_alarmno, c2k_rtc_alarmirq,
			IRQF_DISABLED,  "rtc-alarm", rtc);
	if (ret) {
		dev_err(&pdev->dev, "IRQ%d error %d\n", rtc_alarmno, ret);
		goto err_alarm_irq;
	}

	return 0;

err_alarm_irq:
	platform_set_drvdata(pdev, NULL);
	rtc_device_unregister(rtc);

err_nortc:
	iounmap(c2k_rtc_base);

err_nomap:
	release_resource(c2k_rtc_mem);

err_nores:
	return ret;
}
#endif

static int __init c2k_rtc_probe(struct platform_device *pdev)
{
	struct resource *res;
	struct rtc_plat_data *pdata;
	struct rtc_time tm;
	int ret = 0;

	pdata = devm_kzalloc(&pdev->dev, sizeof(*pdata), GFP_KERNEL);
	if (!pdata)
		return -ENOMEM;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	pdata->ioaddr = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(pdata->ioaddr))
		return PTR_ERR(pdata->ioaddr);

	pdata->clk = devm_clk_get(&pdev->dev, NULL);

	device_init_wakeup(&pdev->dev, 1);

	/* Check RTC Time */
	c2k_rtc_read_time(NULL, &tm);

	if (rtc_valid_tm(&tm)) {
		tm.tm_year	= YEAR;
		tm.tm_mon	= MONTH;
		tm.tm_mday	= DATE;
		tm.tm_wday	= DAY;
		tm.tm_hour	= HOUR;
		tm.tm_min	= MIN;
		tm.tm_sec	= SEC;

		c2k_rtc_set_time(NULL, &tm);

		dev_warn(&pdev->dev, "Warning: Invalid RTC value so initializing it\n");
	}


	pdata->irq = platform_get_irq(pdev, 0);

	platform_set_drvdata(pdev, pdata);

	if (pdata->irq >= 0) {
		device_init_wakeup(&pdev->dev, 1);
		pdata->rtc = devm_rtc_device_register(&pdev->dev, pdev->name,
						 &c2k_rtc_alarm_ops,
						 THIS_MODULE);
	} else {
		pdata->rtc = devm_rtc_device_register(&pdev->dev, pdev->name,
						 &c2k_rtc_ops, THIS_MODULE);
	}
	if (IS_ERR(pdata->rtc)) {
		ret = PTR_ERR(pdata->rtc);
		goto out;
	}

	if (pdata->irq >= 0) {
		rtc_reg_write(DISABLE_ALL_ALAM, pdata->ioaddr + C2K_RTC_RTCIM);
		if (devm_request_irq(&pdev->dev, pdata->irq, c2k_rtc_interrupt,
				     IRQF_SHARED,
				     pdev->name, pdata) < 0) {
			dev_warn(&pdev->dev, "interrupt not available.\n");
			pdata->irq = -1;
		}
	}

	return 0;
out:
	if (!IS_ERR(pdata->clk))
		clk_disable_unprepare(pdata->clk);

	return ret;
}


#ifdef CONFIG_PM

/* RTC Power management control */
static int c2k_rtc_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

static int c2k_rtc_resume(struct platform_device *pdev)
{
	return 0;
}
#else
#define c2k_rtc_suspend NULL
#define c2k_rtc_resume  NULL
#endif

#ifdef CONFIG_OF
static const struct of_device_id rtc_c2k_of_match_table[] = {
	{ .compatible = "comcerto-2k,comcerto-rtc", },
	{}
};
MODULE_DEVICE_TABLE(of, rtc_c2k_of_match_table);
#endif

static struct platform_driver c2k_rtc_driver = {
	.remove		= __exit_p(c2k_rtc_remove),
	.suspend	= c2k_rtc_suspend,
	.resume		= c2k_rtc_resume,
	.driver		= {
		.name	= "rtc-c2k",
		.of_match_table = of_match_ptr(rtc_c2k_of_match_table),
	},
};

module_platform_driver_probe(c2k_rtc_driver, c2k_rtc_probe);

MODULE_DESCRIPTION("Mindspeed RTC Driver");
MODULE_AUTHOR("Hans Uli Kroll <ulli.krollgooglemail.com>");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:c2k-rtc");
