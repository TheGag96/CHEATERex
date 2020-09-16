#ifdef EXT_OPTIONS_MENU

#include "sm64.h"
#include "include/text_strings.h"
#include "engine/math_util.h"
#include "audio/external.h"
#include "game/camera.h"
#include "game/level_update.h"
#include "game/print.h"
#include "game/segment2.h"
#include "game/save_file.h"
#ifdef BETTERCAMERA
#include "game/bettercamera.h"
#endif
#include "game/mario_misc.h"
#include "game/game_init.h"
#include "game/ingame_menu.h"
#include "game/options_menu.h"
#include "pc/pc_main.h"
#include "pc/cliopts.h"
#include "pc/cheats.h"
#include "pc/configfile.h"
#include "pc/controller/controller_api.h"

#include <stdbool.h>
#include "../../include/libc/stdlib.h"

u8 optmenu_open = 0;

static u8 optmenu_binding = 0;
static u8 optmenu_bind_idx = 0;

// How to add stuff:
// strings: add them to include/text_strings.h.in
//          and to menuStr[] / opts*Str[]
// options: add them to the relevant options list
// menus:   add a new submenu definition and a new
//          option to the optsMain list

static const u8 toggleStr[][16] = {
    { TEXT_OPT_DISABLED },
    { TEXT_OPT_ENABLED },
};

static const u8 menuStr[][32] = {
    { TEXT_OPT_HIGHLIGHT },
    { TEXT_OPT_BUTTON1 },
    { TEXT_OPT_BUTTON2 },
    { TEXT_OPT_OPTIONS },
    { TEXT_OPT_CAMERA },
    { TEXT_OPT_CHEATS },
    { TEXT_OPT_CONTROLS },
    { TEXT_OPT_VIDEO },
    { TEXT_OPT_AUDIO },
    { TEXT_EXIT_GAME },

};

static const u8 optsCameraStr[][32] = {
    { TEXT_OPT_CAMX },
    { TEXT_OPT_CAMY },
    { TEXT_OPT_INVERTX },
    { TEXT_OPT_INVERTY },
    { TEXT_OPT_CAMC },
    { TEXT_OPT_CAMP },
    { TEXT_OPT_ANALOGUE },
    { TEXT_OPT_MOUSE },
    { TEXT_OPT_CAMD },
    { TEXT_OPT_CAMON },
};

static const u8 optsVideoStr[][32] = {
    { TEXT_OPT_FSCREEN },
    { TEXT_OPT_TEXFILTER },
    { TEXT_OPT_NEAREST },
    { TEXT_OPT_LINEAR },
    { TEXT_OPT_RESETWND },
    { TEXT_OPT_VSYNC },
    { TEXT_OPT_AUTO },
    { TEXT_OPT_HUD },
    { TEXT_OPT_THREEPT },
    { TEXT_OPT_APPLY },
};

static const u8 optsAudioStr[][32] = {
    { TEXT_OPT_MVOLUME },    
    { TEXT_OPT_MUSVOLUME },
    { TEXT_OPT_SFXVOLUME },
    { TEXT_OPT_ENVVOLUME },
};

static const u8 optsSeqStr[][64] = {
    { TEXT_OPT_SEQ1 },
    { TEXT_OPT_SEQ2 },
    { TEXT_OPT_SEQ3 },
    { TEXT_OPT_SEQ4 },
    { TEXT_OPT_SEQ5 },
    { TEXT_OPT_SEQ6 },
    { TEXT_OPT_SEQ7 },
    { TEXT_OPT_SEQ8 },
    { TEXT_OPT_SEQ9 },
    { TEXT_OPT_SEQ10 },
    { TEXT_OPT_SEQ11 },
    { TEXT_OPT_SEQ12 },
    { TEXT_OPT_SEQ13 },
    { TEXT_OPT_SEQ14 },
    { TEXT_OPT_SEQ15 },
    { TEXT_OPT_SEQ16 },
    { TEXT_OPT_SEQ17 },
    { TEXT_OPT_SEQ18 },
    { TEXT_OPT_SEQ19 },
};

static const u8 *SeqChoices[] = {
    optsSeqStr[0],
    optsSeqStr[1],
    optsSeqStr[2],
    optsSeqStr[3],
    optsSeqStr[4],
    optsSeqStr[5],
    optsSeqStr[6],
    optsSeqStr[7],
    optsSeqStr[8],
    optsSeqStr[9],
    optsSeqStr[10],
    optsSeqStr[11],
    optsSeqStr[12],
    optsSeqStr[13],
    optsSeqStr[14],
    optsSeqStr[15],
    optsSeqStr[16],
    optsSeqStr[17],
    optsSeqStr[18],
};

