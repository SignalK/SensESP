/* Builds the upstream esp_picotts.c with run-time voice selection.
 *
 * Upstream embeds exactly one voice and refers to it through two array
 * symbols, ta_bin_start and sg_bin_start. Here those names are redefined to
 * dereference a pointer instead, so that upstream's declarations
 *
 *   extern const char ta_bin_start[] asm("_binary_picotts_ta_bin_start");
 *
 * turn into declarations of pointer variables, which are defined at the end
 * of this file and set by picotts_set_language(). This keeps the upstream
 * sources unmodified.
 */
#include "picotts_lang.h"

#include <string.h>

#define ta_bin_start (*picotts_ta_selected)
#define sg_bin_start (*picotts_sg_selected)

#include "esp-picotts/esp_picotts.c"

#undef ta_bin_start
#undef sg_bin_start

#define PICOTTS_VOICE_DECL(id) \
  extern const char picotts_##id##_ta[]; \
  extern const char picotts_##id##_sg[];
#define PICOTTS_VOICE(lang, id) \
  { lang, picotts_##id##_ta, picotts_##id##_sg },

#ifdef PICOTTS_LANG_EN_GB
PICOTTS_VOICE_DECL(en_GB)
#endif
#ifdef PICOTTS_LANG_EN_US
PICOTTS_VOICE_DECL(en_US)
#endif
#ifdef PICOTTS_LANG_DE_DE
PICOTTS_VOICE_DECL(de_DE)
#endif
#ifdef PICOTTS_LANG_ES_ES
PICOTTS_VOICE_DECL(es_ES)
#endif
#ifdef PICOTTS_LANG_FR_FR
PICOTTS_VOICE_DECL(fr_FR)
#endif
#ifdef PICOTTS_LANG_IT_IT
PICOTTS_VOICE_DECL(it_IT)
#endif

static const struct {
  const char *language;
  const char *ta;
  const char *sg;
} voices[] = {
#ifdef PICOTTS_LANG_EN_GB
  PICOTTS_VOICE("en-GB", en_GB)
#endif
#ifdef PICOTTS_LANG_EN_US
  PICOTTS_VOICE("en-US", en_US)
#endif
#ifdef PICOTTS_LANG_DE_DE
  PICOTTS_VOICE("de-DE", de_DE)
#endif
#ifdef PICOTTS_LANG_ES_ES
  PICOTTS_VOICE("es-ES", es_ES)
#endif
#ifdef PICOTTS_LANG_FR_FR
  PICOTTS_VOICE("fr-FR", fr_FR)
#endif
#ifdef PICOTTS_LANG_IT_IT
  PICOTTS_VOICE("it-IT", it_IT)
#endif
};

const char (*picotts_ta_selected)[] = NULL;
const char (*picotts_sg_selected)[] = NULL;

bool picotts_set_language(const char *language)
{
  for (unsigned i = 0; i < sizeof(voices) / sizeof(voices[0]); ++i)
  {
    if (strcmp(voices[i].language, language) == 0)
    {
      picotts_ta_selected = (const void *)voices[i].ta;
      picotts_sg_selected = (const void *)voices[i].sg;
      return true;
    }
  }
  ESP_LOGE(tag, "Language '%s' is not built in", language);
  return false;
}

// Select the default voice before app_main()/setup() runs.
__attribute__((constructor)) static void picotts_select_default(void)
{
  picotts_set_language(PICOTTS_DEFAULT_LANGUAGE);
}
