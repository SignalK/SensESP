# Build settings for the esp-picotts sources. These mirror what the upstream
# CMakeLists.txt and Kconfig do for ESP-IDF builds.
#
# The voices to build in are listed in platformio.ini with, e.g.:
#   custom_picotts_languages = en-US, it-IT
# Valid values: en-GB, en-US, de-DE, es-ES, fr-FR, it-IT. The first one is
# the default; the others are selected with picotts_set_language().

import os

Import("env")

VOICES = {
    "en-GB": ("en-GB_ta.bin", "en-GB_kh0_sg.bin"),
    "en-US": ("en-US_ta.bin", "en-US_lh0_sg.bin"),
    "de-DE": ("de-DE_ta.bin", "de-DE_gl0_sg.bin"),
    "es-ES": ("es-ES_ta.bin", "es-ES_zl0_sg.bin"),
    "fr-FR": ("fr-FR_ta.bin", "fr-FR_nk0_sg.bin"),
    "it-IT": ("it-IT_ta.bin", "it-IT_cm0_sg.bin"),
}

# PlatformIO runs library scripts from the library directory.
lib_dir = os.getcwd()
src_dir = os.path.join(lib_dir, "esp-picotts")
lang_dir = os.path.join(src_dir, "pico", "lang")

if not os.path.isfile(os.path.join(src_dir, "esp_picotts.c")):
    raise SystemExit(
        "picotts: lib/picotts/esp-picotts is empty. "
        "Run: git submodule update --init"
    )

languages = [
    lang.strip()
    for lang in env.GetProjectOption("custom_picotts_languages", "en-US")
    .replace("\n", ",")
    .split(",")
    if lang.strip()
]
for language in languages:
    if language not in VOICES:
        raise SystemExit(
            "picotts: unknown language '%s' in custom_picotts_languages "
            "(valid: %s)" % (language, ", ".join(sorted(VOICES)))
        )

defines = [
    ("CONFIG_PICOTTS_RESOURCE_MODE_EMBED", 1),
    ("CONFIG_PICOTTS_INPUT_QUEUE_SIZE", 256),
    ("PICOTTS_DEFAULT_LANGUAGE", env.StringifyMacro(languages[0])),
]
for language in languages:
    # Paths of the voice resources embedded by picotts_resources.S
    ident = language.upper().replace("-", "_")
    ta_bin, sg_bin = VOICES[language]
    defines += [
        ("PICOTTS_LANG_" + ident, 1),
        ("PICOTTS_%s_TA" % ident,
         env.StringifyMacro(os.path.join(lang_dir, ta_bin))),
        ("PICOTTS_%s_SG" % ident,
         env.StringifyMacro(os.path.join(lang_dir, sg_bin))),
    ]

env.Append(
    CPPPATH=[src_dir, os.path.join(src_dir, "pico", "lib")],
    CPPDEFINES=defines,
    CCFLAGS=[
        "-Wno-implicit-fallthrough",
        "-Wno-unused-but-set-variable",
        "-Wno-unused-function",
    ],
)


# PicoTTS's quick exp() relies on a floating point representation that does
# not hold on Xtensa. esp_picotts.c provides a replacement, so rename the
# original out of the way.
def rename_quick_exp(env, node):
    return env.Object(
        node,
        CPPDEFINES=list(env["CPPDEFINES"])
        + [("picoos_quick_exp", "picoos_quick_nope")],
    )


env.AddBuildMiddleware(rename_quick_exp, "*/pico/lib/picoos.c")