static const u8 optsSpeedStr[][16] = {
    { TEXT_OPT_SS1 },
    { TEXT_OPT_SS2 },
    { TEXT_OPT_SS3 },
    { TEXT_OPT_SS4 },
    { TEXT_OPT_SS5 },
};

static const u8 *SpeedChoices[] = {
    optsSpeedStr[0],
    optsSpeedStr[1],
    optsSpeedStr[2],
    optsSpeedStr[3],
    optsSpeedStr[4],
};

static const u8 optsCheatsStr[][64] = {
    { TEXT_OPT_CHEAT1 },
    { TEXT_OPT_CHEAT2 },
    { TEXT_OPT_CHEAT3 },
    { TEXT_OPT_CHEAT4 },
    { TEXT_OPT_CHEAT5 },
    { TEXT_OPT_CHEAT6 },
    { TEXT_OPT_CHEAT7 },
    { TEXT_OPT_CHEAT8 },
    { TEXT_OPT_CHEAT9 },
    { TEXT_OPT_NDB },
    { TEXT_OPT_JUMP },
    { TEXT_OPT_SPDDPS },
    { TEXT_OPT_TPF },
    { TEXT_OPT_JB },
    { TEXT_OPT_JBC },
    { TEXT_OPT_QUIKEND },
    { TEXT_OPT_HURT },
    { TEXT_OPT_CANN },
    { TEXT_OPT_AWK },
    { TEXT_OPT_SHELL },
    { TEXT_OPT_BOB },
    { TEXT_OPT_SPAMBA },
    { TEXT_OPT_SWIM },
    { TEXT_OPT_WING_CAP },
    { TEXT_OPT_METAL_CAP },
    { TEXT_OPT_VANISH_CAP },
    { TEXT_OPT_REMOVE_CAP },
    { TEXT_OPT_DCM },
    { TEXT_OPT_NORMAL_CAP },
    { TEXT_OPT_BLJ },
    { TEXT_OPT_PAC },
};

static const u8 optsPlayAsCheatStr[][32] = {
    { TEXT_OPT_PA1 },
    { TEXT_OPT_PA2 },
    { TEXT_OPT_PA3 },
    { TEXT_OPT_PA4 },
    { TEXT_OPT_PA5 },
    { TEXT_OPT_PA6 },
    { TEXT_OPT_PA7 },
};


static const u8 optsHurtCheatStr[][32] = {
    { TEXT_OPT_HURTCHT1 },
    { TEXT_OPT_HURTCHT2 },
    { TEXT_OPT_HURTCHT3 },
    { TEXT_OPT_HURTCHT4 },
};

static const u8 optsSpamCheatStr[][32] = {
    { TEXT_OPT_SPAMCHT1 },
    { TEXT_OPT_SPAMCHT2 },
    { TEXT_OPT_SPAMCHT3 },
    { TEXT_OPT_SPAMCHT4 },
    { TEXT_OPT_SPAMCHT5 },
    { TEXT_OPT_SPAMCHT6 },
    { TEXT_OPT_SPAMCHT7 },
    { TEXT_OPT_SPAMCHT8 },
    { TEXT_OPT_SPAMCHT9 },
    { TEXT_OPT_SPAMCHT10 },
    { TEXT_OPT_SPAMCHT11 },
    { TEXT_OPT_SPAMCHT12 },
    { TEXT_OPT_SPAMCHT13 },
    { TEXT_OPT_SPAMCHT14 },
};

static const u8 optsBLJCheatStr[][32] = {
    { TEXT_OPT_BLJCHT1 },
    { TEXT_OPT_BLJCHT2 },
    { TEXT_OPT_BLJCHT3 },
    { TEXT_OPT_BLJCHT4 },
    { TEXT_OPT_BLJCHT5 },
    { TEXT_OPT_BLJCHT6 },
    { TEXT_OPT_BLJCHT7 },
    { TEXT_OPT_BLJCHT8 },
    { TEXT_OPT_BLJCHT9 },
    { TEXT_OPT_BLJCHT10 },
    { TEXT_OPT_BLJCHT11 },
    { TEXT_OPT_BLJCHT12 },
    { TEXT_OPT_BLJCHT13 },
};

static const u8 *PlayAsCheatChoices[] = {
    optsPlayAsCheatStr[0],
    optsPlayAsCheatStr[1],
    optsPlayAsCheatStr[2],
    optsPlayAsCheatStr[3],
    optsPlayAsCheatStr[4],
    optsPlayAsCheatStr[5],
    optsPlayAsCheatStr[6],
};

static const u8 *HurtCheatChoices[] = {
    optsHurtCheatStr[0],
    optsHurtCheatStr[1],
    optsHurtCheatStr[2],
    optsHurtCheatStr[3],
};

