#include "emu.h"
#include "m68705.h"
#include "m6805defs.h"

namespace {

ROM_START( m68705p3 )
	ROM_REGION(0x0073, "bootstrap", 0)
	ROM_LOAD("bootstrap.bin", 0x0000, 0x0073, CRC(696e1383) SHA1(45104fe1dbd683d251ed2b9411b1f4befbb5aff4))
ROM_END

ROM_START( m68705p5 )
	ROM_REGION(0x0073, "bootstrap", 0)
	ROM_LOAD("bootstrap.bin", 0x0000, 0x0073, CRC(f70a8620) SHA1(c154f78c23f10bb903a531cb19e99121d5f7c19c))
ROM_END

ROM_START( m68705u3 )
	ROM_REGION(0x0078, "bootstrap", 0)
	ROM_LOAD("bootstrap.bin", 0x0000, 0x0078, CRC(5946479b) SHA1(834ea00aef5de12dbcd6421a6e21d5ea96cfbf37))
ROM_END

} // anonymous namespace


device_type const M68705 = &device_creator<m68705_device>;
device_type const M68705P3 = &device_creator<m68705p3_device>;
device_type const M68705P5 = &device_creator<m68705p5_device>;
device_type const M68705U3 = &device_creator<m68705u3_device>;


/****************************************************************************
 * M68705 device (no peripherals)
 ****************************************************************************/

m68705_device::m68705_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: m6805_base_device(mconfig, tag, owner, clock, M68705, "M68705", 12, "m68705", __FILE__)
{
}

m68705_device::m68705_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		u32 clock,
		device_type type,
		char const *name,
		u32 addr_width,
		address_map_delegate internal_map,
		char const *shortname,
		char const *source)
	: m6805_base_device(mconfig, tag, owner, clock, type, name, addr_width, internal_map, shortname, source)
{
}

/* Generate interrupt - m68705 version */
void m68705_device::interrupt()
{
	if ((m_pending_interrupts & ((1 << M6805_IRQ_LINE) | M68705_INT_MASK)) != 0 )
	{
		if ((CC & IFLAG) == 0)
		{
			PUSHWORD(m_pc);
			PUSHBYTE(m_x);
			PUSHBYTE(m_a);
			PUSHBYTE(m_cc);
			SEI;
			standard_irq_callback(0);

			if ((m_pending_interrupts & (1 << M68705_IRQ_LINE)) != 0 )
			{
				m_pending_interrupts &= ~(1 << M68705_IRQ_LINE);
				RM16(0xfffa, &m_pc);
			}
			else if ((m_pending_interrupts & (1 << M68705_INT_TIMER)) != 0)
			{
				m_pending_interrupts &= ~(1 << M68705_INT_TIMER);
				RM16(0xfff8, &m_pc);
			}
		}
		m_icount -= 11;
	}
}

void m68705_device::device_reset()
{
	m6805_base_device::device_reset();

	RM16(0xfffe, &m_pc);
}

void m68705_device::execute_set_input(int inputnum, int state)
{
	if (m_irq_state[inputnum] != state)
	{
		m_irq_state[inputnum] = state;

		if (state != CLEAR_LINE)
		{
			m_pending_interrupts |= 1 << inputnum;
		}
	}
}


/****************************************************************************
 * M68705 "new" device
 ****************************************************************************/

