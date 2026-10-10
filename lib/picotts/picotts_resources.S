/* Embeds the PicoTTS voice resources into the application binary. Which
 * voices are included, and their file paths, is set by build.py. */

    .section .rodata.embedded, "a"

#ifdef PICOTTS_LANG_EN_GB
    .global picotts_en_GB_ta
    .balign 4
picotts_en_GB_ta:
    .incbin PICOTTS_EN_GB_TA
    .global picotts_en_GB_sg
    .balign 4
picotts_en_GB_sg:
    .incbin PICOTTS_EN_GB_SG
#endif

#ifdef PICOTTS_LANG_EN_US
    .global picotts_en_US_ta
    .balign 4
picotts_en_US_ta:
    .incbin PICOTTS_EN_US_TA
    .global picotts_en_US_sg
    .balign 4
picotts_en_US_sg:
    .incbin PICOTTS_EN_US_SG
#endif

#ifdef PICOTTS_LANG_DE_DE
    .global picotts_de_DE_ta
    .balign 4
picotts_de_DE_ta:
    .incbin PICOTTS_DE_DE_TA
    .global picotts_de_DE_sg
    .balign 4
picotts_de_DE_sg:
    .incbin PICOTTS_DE_DE_SG
#endif

#ifdef PICOTTS_LANG_ES_ES
    .global picotts_es_ES_ta
    .balign 4
picotts_es_ES_ta:
    .incbin PICOTTS_ES_ES_TA
    .global picotts_es_ES_sg
    .balign 4
picotts_es_ES_sg:
    .incbin PICOTTS_ES_ES_SG
#endif

#ifdef PICOTTS_LANG_FR_FR
    .global picotts_fr_FR_ta
    .balign 4
picotts_fr_FR_ta:
    .incbin PICOTTS_FR_FR_TA
    .global picotts_fr_FR_sg
    .balign 4
picotts_fr_FR_sg:
    .incbin PICOTTS_FR_FR_SG
#endif

#ifdef PICOTTS_LANG_IT_IT
    .global picotts_it_IT_ta
    .balign 4
picotts_it_IT_ta:
    .incbin PICOTTS_IT_IT_TA
    .global picotts_it_IT_sg
    .balign 4
picotts_it_IT_sg:
    .incbin PICOTTS_IT_IT_SG
#endif