static const u8 * SpamCheatChoices[] = {
    optsSpamCheatStr[0],
    optsSpamCheatStr[1],
    optsSpamCheatStr[2],
    optsSpamCheatStr[3],
    optsSpamCheatStr[4],
    optsSpamCheatStr[5],
    optsSpamCheatStr[6],
    optsSpamCheatStr[7],
    optsSpamCheatStr[8],
    optsSpamCheatStr[9],
    optsSpamCheatStr[10],
    optsSpamCheatStr[11],
    optsSpamCheatStr[12],
    optsSpamCheatStr[13],
};

static const u8* bljCheatChoices[] = {
    optsBLJCheatStr[0],
    optsBLJCheatStr[1],
    optsBLJCheatStr[2],
    optsBLJCheatStr[3],
    optsBLJCheatStr[4],
    optsBLJCheatStr[5],
    optsBLJCheatStr[6],
    optsBLJCheatStr[7],
    optsBLJCheatStr[8],
    optsBLJCheatStr[9],
    optsBLJCheatStr[10],
    optsBLJCheatStr[11],
    optsBLJCheatStr[12],
};

static const u8 bindStr[][32] = {
    { TEXT_OPT_UNBOUND },
    { TEXT_OPT_PRESSKEY },
    { TEXT_BIND_A },
    { TEXT_BIND_B },
    { TEXT_BIND_X },
    { TEXT_BIND_Y },
    { TEXT_BIND_START },
    { TEXT_BIND_L },
    { TEXT_BIND_R },
    { TEXT_BIND_Z },
    { TEXT_BIND_C_UP },
    { TEXT_BIND_C_DOWN },
    { TEXT_BIND_C_LEFT },
    { TEXT_BIND_C_RIGHT },
    { TEXT_BIND_D_UP },
    { TEXT_BIND_D_DOWN },
    { TEXT_BIND_D_LEFT },
    { TEXT_BIND_D_RIGHT },
    { TEXT_BIND_UP },
    { TEXT_BIND_DOWN },
    { TEXT_BIND_LEFT },
    { TEXT_BIND_RIGHT },
    { TEXT_OPT_DEADZONE },
    { TEXT_OPT_RUMBLE },
    { TEXT_OPT_TIGHT },
};

static const u8 *filterChoices[] = {
    optsVideoStr[2],
    optsVideoStr[3],
    optsVideoStr[8],
};

static const u8 *vsyncChoices[] = {
    toggleStr[0],
    toggleStr[1],
    optsVideoStr[6],
};

enum OptType {
    OPT_INVALID = 0,
    OPT_TOGGLE,
    OPT_CHOICE,
    OPT_SCROLL,
    OPT_SUBMENU,
    OPT_BIND,
    OPT_BUTTON,
};

struct SubMenu;

struct Option {
    enum OptType type;
    const u8 *label;
    union {
        u32 *uval;
        bool *bval;
    };
    union {
        struct {
            const u8 **choices;
            u32 numChoices;
        };
        struct {
            u32 scrMin;
            u32 scrMax;
            u32 scrStep;
        };
        struct SubMenu *nextMenu;
        void (*actionFn)(struct Option *, s32);
    };
};

struct SubMenu {
    struct SubMenu *prev; // this is set at runtime to avoid needless complication
    const u8 *label;
    struct Option *opts;
    s32 numOpts;
    s32 select;
    s32 scroll;
};

/* helper macros */

#define DEF_OPT_TOGGLE(lbl, bv) \
    { .type = OPT_TOGGLE, .label = lbl, .bval = bv }
#define DEF_OPT_SCROLL(lbl, uv, min, max, st) \
    { .type = OPT_SCROLL, .label = lbl, .uval = uv, .scrMin = min, .scrMax = max, .scrStep = st }
#define DEF_OPT_CHOICE(lbl, uv, ch) \
    { .type = OPT_CHOICE, .label = lbl, .uval = uv, .choices = ch, .numChoices = sizeof(ch) / sizeof(ch[0]) }
#define DEF_OPT_SUBMENU(lbl, nm) \
    { .type = OPT_SUBMENU, .label = lbl, .nextMenu = nm }
#define DEF_OPT_BIND(lbl, uv) \
    { .type = OPT_BIND, .label = lbl, .uval = uv }
#define DEF_OPT_BUTTON(lbl, act) \
    { .type = OPT_BUTTON, .label = lbl, .actionFn = act }
#define DEF_SUBMENU(lbl, opt) \
    { .label = lbl, .opts = opt, .numOpts = sizeof(opt) / sizeof(opt[0]) }

/* button action functions */

static void optmenu_act_exit(UNUSED struct Option *self, s32 arg) {
    if (!arg) game_exit(); // only exit on A press and not directions
}