/*
The 68(7)05 peripheral memory map:
Common for Px, Rx, Ux parts:
0x00: Port A data (RW)
0x01: Port B data (RW)
0x02: Port C data (RW) [top 4 bits do nothing (read as 1s) on Px parts, work as expected on Rx, Ux parts]
0x03: [Port D data (Read only), only on Rx, Ux parts]
0x04: Port A DDR (Write only, reads as 0xFF)
0x05: Port B DDR (Write only, reads as 0xFF)
0x06: Port C DDR (Write only, reads as 0xFF) [top 4 bits do nothing on Px parts, work as expected on Rx, Ux parts]
0x07: Unused (reads as 0xFF?)
0x08: Timer Data Register (RW; acts as ram when timer isn't counting, otherwise decrements once per prescaler expiry)
0x09: Timer Control Register (RW; on certain mask part and when MOR bit 6 is not set, all bits are RW except bit 3 which
always reads as zero. when MOR bit 6 is set and on all mask parts except one listed in errata in the 6805 daatsheet,
the top two bits are RW, bottom 6 always read as 1 and writes do nothing; on the errata chip, bit 3 is writable and
clears the prescaler, reads as zero)
0x0A: [Miscellaneous Register, only on Rx, Sx, Ux parts]
0x0B: [Eprom parts: Programming Control Register (write only?, low 3 bits; reads as 0xFF?); Unused (reads as 0xFF?) on
Mask parts]
0x0C: Unused (reads as 0xFF?)
0x0D: Unused (reads as 0xFF?)
0x0E: [A/D control register, only on Rx, Sx parts]
0x0F: [A/D result register, only on Rx, Sx parts]
0x10-0x7f: internal ram; SP can only point to 0x60-0x7F. U2/R2 parts have an unused hole from 0x10-0x3F (reads as 0xFF?)
0x80-0xFF: Page 0 user rom
The remainder of the memory map differs here between parts, see appropriate datasheet for each part.
The four vectors are always stored in big endian form as the last 8 bytes of the address space.

Sx specific differences:
0x02: Port C data (RW) [top 6 bits do nothing (read as 1s) on Sx parts]
0x06: Port C DDR (Write only, reads as 0xFF) [top 6 bits do nothing on Sx parts]
0x0B: Timer 2 Data Register MSB
0x0C: Timer 2 Data Register LSB
0x0D: Timer 2 Control Register
0x10: SPI Data Register
0x11: SPI Control Register
0x12-0x3F: Unused (reads as 0xFF?)

Port A has internal pull-ups (about 10kΩ), and can sink 1.6mA at 0.4V
making it capable of driving CMOS or one TTL load directly.

Port B has a true high-Z state (maximum 20µA for Px parts or 10µA for
Rx/Ux parts), and can sink 10mA at 1V (LED drive) or 3.2mA at 0.4V (two
TTL loads).  It requires external pull-ups to drive CMOS.

Port C has a true high-Z state (maximum 20µA for Px parts or 10µA for
Rx/Ux parts), and can sink 1.6mA at 0.4V (one TTL load).  It requires
external pull-ups to drive CMOS.

MOR ADDRESS: Mask Option Register; does not exist on R2 and several other but not all mask parts, located at 0x784 on Px parts

Px Parts:
* 28 pins
* address space is 0x000-0x7ff
* has Ports A-C
* port C is just 4 bits
* EPROM parts have MOR at 0x784 and bootstrap ROM at 0x785-0x7f7
* mask parts have a selftest rom at similar area

Rx Parts:
* 40 pins
* address space is 0x000-0xfff with an unused hole at 0xf39-0xf7f
* R2 parts have an unused hole at 0x10-0x3f and at 0x100-0x7bF
* has A/D converter, Ports A-D
* mask parts lack programmable prescaler
* EPROM parts have MOR at 0xf38 and bootstrap ROM at 0xf80-0xff7
* mask parts have selftest ROM at 0xf38-0xff7
* selftest ROMs differ between the U2 and U3 versions

Sx Parts:
* 40 pins
* address space is 0x000-0xfff with an unused hole at 0x12-0x3f and at 0x100-0x9BF
* has A/D converter, SPI serial
* port C is just two bits
* has an extra 16-bit timer compared to Ux/Rx
* selftest rom at 0xF00-0xFF7

Ux Parts:
* 40 pins
* address space is 0x000-0xfff with an unused hole at 0xf39-0xf7f
* U2 parts have an unused hole at 0x10-0x3f and at 0x100-0x7bF
* has Ports A-D
* EPROM parts have MOR at 0xf38 and bootstrap ROM at 0xf80-0xff7
* mask parts have selftest ROM at 0xf38-0xff7
* selftest ROMs differ between the U2 and U3 versions

*/

