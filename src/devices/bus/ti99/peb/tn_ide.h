// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    Nouspikel IDE controller card
    See tn_ide.c for documentation

    Michael Zapf

    February 2012: Rewritten as class

*****************************************************************************/

#ifndef MAME_BUS_TI99_PEB_TN_IDE_H
#define MAME_BUS_TI99_PEB_TN_IDE_H

#pragma once

#include "peribox.h"
#include "machine/ataintf.h"
#include "machine/ram.h"
#include "machine/rtc65271.h"

DECLARE_DEVICE_TYPE(TI99_IDE, nouspikel_ide_interface_device)

class nouspikel_ide_interface_device : public ti_expansion_card_device
{
public:
	nouspikel_ide_interface_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	DECLARE_READ8Z_MEMBER(readz) override;
	DECLARE_WRITE8_MEMBER(write) override;

	DECLARE_READ8Z_MEMBER(crureadz) override;
	DECLARE_WRITE8_MEMBER(cruwrite) override;

	void    do_inta(int state);
	bool    m_ata_irq;
	int     m_cru_register;

	DECLARE_WRITE_LINE_MEMBER(clock_interrupt_callback);
	DECLARE_WRITE_LINE_MEMBER(ide_interrupt_callback);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

private:
	rtc65271_device*    m_rtc;
	required_device<ata_interface_device> m_ata;

	bool    m_clk_irq;
	bool    m_sram_enable;
	bool    m_sram_enable_dip;
	int     m_cur_page;

	bool    m_tms9995_mode;

	uint16_t  m_input_latch;
	uint16_t  m_output_latch;

	required_device<ram_device> m_ram;
};

#endif // MAME_BUS_TI99_PEB_TN_IDE_H