static void optvideo_reset_window(UNUSED struct Option *self, s32 arg) {
    if (!arg) {
        // Restrict reset to A press and not directions
        configWindow.reset = true;
        configWindow.settings_changed = true;
    }
}

static void optvideo_apply(UNUSED struct Option *self, s32 arg) {
    if (!arg) configWindow.settings_changed = true;
}


static void setCap_Wing(UNUSED struct Option *self, s32 arg) {
    if (!arg) Cheats.WingCap = true;
}
static void setCap_Metal(UNUSED struct Option *self, s32 arg) {
    if (!arg) Cheats.MetalCap = true;
}
static void setCap_Vanish(UNUSED struct Option *self, s32 arg) {
    if (!arg) Cheats.VanishCap = true;
}
static void setCap_Remove(UNUSED struct Option *self, s32 arg) {
    if (!arg) Cheats.RemoveCap = true;
}
static void setCap_Normal(UNUSED struct Option *self, s32 arg) {
    Cheats.WingCap = false;
    Cheats.MetalCap = false;
    Cheats.VanishCap = false;
    Cheats.RemoveCap = false;
    if (!arg) Cheats.NormalCap = true;
}


/* submenu option lists */

#ifdef BETTERCAMERA
static struct Option optsCamera[] = {
    DEF_OPT_TOGGLE( optsCameraStr[9], &configEnableCamera ),
    DEF_OPT_TOGGLE( optsCameraStr[6], &configCameraAnalog ),
    DEF_OPT_TOGGLE( optsCameraStr[7], &configCameraMouse ),
    DEF_OPT_TOGGLE( optsCameraStr[2], &configCameraInvertX ),
    DEF_OPT_TOGGLE( optsCameraStr[3], &configCameraInvertY ),
    DEF_OPT_SCROLL( optsCameraStr[0], &configCameraXSens, 1, 100, 1 ),
    DEF_OPT_SCROLL( optsCameraStr[1], &configCameraYSens, 1, 100, 1 ),
    DEF_OPT_SCROLL( optsCameraStr[4], &configCameraAggr, 0, 100, 1 ),
    DEF_OPT_SCROLL( optsCameraStr[5], &configCameraPan, 0, 100, 1 ),
    DEF_OPT_SCROLL( optsCameraStr[8], &configCameraDegrade, 0, 100, 1 ),
};
#endif

static struct Option optsControls[] = {
    DEF_OPT_BIND( bindStr[ 2], configKeyA ),
    DEF_OPT_BIND( bindStr[ 3], configKeyB ),
    DEF_OPT_BIND( bindStr[ 4], configKeyX),
    DEF_OPT_BIND( bindStr[ 5], configKeyY),
    DEF_OPT_BIND( bindStr[ 6], configKeyStart),
    DEF_OPT_BIND( bindStr[ 7], configKeyL),
    DEF_OPT_BIND( bindStr[ 8], configKeyR),
    DEF_OPT_BIND( bindStr[ 9], configKeyZ),
    DEF_OPT_BIND( bindStr[10], configKeyCUp),
    DEF_OPT_BIND( bindStr[11], configKeyCDown),
    DEF_OPT_BIND( bindStr[12], configKeyCLeft),
    DEF_OPT_BIND( bindStr[13], configKeyCRight),
    DEF_OPT_BIND( bindStr[14], configKeyDUp),
    DEF_OPT_BIND( bindStr[15], configKeyDDown),
    DEF_OPT_BIND( bindStr[16], configKeyDLeft),
    DEF_OPT_BIND( bindStr[17], configKeyDRight),
    DEF_OPT_BIND( bindStr[18], configKeyStickUp),
    DEF_OPT_BIND( bindStr[19], configKeyStickDown),
    DEF_OPT_BIND( bindStr[20], configKeyStickLeft),
    DEF_OPT_BIND( bindStr[21], configKeyStickRight),
    // max deadzone is 31000; this is less than the max range of ~32768, but this
    // way, the player can't accidentally lock themselves out of using the stick
    DEF_OPT_SCROLL( bindStr[22], &configStickDeadzone, 0, 100, 1 ),
    DEF_OPT_SCROLL( bindStr[23], &configRumbleStrength, 0, 100, 1),
    DEF_OPT_TOGGLE( bindStr[24], &configTight),

};

static struct Option optsVideo[] = {
    DEF_OPT_TOGGLE( optsVideoStr[0], &configWindow.fullscreen ),
    DEF_OPT_TOGGLE( optsVideoStr[5], &configWindow.vsync ),
    DEF_OPT_CHOICE( optsVideoStr[1], &configFiltering, filterChoices ),
    DEF_OPT_TOGGLE( optsVideoStr[7], &configHUD ),
    DEF_OPT_BUTTON( optsVideoStr[4], optvideo_reset_window ),
    DEF_OPT_BUTTON( optsVideoStr[9], optvideo_apply ),
};

