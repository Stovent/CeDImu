#ifndef M68000_H
#define M68000_H

#include <stdbool.h>
#include <stdint.h>

/**
 * Exception vectors of the 68000.
 *
 * You can directly cast the enum to u8 to get the vector number.
 * ```
 * use m68000::exception::Vector;
 * assert_eq!(Vector::AccessError as u8, 2);
 * ```
 *
 * The `FormatError` and `OnChipInterrupt` vectors are only used by the SCC68070.
 */
typedef enum Vector
{
    ResetSspPc = 0,
    /**
     * Bus error. Sent when the accessed address is not in the memory range of the system.
     */
    AccessError = 2,
    AddressError,
    IllegalInstruction,
    ZeroDivide,
    ChkInstruction,
    TrapVInstruction,
    PrivilegeViolation,
    Trace,
    FormatError = 14,
    UninitializedInterrupt,
    /**
     * The spurious interrupt vector is taken when there is a bus error indication during interrupt processing.
     */
    SpuriousInterrupt = 24,
    Level1Interrupt,
    Level2Interrupt,
    Level3Interrupt,
    Level4Interrupt,
    Level5Interrupt,
    Level6Interrupt,
    Level7Interrupt,
    Trap0Instruction,
    Trap1Instruction,
    Trap2Instruction,
    Trap3Instruction,
    Trap4Instruction,
    Trap5Instruction,
    Trap6Instruction,
    Trap7Instruction,
    Trap8Instruction,
    Trap9Instruction,
    Trap10Instruction,
    Trap11Instruction,
    Trap12Instruction,
    Trap13Instruction,
    Trap14Instruction,
    Trap15Instruction,
    Level1OnChipInterrupt = 57,
    Level2OnChipInterrupt,
    Level3OnChipInterrupt,
    Level4OnChipInterrupt,
    Level5OnChipInterrupt,
    Level6OnChipInterrupt,
    Level7OnChipInterrupt,
    UserInterrupt,
} Vector;

/**
 * A M68000 core.
 */
typedef struct M68000 M68000;

#if defined(DOC)
/**
 * Return type of the memory callback functions.
 */
typedef struct GetSetResult
{
    /**
     * Set to the value to be returned. Only the low order bytes are read depending on the size. Unused with SetResult.
     */
    uint32_t data;
    /**
     * Set to 0 if read successfully, set to 2 (Access Error) otherwise (Address errors are automatically detected by the library).
     */
    uint8_t exception;
} GetSetResult;
#endif

#if !defined(DOC)
/**
 * Return type of the memory callback functions.
 */
typedef struct GetSetResult
{
    /**
     * Set to the value to be returned. Only the low order bytes are read depending on the size. Unused with SetResult.
     */
    uint32_t data;
    /**
     * Set to 0 if read successfully, set to 2 (Access Error) otherwise (Address errors are automatically detected by the library).
     */
    uint8_t exception;
} GetSetResult;
#endif

#if defined(DOC)
/**
 * Memory callbacks sent to the interpreter methods.
 *
 * The void* argument passed on each callback is the `user_data` member, and its usage is let to the user of this library.
 * For example, this can be used to allow the usage of C++ objects, where `user_data` has the value of the `this` pointer of the object.
 */
typedef struct M68000Callbacks
{
    struct GetSetResult (*get_byte)(uint32_t addr, void *user_data);
    struct GetSetResult (*get_word)(uint32_t addr, void *user_data);
    struct GetSetResult (*get_long)(uint32_t addr, void *user_data);
    struct GetSetResult (*set_byte)(uint32_t addr, uint8_t data, void *user_data);
    struct GetSetResult (*set_word)(uint32_t addr, uint16_t data, void *user_data);
    struct GetSetResult (*set_long)(uint32_t addr, uint32_t data, void *user_data);
    void (*reset_instruction)(void*);
    void *user_data;
} M68000Callbacks;
#endif

