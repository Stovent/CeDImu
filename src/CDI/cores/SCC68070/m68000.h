#ifndef M68000_H
#define M68000_H

#include <stdbool.h>
#include <stdint.h>

/**
 * A M68000 core.
 */
typedef struct M68000 M68000;

typedef struct GetSetResult
{
    /**
     * Set to the value to be returned. Only the low order bytes are read depending on the size. Unused with SetResult.
     */
    uint32_t data;
    /**
     * Set to 0 if read successfully, set to the exception vector otherwise (Access or Address error).
     */
    uint8_t exception;
} GetSetResult;

/**
 * Memory callbacks sent to the interpreter methods.
 *
 * The void* argument passed on each callback is [Self::user_data], and usage is let to the user of this library.
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
    void (*disassembler)(uint32_t, const char*, void*);
    void *user_data;
} M68000Callbacks;

/**
 * Return value of the `cycle_until_exception`, `loop_until_exception_stop` and `interpreter_exception` methods.
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

struct M68000 *m68000_new(void);

void m68000_delete(struct M68000 *m68000);

size_t m68000_cycle(struct M68000 *m68000, struct M68000Callbacks *memory, size_t cycles);

struct ExceptionResult m68000_cycle_until_exception(struct M68000 *m68000, struct M68000Callbacks *memory, size_t cycles);

struct ExceptionResult m68000_loop_until_exception_stop(struct M68000 *m68000, struct M68000Callbacks *memory);

size_t m68000_interpreter(struct M68000 *m68000, struct M68000Callbacks *memory);

struct ExceptionResult m68000_interpreter_exception(struct M68000 *m68000, struct M68000Callbacks *memory);

void m68000_exception(struct M68000 *m68000, uint8_t vector);

struct GetSetResult m68000_peek_next_word(struct M68000 *m68000, struct M68000Callbacks *memory);

struct Registers *m68000_registers(struct M68000 *m68000);

struct Registers m68000_get_registers(const struct M68000 *m68000);

void m68000_set_registers(struct M68000 *m68000, struct Registers regs);

void m68000_enable_disassembler(struct M68000 *m68000, bool enabled);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif /* M68000_H */