static struct Option optsAudio[] = {
    DEF_OPT_SCROLL( optsAudioStr[0], &configMasterVolume, 0, MAX_VOLUME, 1 ),
    DEF_OPT_SCROLL( optsAudioStr[1], &configMusicVolume, 0, MAX_VOLUME, 1),
    DEF_OPT_SCROLL( optsAudioStr[2], &configSfxVolume, 0, MAX_VOLUME, 1),
    DEF_OPT_SCROLL( optsAudioStr[3], &configEnvVolume, 0, MAX_VOLUME, 1),
};

static struct Option optsCheats[] = {
    DEF_OPT_TOGGLE(optsCheatsStr[0], &Cheats.EnableCheats),
    DEF_OPT_TOGGLE(optsCheatsStr[1], &Cheats.MoonJump),
    DEF_OPT_TOGGLE(optsCheatsStr[2], &Cheats.GodMode),
    DEF_OPT_TOGGLE(optsCheatsStr[3], &Cheats.InfiniteLives),
    DEF_OPT_CHOICE(optsCheatsStr[4], &Cheats.SuperSpeed, SpeedChoices),
    DEF_OPT_TOGGLE(optsCheatsStr[5], &Cheats.Responsive),
    DEF_OPT_TOGGLE(optsCheatsStr[6], &Cheats.ExitAnywhere),
    DEF_OPT_TOGGLE(optsCheatsStr[7], &Cheats.HugeMario),
    DEF_OPT_TOGGLE(optsCheatsStr[8], &Cheats.TinyMario),
    DEF_OPT_TOGGLE(optsCheatsStr[9], &Cheats.NDB),
    DEF_OPT_TOGGLE(optsCheatsStr[10], &Cheats.Jump),
    DEF_OPT_TOGGLE(optsCheatsStr[11], &Cheats.SPD),
    DEF_OPT_TOGGLE(optsCheatsStr[12], &Cheats.TPF),
    DEF_OPT_CHOICE(optsCheatsStr[13], &Cheats.JB, SeqChoices),
    DEF_OPT_TOGGLE(optsCheatsStr[14], &Cheats.JBC),
    DEF_OPT_TOGGLE(optsCheatsStr[15], &Cheats.QuikEnd),
    DEF_OPT_CHOICE(optsCheatsStr[16], &Cheats.Hurt, HurtCheatChoices),
    DEF_OPT_TOGGLE(optsCheatsStr[17], &Cheats.Cann),
    DEF_OPT_TOGGLE(optsCheatsStr[18], &Cheats.AutoWK),
    DEF_OPT_TOGGLE(optsCheatsStr[19], &Cheats.GetShell),
    DEF_OPT_TOGGLE(optsCheatsStr[20], &Cheats.GetBob),
    DEF_OPT_CHOICE(optsCheatsStr[21], &Cheats.Spamba, SpamCheatChoices),
    DEF_OPT_TOGGLE(optsCheatsStr[22], &Cheats.Swim),
    DEF_OPT_BUTTON(optsCheatsStr[23], setCap_Wing),
    DEF_OPT_BUTTON(optsCheatsStr[24], setCap_Metal),
    DEF_OPT_BUTTON(optsCheatsStr[25], setCap_Vanish),
    DEF_OPT_BUTTON(optsCheatsStr[26], setCap_Remove),
    DEF_OPT_TOGGLE(optsCheatsStr[27], &Cheats.DCM),
    DEF_OPT_BUTTON(optsCheatsStr[28], setCap_Normal),
    DEF_OPT_CHOICE(optsCheatsStr[29], &Cheats.BLJAnywhere, bljCheatChoices),
    DEF_OPT_CHOICE(optsCheatsStr[30], &Cheats.PAC, PlayAsCheatChoices),

};

#ifndef SMO_OPTIONS_H
/* submenu definitions */

#ifdef BETTERCAMERA
static struct SubMenu menuCamera   = DEF_SUBMENU( menuStr[4], optsCamera );
#endif
static struct SubMenu menuCheats   = DEF_SUBMENU( menuStr[5], optsCheats);
static struct SubMenu menuControls = DEF_SUBMENU( menuStr[6], optsControls );
static struct SubMenu menuVideo    = DEF_SUBMENU( menuStr[7], optsVideo );
static struct SubMenu menuAudio    = DEF_SUBMENU( menuStr[8], optsAudio );


/* main options menu definition */

