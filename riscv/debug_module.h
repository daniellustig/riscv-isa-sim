// See LICENSE for license details.
#ifndef _RISCV_DEBUG_MODULE_H
#define _RISCV_DEBUG_MODULE_H

#include <set>

#include "devices.h"

class sim_t;

typedef struct {
  bool haltreq;
  bool resumereq;
  bool reset;
  bool dmactive;
  enum {
    HARTSTATUS_HALTED,
    HARTSTATUS_RUNNING,
    HARTSTATUS_UNAVAILABLE,
    HARTSTATUS_NOTEXIST
  } hartstatus;
  unsigned hartsel;
  bool authenticated;
  bool authbusy;
  enum {
    AUTHTYPE_NOAUTH,
    AUTHTYPE_PASSWORD,
    AUTHTYPE_CHALLENGE
  } authtype;
  unsigned version;
} dmcontrol_t;

typedef struct {
  bool autoexec7;
  bool autoexec6;
  bool autoexec5;
  bool autoexec4;
  bool autoexec3;
  bool autoexec2;
  bool autoexec1;
  bool autoexec0;
  enum {
    CMDERR_NONE = 0,
    CMDERR_BUSY = 1,
    CMDERR_NOTSUP = 2,
    CMDERR_EXCEPTION = 3,
    CMDERR_HALTRESUME = 4,
    CMDERR_OTHER = 7
  } cmderr;
  bool busy;
  unsigned datacount;
} abstractcs_t;

class debug_module_data_t : public abstract_device_t
{
  public:
    debug_module_data_t();

    bool load(reg_t addr, size_t len, uint8_t* bytes);
    bool store(reg_t addr, size_t len, const uint8_t* bytes);

    uint32_t read32(reg_t addr) const;
    void write32(reg_t addr, uint32_t value);

    uint8_t data[DEBUG_EXCHANGE_SIZE];
};

class debug_module_t : public abstract_device_t
{
  public:
    debug_module_t(sim_t *sim);

    void add_device(bus_t *bus);

    bool load(reg_t addr, size_t len, uint8_t* bytes);
    bool store(reg_t addr, size_t len, const uint8_t* bytes);

    void set_interrupt(uint32_t hartid) {
      interrupt.insert(hartid);
    }
    void clear_interrupt(uint32_t hartid) {
      interrupt.erase(hartid);
    }
    bool get_interrupt(uint32_t hartid) const {
      return interrupt.find(hartid) != interrupt.end();
    }

    void set_halt_notification(uint32_t hartid) {
      halt_notification.insert(hartid);
    }
    void clear_halt_notification(uint32_t hartid) {
      halt_notification.erase(hartid);
    }
    bool get_halt_notification(uint32_t hartid) const {
      return halt_notification.find(hartid) != halt_notification.end();
    }

    // Debug Module Interface that the debugger (in our case through JTAG DTM)
    // uses to access the DM.
    // Return true for success, false for failure.
    bool dmi_read(unsigned address, uint32_t *value);
    bool dmi_write(unsigned address, uint32_t value);

  private:
    static const unsigned progsize = 8;

    sim_t *sim;
    // Track which interrupts from module to debugger are set.
    std::set<uint32_t> interrupt;
    // Track which halt notifications from debugger to module are set.
    std::set<uint32_t> halt_notification;

    uint8_t debug_rom_entry[DEBUG_ROM_ENTRY_SIZE];
    uint8_t debug_rom_code[DEBUG_ROM_CODE_SIZE];
    uint8_t debug_rom_exception[DEBUG_ROM_EXCEPTION_SIZE];
    uint8_t program_buffer[progsize * 4];
    bool halted[1024];
    debug_module_data_t dmdata;
    // Instruction that will be placed at the current hart's ROM entry address
    // after the current action has completed.
    uint32_t next_action;
    bool action_executed;

    void write32(uint8_t *rom, unsigned int index, uint32_t value);
    uint32_t read32(uint8_t *rom, unsigned int index);

    dmcontrol_t dmcontrol;
    abstractcs_t abstractcs;

    processor_t *current_proc() const;
    void reset();
    bool perform_abstract_command(uint32_t command);
};

#endif