#if !defined(DOC)
/**
 * Memory callbacks sent to the interpreter methods.
 *
 * The void* argument passed on each callback is the `user_data` member, and its usage is let to the user of this library.
 * For example, this can be used to allow the usage of C++ objects, where `user_data` has the value of the `this` pointer of the object.
 */
typedef struct M68000Callbacks
{
    struct GetSetResult (*get_byte)(uint32_t addr, void *user_data);
    struct GetSetResult (*get_word)(uint32_t addr, void *user_data);
    struct GetSetResult (*get_long)(uint32_t addr, void *user_data);
    struct GetSetResult (*set_byte)(uint32_t addr, uint8_t data, void *user_data);
    struct GetSetResult (*set_word)(uint32_t addr, uint16_t data, void *user_data);
    struct GetSetResult (*set_long)(uint32_t addr, uint32_t data, void *user_data);
    void (*reset_instruction)(void*);
    void *user_data;
} M68000Callbacks;
#endif

#if defined(DOC)
/**
 * Return value of the `cycle_until_exception`, `loop_until_exception_stop` and `interpreter_exception` functions.
 */
typedef struct ExceptionResult
{
    /**
     * The number of cycles executed.
     */
    size_t cycles;
    /**
     * 0 if no exception occured, the vector number that occured otherwise.
     */
    uint8_t exception;
} ExceptionResult;
#endif

#if !defined(DOC)
/**
 * Return value of the `cycle_until_exception`, `loop_until_exception_stop` and `interpreter_exception` functions.
 */
typedef struct ExceptionResult
{
    /**
     * The number of cycles executed.
     */
    size_t cycles;
    /**
     * 0 if no exception occured, the vector number that occured otherwise.
     */
    uint8_t exception;
} ExceptionResult;
#endif

/**
 * M68000 status register.
 */
typedef struct StatusRegister
{
    /**
     * Trace
     */
    bool t;
    /**
     * Supervisor
     */
    bool s;
    /**
     * Interrupt Priority Mask
     */
    uint8_t interrupt_mask;
    /**
     * Extend
     */
    bool x;
    /**
     * Negate
     */
    bool n;
    /**
     * Zero
     */
    bool z;
    /**
     * Overflow
     */
    bool v;
    /**
     * Carry
     */
    bool c;
} StatusRegister;

/**
 * M68000 registers.
 */
typedef struct Registers
{
    /**
     * Data registers.
     */
    uint32_t d[8];
    /**
     * Address registers.
     */
    uint32_t a[7];
    /**
     * User Stack Pointer.
     */
    uint32_t usp;
    /**
     * System Stack Pointer.
     */
    uint32_t ssp;
    /**
     * Status Register.
     */
    struct StatusRegister sr;
    /**
     * Program Counter.
     */
    uint32_t pc;
} Registers;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#if defined(DOC)
/**
 * Allocates a new core and returns the pointer to it.
 *
 * The created core has a [Reset vector](crate::exception::Vector::ResetSspPc) pushed, so that the first call to an interpreter method
 * will first fetch the reset vectors, then will execute the first instruction.
 *
 * It is not managed by Rust, so you have to delete it after usage with [m68000_delete].
 */
struct M68000 *m68000_new(void);
#endif

#if defined(DOC)
/**
 * [m68000_new] but without the initial reset vector, so you can initialize the core as you want.
 */
struct M68000 *m68000_new_no_reset(void);
#endif

#if defined(DOC)
/**
 * Frees the memory of the given core.
 */
void m68000_delete(struct M68000 *m68000);
#endif

#if defined(DOC)
/**
 * Runs the CPU for `cycles` number of cycles.
 *
 * This function executes **at least** the given number of cycles.
 * Returns the number of cycles actually executed.
 *
 * If you ask to execute 4 cycles but the next instruction takes 6 cycles to execute,
 * it will be executed and the 2 extra cycles will be subtracted in the next call.
 */
size_t m68000_cycle(struct M68000 *m68000, struct M68000Callbacks *memory, size_t cycles);
#endif