static struct Option optsMain[] = {
#ifdef BETTERCAMERA
    DEF_OPT_SUBMENU( menuStr[4], &menuCamera ),
#endif
    DEF_OPT_SUBMENU( menuStr[5], &menuCheats),
    DEF_OPT_SUBMENU( menuStr[6], &menuControls ),
    DEF_OPT_SUBMENU( menuStr[7], &menuVideo ),
    DEF_OPT_SUBMENU( menuStr[8], &menuAudio ),
    DEF_OPT_BUTTON ( menuStr[9], optmenu_act_exit ),
};

static struct SubMenu menuMain = DEF_SUBMENU( menuStr[3], optsMain );

#else
#include "data/smo/system/smo_options.inl"
#endif
/* implementation */

static s32 optmenu_option_timer = 0;
static u8 optmenu_hold_count = 0;

static struct SubMenu *currentMenu = &menuMain;

static inline s32 wrap_add(s32 a, const s32 b, const s32 min, const s32 max) {
    a += b;
    if (a < min) a = max - (min - a) + 1;
    else if (a > max) a = min + (a - max) - 1;
    return a;
}

static void uint_to_hex(u32 num, u8 *dst) {
    u8 places = 4;
    while (places--) {
        const u32 digit = num & 0xF;
        dst[places] = digit;
        num >>= 4;
    }
    dst[4] = DIALOG_CHAR_TERMINATOR;
}

//Displays a box.
static void optmenu_draw_box(s16 x1, s16 y1, s16 x2, s16 y2, u8 r, u8 g, u8 b) {
    gDPPipeSync(gDisplayListHead++);
    gDPSetRenderMode(gDisplayListHead++, G_RM_OPA_SURF, G_RM_OPA_SURF2);
    gDPSetCycleType(gDisplayListHead++, G_CYC_FILL);
    gDPSetFillColor(gDisplayListHead++, GPACK_RGBA5551(r, g, b, 255));
    gDPFillRectangle(gDisplayListHead++, x1, y1, x2 - 1, y2 - 1);
    gDPPipeSync(gDisplayListHead++);
    gDPSetCycleType(gDisplayListHead++, G_CYC_1CYCLE);
}

static void optmenu_draw_text(s16 x, s16 y, const u8 *str, u8 col) {
    const u8 textX = get_str_x_pos_from_center(x, (u8*)str, 10.0f);
    gDPSetEnvColor(gDisplayListHead++, 0, 0, 0, 255);
    print_generic_string(textX+1, y-1, str);
    if (col == 0) {
        gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 255);
    } else {
        gDPSetEnvColor(gDisplayListHead++, 255, 32, 32, 255);
    }
    print_generic_string(textX, y, str);
}

static void optmenu_draw_opt(const struct Option *opt, s16 x, s16 y, u8 sel) {
    u8 buf[32] = { 0 };

    if (opt->type == OPT_SUBMENU || opt->type == OPT_BUTTON)
        y -= 6;

    optmenu_draw_text(x, y, opt->label, sel);

    switch (opt->type) {
        case OPT_TOGGLE:
            optmenu_draw_text(x, y-13, toggleStr[(int)*opt->bval], sel);
            break;

        case OPT_CHOICE:
            optmenu_draw_text(x, y-13, opt->choices[*opt->uval], sel);
            break;

        case OPT_SCROLL:
            int_to_str(*opt->uval, buf);
            optmenu_draw_text(x, y-13, buf, sel);
            break;

        case OPT_BIND:
            x = 112;
            for (u8 i = 0; i < MAX_BINDS; ++i, x += 48) {
                const u8 white = (sel && (optmenu_bind_idx == i));
                // TODO: button names
                if (opt->uval[i] == VK_INVALID) {
                    if (optmenu_binding && white)
                        optmenu_draw_text(x, y-13, bindStr[1], 1);
                    else
                        optmenu_draw_text(x, y-13, bindStr[0], white);
                } else {
                    uint_to_hex(opt->uval[i], buf);
                    optmenu_draw_text(x, y-13, buf, white);
                }
            }
            break;

        default: break;
    };
}

static void optmenu_opt_change(struct Option *opt, s32 val) {
    switch (opt->type) {
        case OPT_TOGGLE:
            *opt->bval = !*opt->bval;
            break;

        case OPT_CHOICE:
            *opt->uval = wrap_add(*opt->uval, val, 0, opt->numChoices - 1);
            break;

        case OPT_SCROLL:
            *opt->uval = wrap_add(*opt->uval, opt->scrStep * val, opt->scrMin, opt->scrMax);
            break;

        case OPT_SUBMENU:
            opt->nextMenu->prev = currentMenu;
            currentMenu = opt->nextMenu;
            break;

        case OPT_BUTTON:
            if (opt->actionFn)
                opt->actionFn(opt, val);
            break;

        case OPT_BIND:
            if (val == 0xFF) {
                // clear the bind
                opt->uval[optmenu_bind_idx] = VK_INVALID;
            } else if (val == 0) {
                opt->uval[optmenu_bind_idx] = VK_INVALID;
                optmenu_binding = 1;
                controller_get_raw_key(); // clear the last key, which is probably A
            } else {
                optmenu_bind_idx = wrap_add(optmenu_bind_idx, val, 0, MAX_BINDS - 1);
            }
            break;

        default: break;
    }
}