m68705_new_device::m68705_new_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		u32 clock,
		device_type type,
		char const *name,
		u32 addr_width,
		address_map_delegate internal_map,
		char const *shortname,
		char const *source)
	: m68705_device(mconfig, tag, owner, clock, type, name, addr_width, internal_map, shortname, source)
	, m_user_rom(*this, DEVICE_SELF, u32(1) << addr_width)
	, m_port_open_drain{ false, false, false, false }
	, m_port_mask{ 0x00, 0x00, 0x00, 0x00 }
	, m_port_input{ 0xff, 0xff, 0xff, 0xff }
	, m_port_latch{ 0xff, 0xff, 0xff, 0xff }
	, m_port_ddr{ 0x00, 0x00, 0x00, 0x00 }
	, m_port_cb_r{ { *this }, { *this }, { *this }, { *this } }
	, m_port_cb_w{ { *this }, { *this }, { *this }, { *this } }
	, m_pcr(0xff)
{
}

template <offs_t B> READ8_MEMBER(m68705_new_device::eprom_r)
{
	// read locked out when /VPON and /PLE are asserted
	return (BIT(m_pcr, 2) || BIT(m_pcr, 0)) ? m_user_rom[B + offset] : 0xff;
}

template <offs_t B> WRITE8_MEMBER(m68705_new_device::eprom_w)
{
	// programming latch enabled when /VPON and /PLE are asserted
	if (!BIT(m_pcr, 2) && !BIT(m_pcr, 0))
	{
		if (BIT(m_pcr, 1))
		{
			m_pl_data = data;
			m_pl_addr = B + offset;
		}
		else
		{
			// this causes undefined behaviour, which is bad when EPROM programming is involved
			logerror("warning: write to EPROM when /PGE = 0 (%x = %x)\n", B + offset, data);
		}
	}
}

template <std::size_t N> void m68705_new_device::set_port_open_drain(bool value)
{
	m_port_open_drain[N] = value;
}

template <std::size_t N> void m68705_new_device::set_port_mask(u8 mask)
{
	m_port_mask[N] = mask;
}

template <std::size_t N> READ8_MEMBER(m68705_new_device::port_r)
{
	if (!m_port_cb_r[N].isnull()) m_port_input[N] = m_port_cb_r[N](space, 0, ~m_port_ddr[N]);
	return m_port_mask[N] | (m_port_latch[N] & m_port_ddr[N]) | (m_port_input[N] & ~m_port_ddr[N]);
}

template <std::size_t N> WRITE8_MEMBER(m68705_new_device::port_latch_w)
{
	data &= ~m_port_mask[N];
	u8 const diff = m_port_latch[N] ^ data;
	m_port_latch[N] = data;
	if (diff & m_port_ddr[N])
		port_cb_w<N>();
}

template <std::size_t N> WRITE8_MEMBER(m68705_new_device::port_ddr_w)
{
	data &= ~m_port_mask[N];
	if (data != m_port_ddr[N])
	{
		m_port_ddr[N] = data;
		port_cb_w<N>();
	}
}

template <std::size_t N> void m68705_new_device::port_cb_w()
{
	u8 const data(m_port_open_drain[N] ? m_port_latch[N] | ~m_port_ddr[N] : m_port_latch[N]);
	u8 const mask(m_port_open_drain[N] ? (~m_port_latch[N] & m_port_ddr[N]) : m_port_ddr[N]);
	m_port_cb_w[N](space(AS_PROGRAM), 0, data, mask);
}

READ8_MEMBER(m68705_new_device::pcr_r)
{
	return m_pcr;
}

WRITE8_MEMBER(m68705_new_device::pcr_w)
{
	data |= ((data & 0x01) << 1); // lock out /PGE if /PLE is not asserted
	if (!BIT(m_pcr, 2) && (0x20 & ((m_pcr ^ data) & ~data)))
	{
		logerror("warning: unimplemented EPROM write %x |= %x\n", m_pl_addr, m_pl_data);
		popmessage("%s: EPROM write", tag());
	}
	m_pcr = (m_pcr & 0xfc) | (data & 0x03);
}

