// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#ifndef MAME_CPU_M6805_M68705_H
#define MAME_CPU_M6805_M68705_H

#pragma once

#include "m6805.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

extern device_type const M68705;
extern device_type const M68705P3;
extern device_type const M68705P5;
extern device_type const M68705U3;


//**************************************************************************
//  TYPE DECLARATIONS
//**************************************************************************

// ======================> m68705_device

class m68705_device : public m6805_base_device
{
public:
	m68705_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	enum
	{
		M68705_A = M6805_A,
		M68705_PC = M6805_PC,
		M68705_S = M6805_S,
		M68705_X = M6805_X,
		M68705_CC = M6805_CC,
		M68705_IRQ_STATE = M6805_IRQ_STATE
	};

	m68705_device(
			machine_config const &mconfig,
			char const *tag,
			device_t *owner,
			u32 clock,
			device_type type,
			char const *name,
			u32 addr_width,
			address_map_delegate internal_map,
			char const *shortname,
			char const *source);

	virtual void device_reset() override;
	virtual void execute_set_input(int inputnum, int state) override;
	virtual void interrupt() override;
};


// ======================> m68705_new_device

#define MCFG_M68705_PORTA_R_CB(obj) \
	devcb = &m68705_new_device::set_port_cb_r<0>(*device, DEVCB_##obj);

#define MCFG_M68705_PORTB_R_CB(obj) \
	devcb = &m68705_new_device::set_port_cb_r<1>(*device, DEVCB_##obj);

#define MCFG_M68705_PORTC_R_CB(obj) \
	devcb = &m68705_new_device::set_port_cb_r<2>(*device, DEVCB_##obj);

#define MCFG_M68705_PORTD_R_CB(obj) \
	devcb = &m68705_new_device::set_port_cb_r<3>(*device, DEVCB_##obj);

#define MCFG_M68705_PORTA_W_CB(obj) \
	devcb = &m68705_new_device::set_port_cb_w<0>(*device, DEVCB_##obj);

#define MCFG_M68705_PORTB_W_CB(obj) \
	devcb = &m68705_new_device::set_port_cb_w<1>(*device, DEVCB_##obj);

#define MCFG_M68705_PORTC_W_CB(obj) \
	devcb = &m68705_new_device::set_port_cb_w<2>(*device, DEVCB_##obj);


class m68705_new_device : public m68705_device
{
public:
	// static configuration helpers
	template<std::size_t N, typename Object> static devcb_base &set_port_cb_r(device_t &device, Object &&obj)
	{ return downcast<m68705_new_device &>(device).m_port_cb_r[N].set_callback(std::forward<Object>(obj)); }
	template<std::size_t N, typename Object> static devcb_base &set_port_cb_w(device_t &device, Object &&obj)
	{ return downcast<m68705_new_device &>(device).m_port_cb_w[N].set_callback(std::forward<Object>(obj)); }

protected:
	enum
	{
		PORT_COUNT = 4
	};

	enum
	{
		TIMER_68705_PRESCALER_EXPIRED,
	};

	m68705_new_device(
			machine_config const &mconfig,
			char const *tag,
			device_t *owner,
			u32 clock,
			device_type type,
			char const *name,
			u32 addr_width,
			address_map_delegate internal_map,
			char const *shortname,
			char const *source);

	template <offs_t B> DECLARE_READ8_MEMBER(eprom_r);
	template <offs_t B> DECLARE_WRITE8_MEMBER(eprom_w);

	template <std::size_t N> void set_port_open_drain(bool value);
	template <std::size_t N> void set_port_mask(u8 mask);
	template <std::size_t N> DECLARE_WRITE8_MEMBER(port_input_w) { m_port_input[N] = data & ~m_port_mask[N]; }
	template <std::size_t N> DECLARE_READ8_MEMBER(port_r);
	template <std::size_t N> DECLARE_WRITE8_MEMBER(port_latch_w);
	template <std::size_t N> DECLARE_WRITE8_MEMBER(port_ddr_w);
	template <std::size_t N> void port_cb_w();

	DECLARE_READ8_MEMBER(internal_68705_tdr_r);
	DECLARE_WRITE8_MEMBER(internal_68705_tdr_w);
	DECLARE_READ8_MEMBER(internal_68705_tcr_r);
	DECLARE_WRITE8_MEMBER(internal_68705_tcr_w);

	DECLARE_READ8_MEMBER(pcr_r);
	DECLARE_WRITE8_MEMBER(pcr_w);

	TIMER_CALLBACK_MEMBER(timer_68705_increment);

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void execute_set_input(int inputnum, int state) override;

	u8 m_tdr;
	u8 m_tcr;

	/* Timers */
	emu_timer *m_68705_timer;

private:
	required_region_ptr<u8> m_user_rom;

	bool                    m_port_open_drain[PORT_COUNT];
	u8                      m_port_mask[PORT_COUNT];
	u8                      m_port_input[PORT_COUNT];
	u8                      m_port_latch[PORT_COUNT];
	u8                      m_port_ddr[PORT_COUNT];
	devcb_read8             m_port_cb_r[PORT_COUNT];
	devcb_write8            m_port_cb_w[PORT_COUNT];

	u8                      m_vihtp;
	u8                      m_pcr;
	u8                      m_pl_data;
	u16                     m_pl_addr;
};


// ======================> m68705p_device

class m68705p_device : public m68705_new_device
{
public:
	DECLARE_WRITE8_MEMBER(pa_w) { port_input_w<0>(space, offset, data, mem_mask); }
	DECLARE_WRITE8_MEMBER(pb_w) { port_input_w<1>(space, offset, data, mem_mask); }
	DECLARE_WRITE8_MEMBER(pc_w) { port_input_w<2>(space, offset, data, mem_mask); }

protected:
	DECLARE_ADDRESS_MAP(p_map, 8);

	m68705p_device(
			machine_config const &mconfig,
			char const *tag,
			device_t *owner,
			u32 clock,
			device_type type,
			char const *name,
			char const *shortname,
			char const *source);
};


// ======================> m68705p3_device

class m68705p3_device : public m68705p_device
{
public:
	m68705p3_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	virtual tiny_rom_entry const *device_rom_region() const override;
};


// ======================> m68705p5_device

class m68705p5_device : public m68705p_device
{
public:
	m68705p5_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

protected:
	virtual tiny_rom_entry const *device_rom_region() const override;
};


// ======================> m68705u3_device

class m68705u3_device : public m68705_new_device
{
public:
	m68705u3_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	DECLARE_WRITE8_MEMBER(pa_w) { port_input_w<0>(space, offset, data, mem_mask); }
	DECLARE_WRITE8_MEMBER(pb_w) { port_input_w<1>(space, offset, data, mem_mask); }
	DECLARE_WRITE8_MEMBER(pc_w) { port_input_w<2>(space, offset, data, mem_mask); }
	DECLARE_WRITE8_MEMBER(pd_w) { port_input_w<3>(space, offset, data, mem_mask); } // TODO: PD6 is also /INT2

protected:
	DECLARE_ADDRESS_MAP(u_map, 8);

	virtual tiny_rom_entry const *device_rom_region() const override;
};


/****************************************************************************
 * 68705 section
 ****************************************************************************/

#define M68705_INT_MASK             0x03
#define M68705_IRQ_LINE             (M6805_IRQ_LINE + 0)
#define M68705_INT_TIMER            (M6805_IRQ_LINE + 1)
#define M68705_VPP_LINE             (M6805_IRQ_LINE + 2)
#define M68705_VIHTP_LINE           (M6805_IRQ_LINE + 3)

#endif // MAME_CPU_M6805_M68705_H