static inline s16 get_hudstr_centered_x(const s16 sx, const u8 *str) {
    const u8 *chr = str;
    s16 len = 0;
    while (*chr != GLOBAR_CHAR_TERMINATOR) ++chr, ++len;
    return sx - len * 6; // stride is 12
}

//Options menu
void optmenu_draw(void) {
    s16 scroll;
    s16 scrollpos;

    const s16 labelX = get_hudstr_centered_x(160, currentMenu->label);
    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_begin);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 255);
    print_hud_lut_string(HUD_LUT_GLOBAL, labelX, 40, currentMenu->label);
    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_end);

    if (currentMenu->numOpts > 4) {
        optmenu_draw_box(272, 90, 280, 208, 0x80, 0x80, 0x80);
        scrollpos = 54 * ((f32)currentMenu->scroll / (currentMenu->numOpts-4));
        optmenu_draw_box(272, 90+scrollpos, 280, 154+scrollpos, 0xFF, 0xFF, 0xFF);
    }

    gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);
    gDPSetScissor(gDisplayListHead++, G_SC_NON_INTERLACE, 0, 80, SCREEN_WIDTH, SCREEN_HEIGHT);
    for (u8 i = 0; i < currentMenu->numOpts; i++) {
        scroll = 140 - 32 * i + currentMenu->scroll * 32;
        // FIXME: just start from the first visible option bruh
        if (scroll <= 140 && scroll > 32)
            optmenu_draw_opt(&currentMenu->opts[i], 160, scroll, (currentMenu->select == i));
    }

    gDPSetScissor(gDisplayListHead++, G_SC_NON_INTERLACE, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    gSPDisplayList(gDisplayListHead++, dl_ia_text_end);
    gDPSetEnvColor(gDisplayListHead++, 255, 255, 255, 255);
    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_begin);
    print_hud_lut_string(HUD_LUT_GLOBAL, 80, 90 + (32 * (currentMenu->select - currentMenu->scroll)), menuStr[0]);
    print_hud_lut_string(HUD_LUT_GLOBAL, 224, 90 + (32 * (currentMenu->select - currentMenu->scroll)), menuStr[0]);
    gSPDisplayList(gDisplayListHead++, dl_rgba16_text_end);
}

//This has been separated for interesting reasons. Don't question it.
void optmenu_draw_prompt(void) {
    gSPDisplayList(gDisplayListHead++, dl_ia_text_begin);
    optmenu_draw_text(264, 212, menuStr[1 + optmenu_open], 0);
    gSPDisplayList(gDisplayListHead++, dl_ia_text_end);
}

void optmenu_toggle(void) {
    if (optmenu_open == 0) {
        #ifndef nosound
        play_sound(SOUND_MENU_CHANGE_SELECT, gDefaultSoundArgs);
        #endif

        // HACK: hide the last option in main if cheats are disabled
        menuMain.numOpts = sizeof(optsMain) / sizeof(optsMain[0]);


        currentMenu = &menuMain;
        optmenu_open = 1;
        

    } else {
        #ifndef nosound
        play_sound(SOUND_MENU_MARIO_CASTLE_WARP2, gDefaultSoundArgs);
        #endif
        optmenu_open = 0;
        #ifdef BETTERCAMERA
        newcam_init_settings(); // load bettercam settings from config vars
        #endif
        controller_reconfigure(); // rebind using new config values
        configfile_save(configfile_name());
    }
}