#if defined(DOC)
/**
 * Runs the CPU until either an exception occurs or `cycle` cycles have been executed.
 *
 * This function executes **at least** the given number of cycles.
 * Returns the number of cycles actually executed, and the exception that occured if any.
 *
 * If you ask to execute 4 cycles but the next instruction takes 6 cycles to execute,
 * it will be executed and the 2 extra cycles will be subtracted in the next call.
 */
struct ExceptionResult m68000_cycle_until_exception(struct M68000 *m68000, struct M68000Callbacks *memory, size_t cycles);
#endif

#if defined(DOC)
/**
 * Runs indefinitely until an exception or STOP instruction occurs.
 *
 * Returns the number of cycles executed and the exception that occured.
 * If exception is None, this means the CPU has executed a STOP instruction.
 */
struct ExceptionResult m68000_loop_until_exception_stop(struct M68000 *m68000, struct M68000Callbacks *memory);
#endif

#if defined(DOC)
/**
 * Executes the next instruction, returning the cycle count necessary to execute it.
 */
size_t m68000_interpreter(struct M68000 *m68000, struct M68000Callbacks *memory);
#endif

#if defined(DOC)
/**
 * Executes the next instruction, returning the cycle count necessary to execute it,
 * and the vector of the exception that occured during the execution if any.
 *
 * To process the returned exception, call [M68000::exception].
 */
struct ExceptionResult m68000_interpreter_exception(struct M68000 *m68000, struct M68000Callbacks *memory);
#endif

#if defined(DOC)
/**
 * Executes and disassembles the next instruction, returning the disassembler string and the cycle count necessary to execute it.
 *
 * `str` is a pointer to a C string buffer where the disassembled instruction will be written.
 * `len` is the maximum size of the buffer.
 */
size_t m68000_disassembler_interpreter(struct M68000 *m68000, struct M68000Callbacks *memory, char *str, size_t len);
#endif

#if defined(DOC)
/**
 * Executes and disassembles the next instruction, returning the disassembled string, the cycle count necessary to execute it,
 * and the vector of the exception that occured during the execution if any.
 *
 * To process the returned exception, call [M68000::exception].
 *
 * `str` is a pointer to a C string buffer where the disassembled instruction will be written.
 * `len` is the maximum size of the buffer.
 */
struct ExceptionResult m68000_disassembler_interpreter_exception(struct M68000 *m68000, struct M68000Callbacks *memory, char *str, size_t len);
#endif

#if defined(DOC)
/**
 * Requests the CPU to process the given exception vector.
 */
void m68000_exception(struct M68000 *m68000, enum Vector vector);
#endif

#if defined(DOC)
/**
 * Returns the 16-bits word at the current PC value of the given core.
 */
struct GetSetResult m68000_peek_next_word(struct M68000 *m68000, struct M68000Callbacks *memory);
#endif

#if defined(DOC)
/**
 * Returns a mutable pointer to the registers of the given core.
 */
struct Registers *m68000_registers(struct M68000 *m68000);
#endif

#if defined(DOC)
/**
 * Returns a copy of the registers of the given core.
 */
struct Registers m68000_get_registers(const struct M68000 *m68000);
#endif

#if defined(DOC)
/**
 * Sets the registers of the core to the given value.
 */
void m68000_set_registers(struct M68000 *m68000, struct Registers regs);
#endif

#if !defined(DOC)
/**
 * Allocates a new core and returns the pointer to it.
 *
 * The created core has a [Reset vector](crate::exception::Vector::ResetSspPc) pushed, so that the first call to an interpreter method
 * will first fetch the reset vectors, then will execute the first instruction.
 *
 * It is not managed by Rust, so you have to delete it after usage with [m68000_delete].
 */
struct M68000 *m68000_new(void);
#endif

#if !defined(DOC)
/**
 * [m68000_new] but without the initial reset vector, so you can initialize the core as you want.
 */
struct M68000 *m68000_new_no_reset(void);
#endif

#if !defined(DOC)
/**
 * Frees the memory of the given core.
 */
