/* Cache and manage the values of registers for GDB, the GNU debugger.

   Copyright (C) 1986-2017 Free Software Foundation, Inc.

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#ifndef REGCACHE_H
#define REGCACHE_H

#include "common-regcache.h"
#include <forward_list>

struct regcache;
struct regset;
struct gdbarch;
struct address_space;

extern struct regcache *get_current_regcache (void);
extern struct regcache *get_thread_regcache (ptid_t ptid);
extern struct regcache *get_thread_arch_regcache (ptid_t, struct gdbarch *);
extern struct regcache *get_thread_arch_aspace_regcache (ptid_t,
							 struct gdbarch *,
							 struct address_space *);

void regcache_xfree (struct regcache *regcache);
struct cleanup *make_cleanup_regcache_xfree (struct regcache *regcache);
struct regcache *regcache_xmalloc (struct gdbarch *gdbarch,
				   struct address_space *aspace);

/* Return REGCACHE's ptid.  */

extern ptid_t regcache_get_ptid (const struct regcache *regcache);

/* Return REGCACHE's architecture.  */

extern struct gdbarch *get_regcache_arch (const struct regcache *regcache);

/* Return REGCACHE's address space.  */

extern struct address_space *get_regcache_aspace (const struct regcache *);

enum register_status regcache_register_status (const struct regcache *regcache,
					       int regnum);

/* Make certain that the register REGNUM in REGCACHE is up-to-date.  */

void regcache_raw_update (struct regcache *regcache, int regnum);

/* Transfer a raw register [0..NUM_REGS) between core-gdb and the
   regcache.  The read variants return the status of the register.  */

enum register_status regcache_raw_read (struct regcache *regcache,
					int rawnum, gdb_byte *buf);
void regcache_raw_write (struct regcache *regcache, int rawnum,
			 const gdb_byte *buf);
extern enum register_status
  regcache_raw_read_signed (struct regcache *regcache,
			    int regnum, LONGEST *val);

extern void regcache_raw_write_signed (struct regcache *regcache,
				       int regnum, LONGEST val);
extern void regcache_raw_write_unsigned (struct regcache *regcache,
					 int regnum, ULONGEST val);

/* Return the register's value in signed or throw if it's not
   available.  */

extern LONGEST regcache_raw_get_signed (struct regcache *regcache,
					int regnum);

/* Set a raw register's value in the regcache's buffer.  Unlike
   regcache_raw_write, this is not write-through.  The intention is
   allowing to change the buffer contents of a read-only regcache
   allocated with regcache_xmalloc.  */

extern void regcache_raw_set_cached_value
  (struct regcache *regcache, int regnum, const gdb_byte *buf);

/* Partial transfer of raw registers.  These perform read, modify,
   write style operations.  The read variant returns the status of the
   register.  */

extern enum register_status
  regcache_raw_read_part (struct regcache *regcache, int regnum,
			  int offset, int len, gdb_byte *buf);
void regcache_raw_write_part (struct regcache *regcache, int regnum,
			      int offset, int len, const gdb_byte *buf);

void regcache_invalidate (struct regcache *regcache, int regnum);

/* Transfer of pseudo-registers.  The read variants return a register
   status, as an indication of when a ``cooked'' register was
   constructed from valid, invalid or unavailable ``raw''
   registers.  */

/* Transfer a cooked register [0..NUM_REGS+NUM_PSEUDO_REGS).  */
enum register_status regcache_cooked_read (struct regcache *regcache,
					   int rawnum, gdb_byte *buf);
void regcache_cooked_write (struct regcache *regcache, int rawnum,
			    const gdb_byte *buf);

/* Read register REGNUM from REGCACHE and return a new value.  This
   will call mark_value_bytes_unavailable as appropriate.  */

struct value *regcache_cooked_read_value (struct regcache *regcache,
					  int regnum);

/* Read a register as a signed/unsigned quantity.  */
extern enum register_status
  regcache_cooked_read_signed (struct regcache *regcache,
			       int regnum, LONGEST *val);
extern enum register_status
  regcache_cooked_read_unsigned (struct regcache *regcache,
				 int regnum, ULONGEST *val);
extern void regcache_cooked_write_signed (struct regcache *regcache,
					  int regnum, LONGEST val);
extern void regcache_cooked_write_unsigned (struct regcache *regcache,
					    int regnum, ULONGEST val);

/* Partial transfer of a cooked register.  These perform read, modify,
   write style operations.  */

