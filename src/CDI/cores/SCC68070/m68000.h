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

typedef struct M68000Callbacks
{
    struct GetSetResult (*get_byte)(uint32_t);
    struct GetSetResult (*get_word)(uint32_t);
    struct GetSetResult (*get_long)(uint32_t);
    struct GetSetResult (*set_byte)(uint32_t, uint8_t);
    struct GetSetResult (*set_word)(uint32_t, uint16_t);
    struct GetSetResult (*set_long)(uint32_t, uint32_t);
    void (*reset_instruction)(void);
    void (*disassembler)(uint32_t, const char*);
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

void m68000_enable_disassembler(struct M68000 *m68000, bool enabled);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif /* M68000_H */
