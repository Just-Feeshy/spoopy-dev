#ifndef SPOOPY_CONTEXT_H
#define SPOOPY_CONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct spoopy_context spoopy_context_t;

int spoopy_new_context(spoopy_context_t* context);

#ifdef __cplusplus
}
#endif // extern "C"

#endif