READ8_MEMBER(m68705_new_device::internal_68705_tdr_r)
{
	//logerror("internal_68705 TDR read, returning %02X\n", m_tdr);
	return m_tdr;
}

WRITE8_MEMBER(m68705_new_device::internal_68705_tdr_w)
{
	//logerror("internal_68705 TDR written with %02X, was %02X\n", data, m_tdr);
	m_tdr = data;
}


READ8_MEMBER(m68705_new_device::internal_68705_tcr_r)
{
	//logerror("internal_68705 TCR read, returning %02X\n", (m_tcr&0xF7));
	return (m_tcr & 0xF7);
}

WRITE8_MEMBER(m68705_new_device::internal_68705_tcr_w)
{
/*
    logerror("internal_68705 TCR written with %02X\n", data);
    if (data&0x80) logerror("  TIR=1, Timer Interrupt state is set\n"); else logerror("  TIR=0; Timer Interrupt state is cleared\n");
    if (data&0x40) logerror("  TIM=1, Timer Interrupt is now masked\n"); else logerror("  TIM=0, Timer Interrupt is now unmasked\n");
    if (data&0x20) logerror("  TIN=1, Timer Clock source is set to external\n"); else logerror("  TIN=0, Timer Clock source is set to internal\n");
    if (data&0x10) logerror("  TIE=1, Timer External pin is enabled\n"); else logerror("  TIE=0, Timer External pin is disabled\n");
    if (data&0x08) logerror("  PSC=1, Prescaler counter cleared\n"); else logerror("  PSC=0, Prescaler counter left alone\n");
    logerror("  Prescaler: %d\n", (1<<(data&0x7)));
*/
	// if timer was enabled but now isn't, shut it off.
	// below is a hack assuming the TIMER pin isn't going anywhere except tied to +5v, so basically TIN is acting as an active-low timer enable, and TIE is ignored even in the case where TIE=1, the timer will end up being 5v ANDED against the internal timer clock which == the internal timer clock.
	// Note this hack is incorrect; the timer pin actually does connect somewhere (vblank or maybe one of the V counter bits?), but the game never actually uses the timer pin in external clock mode, so the TIMER connection must be left over from development. We can apparently safely ignore it.
	if ((m_tcr^data)&0x20)// check if TIN state changed
	{
		/* logerror("timer enable state changed!\n"); */
		if (data&0x20) m_68705_timer->adjust(attotime::never, TIMER_68705_PRESCALER_EXPIRED);
		else m_68705_timer->adjust(attotime::from_hz(((clock())/4)/(1<<(data&0x7))), TIMER_68705_PRESCALER_EXPIRED);
	}
	// prescaler check: if timer prescaler has changed, or the PSC bit is set, adjust the timer length for the prescaler expired timer, but only if the timer would be running
	if ( (((m_tcr&0x07)!=(data&0x07))||(data&0x08)) && ((data&0x20)==0) )
	{
		/* logerror("timer reset due to PSC or prescaler change!\n"); */
		m_68705_timer->adjust(attotime::from_hz(((clock())/4)/(1<<(data&0x7))), TIMER_68705_PRESCALER_EXPIRED);
	}
	m_tcr = data;
	// if int state is set, and TIM is unmasked, assert an interrupt. otherwise clear it.
	if ((m_tcr&0xC0) == 0x80)
		set_input_line(M68705_INT_TIMER, ASSERT_LINE);
	else
		set_input_line(M68705_INT_TIMER, CLEAR_LINE);

}

TIMER_CALLBACK_MEMBER(m68705_new_device::timer_68705_increment)
{
	m_tdr++;
	if (m_tdr == 0x00) m_tcr |= 0x80; // if we overflowed, set the int bit
	if ((m_tcr&0xC0) == 0x80)
		set_input_line(M68705_INT_TIMER, ASSERT_LINE);
	else
		set_input_line(M68705_INT_TIMER, CLEAR_LINE);
	m_68705_timer->adjust(attotime::from_hz((clock() / 4) / (1 << (m_tcr & 0x07))), TIMER_68705_PRESCALER_EXPIRED);
}