void m68000_delete(struct M68000 *m68000);
#endif

#if !defined(DOC)
/**
 * Runs the CPU for `cycles` number of cycles.
 *
 * This function executes **at least** the given number of cycles.
 * Returns the number of cycles actually executed.
 *
 * If you ask to execute 4 cycles but the next instruction takes 6 cycles to execute,
 * it will be executed and the 2 extra cycles will be subtracted in the next call.
 */
size_t m68000_cycle(struct M68000 *m68000, struct M68000Callbacks *memory, size_t cycles);
#endif

#if !defined(DOC)
/**
 * Runs the CPU until either an exception occurs or `cycle` cycles have been executed.
 *
 * This function executes **at least** the given number of cycles.
 * Returns the number of cycles actually executed, and the exception that occured if any.
 *
 * If you ask to execute 4 cycles but the next instruction takes 6 cycles to execute,
 * it will be executed and the 2 extra cycles will be subtracted in the next call.
 */
struct ExceptionResult m68000_cycle_until_exception(struct M68000 *m68000, struct M68000Callbacks *memory, size_t cycles);
#endif

#if !defined(DOC)
/**
 * Runs indefinitely until an exception or STOP instruction occurs.
 *
 * Returns the number of cycles executed and the exception that occured.
 * If exception is None, this means the CPU has executed a STOP instruction.
 */
struct ExceptionResult m68000_loop_until_exception_stop(struct M68000 *m68000, struct M68000Callbacks *memory);
#endif

#if !defined(DOC)
/**
 * Executes the next instruction, returning the cycle count necessary to execute it.
 */
size_t m68000_interpreter(struct M68000 *m68000, struct M68000Callbacks *memory);
#endif

#if !defined(DOC)
/**
 * Executes the next instruction, returning the cycle count necessary to execute it,
 * and the vector of the exception that occured during the execution if any.
 *
 * To process the returned exception, call [M68000::exception].
 */
struct ExceptionResult m68000_interpreter_exception(struct M68000 *m68000, struct M68000Callbacks *memory);
#endif

#if !defined(DOC)
/**
 * Executes and disassembles the next instruction, returning the disassembler string and the cycle count necessary to execute it.
 *
 * `str` is a pointer to a C string buffer where the disassembled instruction will be written.
 * `len` is the maximum size of the buffer.
 */
size_t m68000_disassembler_interpreter(struct M68000 *m68000, struct M68000Callbacks *memory, char *str, size_t len);
#endif

#if !defined(DOC)
/**
 * Executes and disassembles the next instruction, returning the disassembled string, the cycle count necessary to execute it,
 * and the vector of the exception that occured during the execution if any.
 *
 * To process the returned exception, call [M68000::exception].
 *
 * `str` is a pointer to a C string buffer where the disassembled instruction will be written.
 * `len` is the maximum size of the buffer.
 */
struct ExceptionResult m68000_disassembler_interpreter_exception(struct M68000 *m68000, struct M68000Callbacks *memory, char *str, size_t len);
#endif

#if !defined(DOC)
/**
 * Requests the CPU to process the given exception vector.
 */
void m68000_exception(struct M68000 *m68000, enum Vector vector);
#endif

#if !defined(DOC)
/**
 * Returns the 16-bits word at the current PC value of the given core.
 */
struct GetSetResult m68000_peek_next_word(struct M68000 *m68000, struct M68000Callbacks *memory);
#endif

#if !defined(DOC)
/**
 * Returns a mutable pointer to the registers of the given core.
 */
struct Registers *m68000_registers(struct M68000 *m68000);
#endif

#if !defined(DOC)
/**
 * Returns a copy of the registers of the given core.
 */
struct Registers m68000_get_registers(const struct M68000 *m68000);
#endif

#if !defined(DOC)
/**
 * Sets the registers of the core to the given value.
 */
void m68000_set_registers(struct M68000 *m68000, struct Registers regs);
#endif

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif /* M68000_H */