enum register_status regcache_cooked_read_part (struct regcache *regcache,
						int regnum, int offset,
						int len, gdb_byte *buf);
void regcache_cooked_write_part (struct regcache *regcache, int regnum,
				 int offset, int len, const gdb_byte *buf);

/* Special routines to read/write the PC.  */

/* For regcache_read_pc see common/common-regcache.h.  */
extern void regcache_write_pc (struct regcache *regcache, CORE_ADDR pc);

/* Transfer a raw register [0..NUM_REGS) between the regcache and the
   target.  These functions are called by the target in response to a
   target_fetch_registers() or target_store_registers().  */

extern void regcache_raw_supply (struct regcache *regcache,
				 int regnum, const void *buf);
extern void regcache_raw_collect (const struct regcache *regcache,
				  int regnum, void *buf);

/* Mapping between register numbers and offsets in a buffer, for use
   in the '*regset' functions below.  In an array of
   'regcache_map_entry' each element is interpreted like follows:

   - If 'regno' is a register number: Map register 'regno' to the
     current offset (starting with 0) and increase the current offset
     by 'size' (or the register's size, if 'size' is zero).  Repeat
     this with consecutive register numbers up to 'regno+count-1'.

   - If 'regno' is REGCACHE_MAP_SKIP: Add 'count*size' to the current
     offset.

   - If count=0: End of the map.  */

struct regcache_map_entry
{
  int count;
  int regno;
  int size;
};

/* Special value for the 'regno' field in the struct above.  */

enum
  {
    REGCACHE_MAP_SKIP = -1,
  };

/* Transfer a set of registers (as described by REGSET) between
   REGCACHE and BUF.  If REGNUM == -1, transfer all registers
   belonging to the regset, otherwise just the register numbered
   REGNUM.  The REGSET's 'regmap' field must point to an array of
   'struct regcache_map_entry'.

   These functions are suitable for the 'regset_supply' and
   'regset_collect' fields in a regset structure.  */

extern void regcache_supply_regset (const struct regset *regset,
				    struct regcache *regcache,
				    int regnum, const void *buf,
				    size_t size);
extern void regcache_collect_regset (const struct regset *regset,
				     const struct regcache *regcache,
				     int regnum, void *buf, size_t size);


/* The type of a register.  This function is slightly more efficient
   then its gdbarch vector counterpart since it returns a precomputed
   value stored in a table.  */

extern struct type *register_type (struct gdbarch *gdbarch, int regnum);


/* Return the size of register REGNUM.  All registers should have only
   one size.  */
   
extern int register_size (struct gdbarch *gdbarch, int regnum);


/* Save/restore a register cache.  The set of registers saved /
   restored into the DST regcache determined by the save_reggroup /
   restore_reggroup respectively.  COOKED_READ returns zero iff the
   register's value can't be returned.  */

typedef enum register_status (regcache_cooked_read_ftype) (void *src,
							   int regnum,
							   gdb_byte *buf);

extern void regcache_save (struct regcache *dst,
			   regcache_cooked_read_ftype *cooked_read,
			   void *cooked_read_context);

enum regcache_dump_what
{
  regcache_dump_none, regcache_dump_raw,
  regcache_dump_cooked, regcache_dump_groups,
  regcache_dump_remote
};

/* The register cache for storing raw register values.  */

class regcache
{
public:
  regcache (gdbarch *gdbarch, address_space *aspace_)
    : regcache (gdbarch, aspace_, true)
  {}

  struct readonly_t {};
  static constexpr readonly_t readonly {};

  /* Create a readonly regcache from a non-readonly regcache.  */
  regcache (readonly_t, const regcache &src);

  regcache (const regcache &) = delete;
  void operator= (const regcache &) = delete;

  /* class regcache is only extended in unit test, so only mark it
     virtual when selftest is enabled.  */
#if GDB_SELF_TEST
  virtual
#endif
  ~regcache ()
  {
    xfree (m_registers);
    xfree (m_register_status);
  }

  gdbarch *arch () const;

  address_space *aspace () const
  {
    return m_aspace;
  }

  void save (regcache_cooked_read_ftype *cooked_read, void *src);

  enum register_status cooked_read (int regnum, gdb_byte *buf);
  void cooked_write (int regnum, const gdb_byte *buf);

  enum register_status raw_read (int regnum, gdb_byte *buf);

  /* class regcache is only extended in unit test, so only mark it
     virtual when selftest is enabled.  */
#if GDB_SELF_TEST
  virtual
#endif
  void raw_write (int regnum, const gdb_byte *buf);

