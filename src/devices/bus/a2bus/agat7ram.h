// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/*********************************************************************

    agat7ram.h

    Implemention of the Agat-7 RAM card

*********************************************************************/

#ifndef __A2BUS_AGAT7RAM__
#define __A2BUS_AGAT7RAM__

#include "emu.h"
#include "a2bus.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_agat7ram_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_agat7ram_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	a2bus_agat7ram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_cnxx(address_space &space, uint8_t offset) override;
	virtual void write_cnxx(address_space &space, uint8_t offset, uint8_t data) override;
	virtual uint8_t read_inh_rom(address_space &space, uint16_t offset) override;
	virtual void write_inh_rom(address_space &space, uint16_t offset, uint8_t data) override;
	virtual uint16_t inh_start() override { return 0x8000; }
	virtual uint16_t inh_end() override { return 0xbfff; }
	virtual int inh_type() override;

private:
	void do_io(int offset);

	int m_inh_state;
	int m_last_offset;
	int m_main_bank;
	uint8_t m_ram[32*1024];
};

// device type definition
extern const device_type A2BUS_AGAT7RAM;

#endif /* __A2BUS_AGAT7RAM__ */