void optmenu_check_buttons(void) {
    if (optmenu_binding) {
        u32 key = controller_get_raw_key();
        if (key != VK_INVALID) {
            #ifndef nosound
            play_sound(SOUND_MENU_CHANGE_SELECT, gDefaultSoundArgs);
            #endif
            currentMenu->opts[currentMenu->select].uval[optmenu_bind_idx] = key;
            optmenu_binding = 0;
            optmenu_option_timer = 12;
        }
        return;
    }

    if (gPlayer1Controller->buttonPressed & R_TRIG)
        optmenu_toggle();
    
    /* Enables cheats if the user press the L trigger 3 times while in the options menu. Also plays a sound. */
    
    if ((gPlayer1Controller->buttonPressed & R_TRIG) && !Cheats.EnableCheats) {
        Cheats.EnableCheats = true;
        play_sound(SOUND_MENU_STAR_SOUND, gDefaultSoundArgs);
    }
    
    if (!optmenu_open) return;

    u8 allowInput = 0;

    optmenu_option_timer--;
    if (optmenu_option_timer <= 0) {
        if (optmenu_hold_count == 0) {
            optmenu_hold_count++;
            optmenu_option_timer = 10;
        } else {
            optmenu_option_timer = 3;
        }
        allowInput = 1;
    }

    if (ABS(gPlayer1Controller->stickY) > 60) {
        if (allowInput) {
            #ifndef nosound
            play_sound(SOUND_MENU_CHANGE_SELECT, gDefaultSoundArgs);
            #endif

            if (gPlayer1Controller->stickY >= 60) {
                currentMenu->select--;
                if (currentMenu->select < 0)
                    currentMenu->select = currentMenu->numOpts-1;
            } else {
                currentMenu->select++;
                if (currentMenu->select >= currentMenu->numOpts)
                    currentMenu->select = 0;
            }

            if (currentMenu->select < currentMenu->scroll)
                currentMenu->scroll = currentMenu->select;
            else if (currentMenu->select > currentMenu->scroll + 3)
                currentMenu->scroll = currentMenu->select - 3;
        }
    } else if (ABS(gPlayer1Controller->stickX) > 60) {
        if (allowInput) {
            #ifndef nosound
            play_sound(SOUND_MENU_CHANGE_SELECT, gDefaultSoundArgs);
            #endif
            if (gPlayer1Controller->stickX >= 60)
                optmenu_opt_change(&currentMenu->opts[currentMenu->select], 1);
            else
                optmenu_opt_change(&currentMenu->opts[currentMenu->select], -1);
        }
    } else if (gPlayer1Controller->buttonPressed & A_BUTTON) {
        if (allowInput) {
            #ifndef nosound
            play_sound(SOUND_MENU_CHANGE_SELECT, gDefaultSoundArgs);
            #endif
            optmenu_opt_change(&currentMenu->opts[currentMenu->select], 0);
        }
     } else if (gPlayer1Controller->buttonPressed & D_CBUTTONS) {
        if (allowInput) {
            #ifndef nosound
            play_sound(SOUND_MENU_CHANGE_SELECT, gDefaultSoundArgs);
            #endif
	    currentMenu->select++;
            if (currentMenu->select >= currentMenu->numOpts)
                currentMenu->select = 0;
            if (currentMenu->select < currentMenu->scroll)
                currentMenu->scroll = currentMenu->select;
            else if (currentMenu->select > currentMenu->scroll + 3)
                currentMenu->scroll = currentMenu->select - 3;
        }
   } else if (gPlayer1Controller->buttonPressed & U_CBUTTONS) {
        if (allowInput) {
            #ifndef nosound
            play_sound(SOUND_MENU_CHANGE_SELECT, gDefaultSoundArgs);
            #endif
	    currentMenu->select--;
            if (currentMenu->select < 0)
                currentMenu->select = currentMenu->numOpts-1;
            if (currentMenu->select < currentMenu->scroll)
                currentMenu->scroll = currentMenu->select;
            else if (currentMenu->select > currentMenu->scroll + 3)
                currentMenu->scroll = currentMenu->select - 3;
        }
    } else if (gPlayer1Controller->buttonPressed & R_CBUTTONS) {
        if (allowInput) {
            #ifndef nosound
            play_sound(SOUND_MENU_CHANGE_SELECT, gDefaultSoundArgs);
            #endif
	    optmenu_opt_change(&currentMenu->opts[currentMenu->select], 1);
        }
    } else if (gPlayer1Controller->buttonPressed & L_CBUTTONS) {
        if (allowInput) {
            #ifndef nosound
            play_sound(SOUND_MENU_CHANGE_SELECT, gDefaultSoundArgs);
            #endif
	    optmenu_opt_change(&currentMenu->opts[currentMenu->select], -1);
	}
    } else if (gPlayer1Controller->buttonPressed & B_BUTTON) {
        if (allowInput) {
            if (currentMenu->prev) {
                #ifndef nosound
                play_sound(SOUND_MENU_CHANGE_SELECT, gDefaultSoundArgs);
                #endif
                currentMenu = currentMenu->prev;
            } else {
                // can't go back, exit the menu altogether
                optmenu_toggle();
            }
        }
    } else if (gPlayer1Controller->buttonPressed & Z_TRIG) {
        // HACK: clear binds with Z
        if (allowInput && currentMenu->opts[currentMenu->select].type == OPT_BIND)
            optmenu_opt_change(&currentMenu->opts[currentMenu->select], 0xFF);
    } else if (gPlayer1Controller->buttonPressed & START_BUTTON) {
        if (allowInput) optmenu_toggle();
    } else {
        optmenu_hold_count = 0;
        optmenu_option_timer = 0;
    }
}

#endif // EXT_OPTIONS_MENU
