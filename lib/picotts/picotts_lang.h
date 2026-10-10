#ifndef PICOTTS_LANG_H
#define PICOTTS_LANG_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Selects the voice used by the next call to picotts_init(). To change the
 * language of a running engine, call picotts_shutdown() first.
 *
 * Only the languages listed in `custom_picotts_languages` in platformio.ini
 * are built in. Until this is called, the first of them is used.
 *
 * @param language Language code, e.g. "en-US" or "it-IT".
 * @returns True if the language is built in, false otherwise.
 */
bool picotts_set_language(const char *language);

#ifdef __cplusplus
}
#endif
#endif
