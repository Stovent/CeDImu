#ifndef M68000_H
#define M68000_H

#include <stdbool.h>
#include <stdint.h>

/**
 * A M68000 core.
 */
typedef struct M68000 M68000;

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

/**
 * Memory callbacks sent to the interpreter methods.
 *
 * The void* argument passed on each callback is the `user_data` member, and its usage is let to the user of this library.
 * For example, this can be used to allow the usage of C++ objects, where `user_data` has the value of the `this` pointer of the object.
 */
typedef struct M68000Callbacks
{
    struct GetSetResult (*get_byte)(uint32_t, void*);
    struct GetSetResult (*get_word)(uint32_t, void*);
    struct GetSetResult (*get_long)(uint32_t, void*);
    struct GetSetResult (*set_byte)(uint32_t, uint8_t, void*);
    struct GetSetResult (*set_word)(uint32_t, uint16_t, void*);
    struct GetSetResult (*set_long)(uint32_t, uint32_t, void*);
    void (*reset_instruction)(void*);
    void *user_data;
} M68000Callbacks;

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

/**
 * Allocates a new core and returns the pointer to it. Is is unmanaged by Rust, so you have to delete it after usage.
 */
struct M68000 *m68000_new(void);

/**
 * Frees the memory of the given core.
 */
void m68000_delete(struct M68000 *m68000);

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

/**
 * Runs indefinitely until an exception or STOP instruction occurs.
 *
 * Returns the number of cycles executed and the exception that occured.
 * If exception is None, this means the CPU has executed a STOP instruction.
 */
struct ExceptionResult m68000_loop_until_exception_stop(struct M68000 *m68000, struct M68000Callbacks *memory);

/**
 * Executes the next instruction, returning the cycle count necessary to execute it.
 */
size_t m68000_interpreter(struct M68000 *m68000, struct M68000Callbacks *memory);

/**
 * Executes the next instruction, returning the cycle count necessary to execute it,
 * and the vector of the exception that occured during the execution if any.
 *
 * To process the returned exception, call [M68000::exception].
 */
struct ExceptionResult m68000_interpreter_exception(struct M68000 *m68000, struct M68000Callbacks *memory);

/**
 * Executes and disassembles the next instruction, returning the disassembler string and the cycle count necessary to execute it.
 *
 * `str` is a pointer to a C string buffer where the disassembled instruction will be written.
 * `len` is the maximum size of the buffer.
 */
size_t m68000_disassembler_interpreter(struct M68000 *m68000, struct M68000Callbacks *memory, char *str, size_t len);

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

/**
 * Requests the CPU to process the given exception vector.
 */
void m68000_exception(struct M68000 *m68000, uint8_t vector);

/**
 * Returns the 16-bits word at the current PC value of the given core.
 */
struct GetSetResult m68000_peek_next_word(struct M68000 *m68000, struct M68000Callbacks *memory);

/**
 * Returns a mutable pointer to the registers of the given core.
 */
struct Registers *m68000_registers(struct M68000 *m68000);

/**
 * Returns a copy of the registers of the given core.
 */
struct Registers m68000_get_registers(const struct M68000 *m68000);

/**
 * Sets the registers of the core to the given value.
 */
void m68000_set_registers(struct M68000 *m68000, struct Registers regs);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif /* M68000_H */