void m68705_new_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_68705_PRESCALER_EXPIRED:
		timer_68705_increment(ptr, param);
		break;
	default:
		m68705_device::device_timer(timer, id, param, ptr);
	}
}

void m68705_new_device::device_start()
{
	m68705_device::device_start();

	save_item(NAME(m_tdr));
	save_item(NAME(m_tcr));

	save_item(NAME(m_port_input));
	save_item(NAME(m_port_latch));
	save_item(NAME(m_port_ddr));

	save_item(NAME(m_pcr));
	save_item(NAME(m_pl_data));
	save_item(NAME(m_pl_addr));

	for (u8 &input : m_port_input) input = 0xff;
	for (devcb_read8 &cb : m_port_cb_r) cb.resolve_safe(0xff);
	for (devcb_write8 &cb : m_port_cb_w) cb.resolve_safe();

	m_pcr = 0xff;
	m_pl_data = 0xff;
	m_pl_addr = 0xffff;

	// allocate the MCU timer, and set it to fire NEVER.
	m_68705_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(m68705_new_device::timer_68705_increment),this));
	m_68705_timer->adjust(attotime::never);
}

void m68705_new_device::device_reset()
{
	m68705_device::device_reset();

	port_ddr_w<0>(space(AS_PROGRAM), 0, 0x00, 0xff);
	port_ddr_w<1>(space(AS_PROGRAM), 0, 0x00, 0xff);
	port_ddr_w<2>(space(AS_PROGRAM), 0, 0x00, 0xff);
	port_ddr_w<3>(space(AS_PROGRAM), 0, 0x00, 0xff);

	m_pcr |= 0xfb; // b2 (/VPON) is driven by external input and hence unaffected by reset

	m_tdr = 0xff;
	m_tcr = 0xff;

	//set_input_line(M68705_IRQ_LINE, CLEAR_LINE);
	m_68705_timer->adjust(attotime::from_hz((clock() / 4) / (1 << 7)));
}

void m68705_new_device::execute_set_input(int inputnum, int state)
{
	switch (inputnum)
	{
	case M68705_VPP_LINE:
		if (ASSERT_LINE == state)
			m_pcr &= 0xfb;
		else
			m_pcr |= 0x40;
		break;
	default:
		m68705_device::execute_set_input(inputnum, state);
	}
}


/****************************************************************************
 * M68705P3x family
 ****************************************************************************/

DEVICE_ADDRESS_MAP_START( p_map, 8, m68705p_device )
	ADDRESS_MAP_GLOBAL_MASK(0x07ff)
	ADDRESS_MAP_UNMAP_HIGH

	AM_RANGE(0x0000, 0x0000) AM_READWRITE(port_r<0>, port_latch_w<0>)
	AM_RANGE(0x0001, 0x0001) AM_READWRITE(port_r<1>, port_latch_w<1>)
	AM_RANGE(0x0002, 0x0002) AM_READWRITE(port_r<2>, port_latch_w<2>)
	// 0x0003 not used (no port D)
	AM_RANGE(0x0004, 0x0004) AM_WRITE(port_ddr_w<0>)
	AM_RANGE(0x0005, 0x0005) AM_WRITE(port_ddr_w<1>)
	AM_RANGE(0x0006, 0x0006) AM_WRITE(port_ddr_w<2>)
	// 0x0007 not used (no port D)
	AM_RANGE(0x0008, 0x0008) AM_READWRITE(internal_68705_tdr_r, internal_68705_tdr_w)
	AM_RANGE(0x0009, 0x0009) AM_READWRITE(internal_68705_tcr_r, internal_68705_tcr_w)
	// 0x000a not used
	AM_RANGE(0x000b, 0x000b) AM_READWRITE(pcr_r, pcr_w)
	// 0x000c-0x000f not used
	AM_RANGE(0x0010, 0x007f) AM_RAM
	AM_RANGE(0x0080, 0x0784) AM_READWRITE(eprom_r<0x0080>, eprom_w<0x0080>) // User EPROM
	AM_RANGE(0x0785, 0x07f7) AM_ROM AM_REGION("bootstrap", 0)
	AM_RANGE(0x07f8, 0x07ff) AM_READWRITE(eprom_r<0x07f8>, eprom_w<0x07f8>) // Interrupt vectors