  template<typename T, typename = RequireLongest<T>>
  enum register_status raw_read (int regnum, T *val);

  template<typename T, typename = RequireLongest<T>>
  void raw_write (int regnum, T val);

  struct value *cooked_read_value (int regnum);

  template<typename T, typename = RequireLongest<T>>
  enum register_status cooked_read (int regnum, T *val);

  template<typename T, typename = RequireLongest<T>>
  void cooked_write (int regnum, T val);

  void raw_update (int regnum);

  void raw_collect (int regnum, void *buf) const;

  void raw_collect_integer (int regnum, gdb_byte *addr, int addr_len,
			    bool is_signed) const;

  void raw_supply (int regnum, const void *buf);

  void raw_supply_integer (int regnum, const gdb_byte *addr, int addr_len,
			   bool is_signed);

  void raw_supply_zeroed (int regnum);

  enum register_status get_register_status (int regnum) const;

  void raw_set_cached_value (int regnum, const gdb_byte *buf);

  void invalidate (int regnum);

  enum register_status raw_read_part (int regnum, int offset, int len,
				      gdb_byte *buf);

  void raw_write_part (int regnum, int offset, int len, const gdb_byte *buf);

  enum register_status cooked_read_part (int regnum, int offset, int len,
					 gdb_byte *buf);

  void cooked_write_part (int regnum, int offset, int len,
			  const gdb_byte *buf);

  void supply_regset (const struct regset *regset,
		      int regnum, const void *buf, size_t size);


  void collect_regset (const struct regset *regset, int regnum,
		       void *buf, size_t size) const;

  void dump (ui_file *file, enum regcache_dump_what what_to_dump);

  ptid_t ptid () const
  {
    return m_ptid;
  }

  void set_ptid (const ptid_t ptid)
  {
    this->m_ptid = ptid;
  }

/* Dump the contents of a register from the register cache to the target
   debug.  */
  void debug_print_register (const char *func, int regno);

  static void regcache_thread_ptid_changed (ptid_t old_ptid, ptid_t new_ptid);
protected:
  regcache (gdbarch *gdbarch, address_space *aspace_, bool readonly_p_);

  static std::forward_list<regcache *> current_regcache;

private:
  gdb_byte *register_buffer (int regnum) const;

  void restore (struct regcache *src);

  void cpy_no_passthrough (struct regcache *src);

  enum register_status xfer_part (int regnum, int offset, int len, void *in,
				  const void *out,
				  decltype (regcache_raw_read) read,
				  decltype (regcache_raw_write) write);

  void transfer_regset (const struct regset *regset,
			struct regcache *out_regcache,
			int regnum, const void *in_buf,
			void *out_buf, size_t size) const;

  struct regcache_descr *m_descr;

  /* The address space of this register cache (for registers where it
     makes sense, like PC or SP).  */
  struct address_space *m_aspace;

  /* The register buffers.  A read-only register cache can hold the
     full [0 .. gdbarch_num_regs + gdbarch_num_pseudo_regs) while a read/write
     register cache can only hold [0 .. gdbarch_num_regs).  */
  gdb_byte *m_registers;
  /* Register cache status.  */
  signed char *m_register_status;
  /* Is this a read-only cache?  A read-only cache is used for saving
     the target's register state (e.g, across an inferior function
     call or just before forcing a function return).  A read-only
     cache can only be updated via the methods regcache_dup() and
     regcache_cpy().  The actual contents are determined by the
     reggroup_save and reggroup_restore methods.  */
  bool m_readonly_p;
  /* If this is a read-write cache, which thread's registers is
     it connected to?  */
  ptid_t m_ptid;

  friend struct regcache *
  get_thread_arch_aspace_regcache (ptid_t ptid, struct gdbarch *gdbarch,
				   struct address_space *aspace);

  friend void
  registers_changed_ptid (ptid_t ptid);

  friend void
  regcache_cpy (struct regcache *dst, struct regcache *src);
};

/* Copy/duplicate the contents of a register cache.  By default, the
   operation is pass-through.  Writes to DST and reads from SRC will
   go through to the target.  See also regcache_cpy_no_passthrough.

   regcache_cpy can not have overlapping SRC and DST buffers.  */

extern struct regcache *regcache_dup (struct regcache *regcache);
extern void regcache_cpy (struct regcache *dest, struct regcache *src);

extern void registers_changed (void);
extern void registers_changed_ptid (ptid_t);

#endif /* REGCACHE_H */