ADDRESS_MAP_END

m68705p_device::m68705p_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		u32 clock,
		device_type type,
		char const *name,
		char const *shortname,
		char const *source)
	: m68705_new_device(mconfig, tag, owner, clock, type, name, 11, address_map_delegate(FUNC(m68705p_device::p_map), this), shortname, source)
{
	set_port_open_drain<0>(true);   // Port A is open drain with internal pull-ups
	set_port_mask<2>(0xf0);         // Port C is four bits wide
	set_port_mask<3>(0xff);         // Port D isn't present
}


/****************************************************************************
 * M68705P3 device
 ****************************************************************************/

m68705p3_device::m68705p3_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: m68705p_device(mconfig, tag, owner, clock, M68705P3, "MC68705P3", "m68705p3", __FILE__)
{
}

tiny_rom_entry const *m68705p3_device::device_rom_region() const
{
	return ROM_NAME(m68705p3);
}


/****************************************************************************
 * M68705P5 device
 ****************************************************************************/

m68705p5_device::m68705p5_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: m68705p_device(mconfig, tag, owner, clock, M68705P3, "MC68705P5", "m68705p5", __FILE__)
{
}

tiny_rom_entry const *m68705p5_device::device_rom_region() const
{
	return ROM_NAME(m68705p5);
}


/****************************************************************************
 * M68705U3 device
 ****************************************************************************/

DEVICE_ADDRESS_MAP_START( u_map, 8, m68705u3_device )
	ADDRESS_MAP_GLOBAL_MASK(0x0fff)
	ADDRESS_MAP_UNMAP_HIGH

	AM_RANGE(0x0000, 0x0000) AM_READWRITE(port_r<0>, port_latch_w<0>)
	AM_RANGE(0x0001, 0x0001) AM_READWRITE(port_r<1>, port_latch_w<1>)
	AM_RANGE(0x0002, 0x0002) AM_READWRITE(port_r<2>, port_latch_w<2>)
	AM_RANGE(0x0003, 0x0003) AM_READWRITE(port_r<3>, port_latch_w<3>)
	AM_RANGE(0x0004, 0x0004) AM_WRITE(port_ddr_w<0>)
	AM_RANGE(0x0005, 0x0005) AM_WRITE(port_ddr_w<1>)
	AM_RANGE(0x0006, 0x0006) AM_WRITE(port_ddr_w<2>)
	// 0x0007 not used (port D is input only)
	AM_RANGE(0x0008, 0x0008) AM_READWRITE(internal_68705_tdr_r, internal_68705_tdr_w)
	AM_RANGE(0x0009, 0x0009) AM_READWRITE(internal_68705_tcr_r, internal_68705_tcr_w)
	// 0x000a TODO: miscellaneous register
	AM_RANGE(0x000b, 0x000b) AM_READWRITE(pcr_r, pcr_w)
	// 0x000c-0x000f not used
	AM_RANGE(0x0010, 0x007f) AM_RAM
	AM_RANGE(0x0080, 0x0f38) AM_READWRITE(eprom_r<0x0080>, eprom_w<0x0080>) // User EPROM
	// 0x0f39-0x0f7f not used
	AM_RANGE(0x0f80, 0x0ff7) AM_ROM AM_REGION("bootstrap", 0)
	AM_RANGE(0x0ff8, 0x0fff) AM_READWRITE(eprom_r<0x0ff8>, eprom_w<0x0ff8>) // Interrupt vectors
ADDRESS_MAP_END

m68705u3_device::m68705u3_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: m68705_new_device(mconfig, tag, owner, clock, M68705U3, "MC68705U3", 11, address_map_delegate(FUNC(m68705u3_device::u_map), this), "m68705u3", __FILE__)
{
	set_port_open_drain<0>(true);   // Port A is open drain with internal pull-ups
}

tiny_rom_entry const *m68705u3_device::device_rom_region() const
{
	return ROM_NAME(m68705u3);
}
