#include QMK_KEYBOARD_H
#include "keymap_german.h"
#include "debug.h"
#include "action_layer.h"
#include "version.h"
#include "layers.h"

// Timer to detect tap/hold on NEO_RMOD3 key
static uint16_t neo3_timer;
// State bitmap to track which key(s) enabled NEO_3 layer
static uint8_t neo3_state = 0;
// State bitmap to track key combo for CAPSLOCK
static uint8_t capslock_state = 0;

// bitmasks for modifier keys
#define MODS_NONE   0
#define MODS_SHIFT  (MOD_BIT(KC_LSHIFT)|MOD_BIT(KC_RSHIFT))
#define MODS_CTRL   (MOD_BIT(KC_LCTL)|MOD_BIT(KC_RCTRL))
#define MODS_ALT    (MOD_BIT(KC_LALT)|MOD_BIT(KC_RALT))
#define MODS_GUI    (MOD_BIT(KC_LGUI)|MOD_BIT(KC_RGUI))

// Used to trigger macros / sequences of keypresses
enum custom_keycodes {
  PLACEHOLDER = SAFE_RANGE,     // can always be here
  NEO2_LMOD3,
  NEO2_RMOD3,
  YELDIR_AC,
  YELDIR_CTLTAB,
  YELDIR_CTLSTAB,
  NEO2_1,
  NEO2_2,
  NEO2_3,
  NEO2_4,
  NEO2_5,
  NEO2_6,
  NEO2_7,
  NEO2_8,
  NEO2_9,
  NEO2_0,
  NEO2_MINUS,
  NEO2_COMMA,
  NEO2_DOT,
  NEO2_SHARP_S
};

#define NEO2_LMOD4                   MO(NEO_4)
#define NEO2_RMOD4                   NEO2_LMOD4

// Use _______ to indicate a key that is transparent / falling through to a lower level
#define _______ KC_TRNS

// NEO_3 special characters
#define NEO2_L3_CAPITAL_SS           RSA(DE_S)                   // ẞ
#define NEO2_L3_CAPITAL_UE           S(DE_UDIA)                  // Ü
#define NEO2_L3_CAPITAL_OE           S(DE_ODIA)                  // Ö
#define NEO2_L3_CAPITAL_AE           S(DE_ADIA)                  // Ä
#define NEO2_L3_SUPERSCRIPT_1        RALT(DE_1)                  // ¹
#define NEO2_L3_SUPERSCRIPT_2        DE_SUP2                     // ²
#define NEO2_L3_SUPERSCRIPT_3        DE_SUP3                     // ³
#define NEO2_L3_RSAQUO               RSA(DE_Y)                   // ›
#define NEO2_L3_LSAQUO               RSA(DE_X)                   // ‹
#define NEO2_L3_RAQUO                RALT(DE_Y)                  // »
#define NEO2_L3_LAQUO                RALT(DE_X)                  // «
#define NEO2_L3_CENT                 RALT(DE_C)                  // ¢
#define NEO2_L3_YEN                  RSA(DE_Z)                   // ¥
#define NEO2_L3_SBQUO                RSA(DE_V)                   // ‚
#define NEO2_L3_LEFT_SINGLE_QUOTE    RSA(DE_B)                   // ‘
#define NEO2_L3_RIGHT_SINGLE_QUOTE   RSA(DE_N)                   // ’
#define NEO2_L3_LOW9_DBQUOTE         RALT(DE_V)                  // „
#define NEO2_L3_LEFT_DBQUOTE         RALT(DE_B)                  // “
#define NEO2_L3_RIGHT_DBQUOTE        RALT(DE_N)                  // ”
#define NEO2_L3_ELLIPSIS             RALT(DE_DOT)                // …
#define NEO2_L3_UNDERSCORE           DE_UNDS                     // _
#define NEO2_L3_LBRACKET             DE_LBRC                     // [
#define NEO2_L3_RBRACKET             DE_RBRC                     // ]
#define NEO2_L3_CIRCUMFLEX           DE_CIRC                     // ^
#define NEO2_L3_EXCLAMATION          DE_EXLM                     // !
#define NEO2_L3_LESSTHAN             DE_LABK                     // <
#define NEO2_L3_GREATERTHAN          DE_RABK                     // >
#define NEO2_L3_EQUAL                DE_EQL                      // =
#define NEO2_L3_AMPERSAND            DE_AMPR                     // &
#define NEO2_L3_SMALL_LONG_S         KC_NO                       // ſ
#define NEO2_L3_BSLASH               DE_BSLS                     // (backslash)
#define NEO2_L3_SLASH                DE_SLSH                     // /
#define NEO2_L3_CLBRACKET            DE_LCBR                     // {
#define NEO2_L3_CRBRACKET            DE_RCBR                     // }
#define NEO2_L3_ASTERISK             DE_ASTR                     // *
#define NEO2_L3_QUESTIONMARK         DE_QUES                     // ?
#define NEO2_L3_LPARENTHESES         DE_LPRN                     // (
#define NEO2_L3_RPARENTHESES         DE_RPRN                     // )
#define NEO2_L3_HYPHEN_MINUS         DE_MINS                     // -
#define NEO2_L3_COLON                DE_COLN                     // :
#define NEO2_L3_AT                   DE_AT                       // @
#define NEO2_L3_HASH                 DE_HASH                     // #
#define NEO2_L3_PIPE                 DE_PIPE                     // |
#define NEO2_L3_TILDE                DE_TILD                     // ~
#define NEO2_L3_BACKTICK             DE_GRV                      // `
#define NEO2_L3_PLUS                 DE_PLUS                     // +
#define NEO2_L3_PERCENT              DE_PERC                     // %
#define NEO2_L3_DOUBLE_QUOTE         DE_DQUO                     // "
#define NEO2_L3_SINGLE_QUOTE         DE_QUOT                     // '
#define NEO2_L3_SEMICOLON            DE_SCLN                     // ;

// NEO_4 special characters
#define NEO2_L3_FEMININE_ORDINAL     RSA(DE_F)                   // ª
#define NEO2_L3_MASCULINE_ORDINAL    RSA(DE_M)                   // º
#define NEO2_L3_NUMERO_SIGN          KC_NO                       // №
#define NEO2_L3_MIDDLE_DOT           RALT(DE_COMM)               // ·
#define NEO2_L3_BRITISH_POUND        RSA(DE_3)                   // £
#define NEO2_L3_CURRENCY_SIGN        RSA(DE_4)                   // ¤
#define NEO2_L3_INV_EXCLAMATION      RSA(DE_1)                   // ¡
#define NEO2_L3_INV_QUESTIONMARK     RSA(DE_SS)                  // ¿
#define NEO2_L3_DOLLAR               DE_DLR                      // $
#define NEO2_L3_EN_DASH              RALT(DE_MINS)               // –
#define NEO2_L3_EM_DASH              RSA(DE_MINS)                // —

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  /* NEO_1: Basic layer
   *
   * ,--------------------------------------------------.           ,--------------------------------------------------.
   * |  ----  |  1/° |  2/§ |  3/  |  4/» |  5/« | ---- |           |  DE  |  6/$ |  7/€ |  8/„ |  9/“ |  0/” |  -/—   |
   * |--------+------+------+------+------+-------------|           |------+------+------+------+------+------+--------|
   * |  TAB   |   X  |   V  |   L  |   C  |   W  | ALTS |           | ALT  |   K  |   H  |   G  |   F  |   Q  |   ß    |
   * |--------+------+------+------+------+------| TAB  |           | TAB  |------+------+------+------+------+--------|
   * |  NEO3  |   U  |   I  |   A  |   E  |   O  |------|           |------|   S  |   N  |   R  |   T  |   D  |   Y    |
   * |--------+------+------+------+------+------| ---- |           | ---- |------+------+------+------+------+--------|
   * | LSHIFT |   Ü  |   Ö  |   Ä  |   P  |   Z  |      |           |      |   B  |   M  |  ,/– |  ./• |   J  | RSHIFT |
   * `--------+------+------+------+------+-------------'           `-------------+------+------+------+------+--------'
   *   | ---- | ---- | ---- | ---- | NEO4 |                                       | NEO4 | ---- | ---- | ---- | ---- |
   *   `----------------------------------'                                       `----------------------------------'
   *                                        ,-------------.       ,-------------.
   *                                        | ---- | ---- |       | ---- | FKEY |
   *                                 ,------|------|------|       |------+------+------.
   *                                 |      |      | AC   |       | AC   |      |      |
   *                                 | LGUI | LALT |------|       |------| ALTG |Space |
   *                                 |      |      | LCTRL|       | RCTRL|      |      |
   *                                 `--------------------'       `--------------------'
   */
  [NEO_1] = LAYOUT_ergodox(
    // left hand side - main
    KC_NO /* NOOP */, NEO2_1,                   NEO2_2,                   NEO2_3,                   NEO2_4,           NEO2_5,           KC_NO,
    KC_TAB,           DE_X,                     DE_V,                     DE_L,                     DE_C,             DE_W,             YELDIR_CTLSTAB,
    NEO2_LMOD3,       DE_U,                     DE_I,                     DE_A,                     DE_E,             DE_O,             /* --- */
    KC_LSHIFT,        DE_UDIA,                  DE_ODIA,                  DE_ADIA,                  DE_P,             DE_Z,             KC_NO,
    KC_NO /* NOOP */, KC_NO /* NOOP */,         KC_NO,                    KC_NO,                    NEO2_LMOD4,       /* --- */         /* --- */

    // left hand side - thumb cluster
    /* --- */         KC_NO,            KC_NO,
    /* KC_BSPACE */   /* KC_DELETE */   YELDIR_AC,
    KC_LGUI,          KC_LALT,          KC_LCTRL,

    // right hand side - main
    TO(DE_NORMAL),    NEO2_6,           NEO2_7,           NEO2_8,           NEO2_9,           NEO2_0,           NEO2_MINUS,
    YELDIR_CTLTAB,    DE_K,             DE_H,             DE_G,             DE_F,             DE_Q,             NEO2_SHARP_S,
    /* --- */         DE_S,             DE_N,             DE_R,             DE_T,             DE_D,             DE_Y,
    KC_NO,            DE_B,             DE_M,             NEO2_COMMA,       NEO2_DOT,         DE_J,             KC_RSHIFT,
    /* --- */         /* --- */         NEO2_RMOD4,       KC_NO,            KC_NO,            KC_NO,            KC_NO,

    // right hand side - thumb cluster
    KC_NO,            MO(FKEYS),        /* --- */
    YELDIR_AC,        /* --- */         /* --- */
    KC_RCTRL,         KC_RALT,          KC_SPACE
  ),

  /* NEO_3: Symbol layer
   *
   * ,--------------------------------------------------.           ,--------------------------------------------------.
   * |  ----  | ---- | ---- | ---- |   ›  |   ‹  |      |           |      |   ¢ 	|   ¥  |   ‚  |   ‘  |   ’  |  ----  |
   * |--------+------+------+------+------+-------------|           |------+------+------+------+------+------+--------|
   * |  ----  |   …  |   _  |   [  |   ]  |   ^  |      |           |      |   !  |   <  |   >  |   =  |   &  |  ----  |
   * |--------+------+------+------+------+------|      |           |      |------+------+------+------+------+--------|
   * |        |   \  |   /  |   {  |   }  |   *  |------|           |------|   ?  |   (  |   )  |   -  |   :  |   @    |
   * |--------+------+------+------+------+------|      |           |      |------+------+------+------+------+--------|
   * |        |   #  |   $  |   |  |   ~  |   `  |      |           |      |   +  |   %  |   "  |   '  |   ;  |        |
   * `--------+------+------+------+------+-------------'           `-------------+------+------+------+------+--------'
   *   |      |      |      |      |      |                                       |      |      |      |      |      |
   *   `----------------------------------'                                       `----------------------------------'
   *                                        ,-------------.       ,-------------.
   *                                        |      |      |       |      |      |
   *                                 ,------|------|------|       |------+------+------.
   *                                 |      |      |      |       |      |      |      |
   *                                 |      |      |------|       |------|      |      |
   *                                 |      |      |      |       |      |      |      |
   *                                 `--------------------'       `--------------------'
   */
  [NEO_3] = LAYOUT_ergodox(
    // left hand side - main
    KC_NO /* NOOP */,   KC_NO /* NOOP */,      KC_NO /* NOOP */,      KC_NO /* NOOP */,      NEO2_L3_RSAQUO,            NEO2_L3_LSAQUO,                _______,
    KC_NO /* NOOP */,   NEO2_L3_ELLIPSIS,      NEO2_L3_UNDERSCORE,    NEO2_L3_LBRACKET,      NEO2_L3_RBRACKET,          NEO2_L3_CIRCUMFLEX,            _______,
    _______,            NEO2_L3_BSLASH,        NEO2_L3_SLASH,         NEO2_L3_CLBRACKET,     NEO2_L3_CRBRACKET,         NEO2_L3_ASTERISK,              /* --- */
    _______,            NEO2_L3_HASH,          NEO2_L3_DOLLAR,        NEO2_L3_PIPE,          NEO2_L3_TILDE,             NEO2_L3_BACKTICK,              _______,
    _______,            _______,               _______,               _______,               _______,                   /* --- */                      /* --- */

    // left hand side - thumb cluster
    /* --- */           _______,              _______,
    /* --- */           /* --- */             _______,
    _______,            _______,              _______,

    // right hand side - main
    _______,            NEO2_L3_CENT,          NEO2_L3_YEN,           NEO2_L3_SBQUO,         NEO2_L3_LEFT_SINGLE_QUOTE,  NEO2_L3_RIGHT_SINGLE_QUOTE,   KC_NO,
    _______,            NEO2_L3_EXCLAMATION,   NEO2_L3_LESSTHAN,      NEO2_L3_GREATERTHAN,   NEO2_L3_EQUAL,              NEO2_L3_AMPERSAND,            NEO2_L3_SMALL_LONG_S,
    /* --- */           NEO2_L3_QUESTIONMARK,  NEO2_L3_LPARENTHESES,  NEO2_L3_RPARENTHESES,  NEO2_L3_HYPHEN_MINUS,       NEO2_L3_COLON,                DE_AT,
    _______,            NEO2_L3_PLUS,          NEO2_L3_PERCENT,       NEO2_L3_DOUBLE_QUOTE,  NEO2_L3_SINGLE_QUOTE,       NEO2_L3_SEMICOLON,            _______,
    /* --- */           /* --- */              _______,               _______,               _______,                    _______,                      _______,

    // right hand side - thumb cluster
    _______,            _______,              /* --- */
    _______,            /* --- */             /* --- */
    _______,            _______,              _______
  ),

  /* NEO_4: Cursor & Numpad
   *
   * ,--------------------------------------------------.           ,--------------------------------------------------.
   * |  ----  |   ª  |   º  | ---- |   ·  |   £  |      |           |      | ---- | Tab  |   /  |   *  |   -  |  ----  |
   * |--------+------+------+------+------+-------------|           |------+------+------+------+------+------+--------|
   * |  ----  | PgUp |   ⌫  |  Up  |   ⌦  | PgDn |      |           |      |   ¡  |   7  |   8  |   9  |   +  |   –    |
   * |--------+------+------+------+------+------|      |           |      |------+------+------+------+------+--------|
   * |        | Home | Left | Down | Right| End  |------|           |------|   ¿  |   4  |   5  |   6  |   ,  |   .    |
   * |--------+------+------+------+------+------|      |           |      |------+------+------+------+------+--------|
   * |        | Esc  | Tab  | Ins  |Return| ---- |      |           |      |   :  |   1  |   2  |   3  |   ;  |        |
   * `--------+------+------+------+------+-------------'           `-------------+------+------+------+------+--------'
   *   |      |      |      |      |      |                                       |      |   0  |      |      |      |
   *   `----------------------------------'                                       `----------------------------------'
   *                                        ,-------------.       ,-------------.
   *                                        |      |      |       |      |      |
   *                                 ,------|------|------|       |------+------+------.
   *                                 |      |      |      |       |      |      |      |
   *                                 |      |      |------|       |------|      |      |
   *                                 |      |      |      |       |      |      |      |
   *                                 `--------------------'       `--------------------'
   */
  [NEO_4] = LAYOUT_ergodox(
    // left hand side - main
    KC_NO /* NOOP */,   NEO2_L3_FEMININE_ORDINAL, NEO2_L3_MASCULINE_ORDINAL,KC_NO /* NOOP */,     NEO2_L3_MIDDLE_DOT, NEO2_L3_BRITISH_POUND, _______,
    _______,            KC_PGUP,                  KC_BSPACE,                KC_UP,                KC_DELETE,          KC_PGDOWN,            _______,
    _______,            KC_HOME,                  KC_LEFT,                  KC_DOWN,              KC_RIGHT,           KC_END,               /* --- */
    _______,            KC_ESCAPE,                KC_TAB,                   KC_INSERT,            KC_ENTER,           KC_NO /* NOOP */,     _______,
    _______,            _______,                  _______,                  _______,              _______,            /* --- */             /* --- */

    // left hand side - thumb cluster
    /* --- */           _______,                  _______,
    /* --- */           /* --- */                 _______,
    _______,            _______,                  _______,

    // right hand side - main
    _______,            NEO2_L3_CURRENCY_SIGN,     KC_TAB,                   KC_SLASH,          DE_ASTR,         DE_MINS,              KC_NO /* NOOP */,
    _______,            NEO2_L3_INV_EXCLAMATION,   KC_7,                     KC_8,              KC_9,            DE_PLUS,              NEO2_L3_EM_DASH,
    /* --- */           NEO2_L3_INV_QUESTIONMARK,  KC_4,                     KC_5,              KC_6,            DE_COMM,              KC_DOT,
    _______,            NEO2_L3_COLON,             KC_1,                     KC_2,              KC_3,            NEO2_L3_SEMICOLON,    _______,
    /* --- */           /* --- */                 _______,                   KC_0,              _______,         _______,              _______,

    // right hand side - thumb cluster
    _______,            _______,                  /* --- */
    _______,            /* --- */                 /* --- */
    _______,            _______,                  _______
  ),

  /* NEO_5: Greek
   *
   * ,--------------------------------------------------.           ,--------------------------------------------------.
   * |  ----  | ---- | ---- | ---- | ---- | ---- |      |           |      | ---- | ---- | ---- | ---- | ---- |  ----  |
   * |--------+------+------+------+------+-------------|           |------+------+------+------+------+------+--------|
   * |  ----  | ---- | ---- | ---- | ---- | ---- |      |           |      | ---- | ---- | ---- | ---- | ---- |  ----  |
   * |--------+------+------+------+------+------|      |           |      |------+------+------+------+------+--------|
   * |        | ---- |  ----| ---- | ---- | ---- |------|           |------| ---- | ---- | ---- | ---- | ---- |  ----  |
   * |--------+------+------+------+------+------|      |           |      |------+------+------+------+------+--------|
   * |        | ---- |  ----| ---- | ---- | ---- |      |           |      | ---- | ---- | ---- | ---- | ---- |        |
   * `--------+------+------+------+------+-------------'           `-------------+------+------+------+------+--------'
   *   |      |      |      |      |      |                                       |      |      |      |      |      |
   *   `----------------------------------'                                       `----------------------------------'
   *                                        ,-------------.       ,-------------.
   *                                        |      |      |       |      |      |
   *                                 ,------|------|------|       |------+------+------.
   *                                 |      |      |      |       |      |      |      |
   *                                 |      |      |------|       |------|      |      |
   *                                 |      |      |      |       |      |      |      |
   *                                 `--------------------'       `--------------------'
   */
  [NEO_5] = LAYOUT_ergodox(
    // left hand side - main
    KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,     KC_NO /* NOOP */,   KC_NO /* NOOP */,   _______,
    KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,     KC_NO /* NOOP */,   KC_NO /* NOOP */,   _______,
    _______,            KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,     KC_NO /* NOOP */,   KC_NO /* NOOP */,   /* --- */
    _______,            KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,     KC_NO /* NOOP */,   KC_NO /* NOOP */,   _______,
    _______,            _______,            _______,            _______,              _______,            /* --- */           /* --- */

    // left hand side - thumb cluster
    /* --- */           _______,            _______,
    /* --- */           /* --- */           _______,
    _______,            _______,            _______,

    // right hand side - main
    _______,            KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,     KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,
    _______,            KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,     KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,
    /* --- */           KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,     KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,
    _______,            KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,     KC_NO /* NOOP */,   KC_NO /* NOOP */,   _______,
    /* --- */           /* --- */           _______,            _______,              _______,            _______,            _______,

    // right hand side - thumb cluster
    _______,            _______,            /* --- */
    _______,            /* --- */           /* --- */
    _______,            _______,            _______
  ),

  /* NEO_6: Math symbols
   *
   * ,--------------------------------------------------.           ,--------------------------------------------------.
   * |  ----  | ---- | ---- | ---- | ---- | ---- |      |           |      | ---- | ---- | ---- | ---- | ---- |  ----  |
   * |--------+------+------+------+------+-------------|           |------+------+------+------+------+------+--------|
   * |  ----  | ---- | ---- | ---- | ---- | ---- |      |           |      | ---- | ---- | ---- | ---- | ---- |  ----  |
   * |--------+------+------+------+------+------|      |           |      |------+------+------+------+------+--------|
   * |        | ---- |  ----| ---- | ---- | ---- |------|           |------| ---- | ---- | ---- | ---- | ---- |  ----  |
   * |--------+------+------+------+------+------|      |           |      |------+------+------+------+------+--------|
   * |        | ---- |  ----| ---- | ---- | ---- |      |           |      | ---- | ---- | ---- | ---- | ---- |        |
   * `--------+------+------+------+------+-------------'           `-------------+------+------+------+------+--------'
   *   |      |      |      |      |      |                                       |      |      |      |      |      |
   *   `----------------------------------'                                       `----------------------------------'
   *                                        ,-------------.       ,-------------.
   *                                        |      |      |       |      |      |
   *                                 ,------|------|------|       |------+------+------.
   *                                 |      |      |      |       |      |      |      |
   *                                 |      |      |------|       |------|      |      |
   *                                 |      |      |      |       |      |      |      |
   *                                 `--------------------'       `--------------------'
   */
  [NEO_6] = LAYOUT_ergodox(
    // left hand side - main
    KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,     KC_NO /* NOOP */,   KC_NO /* NOOP */,   _______,
    KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,     KC_NO /* NOOP */,   KC_NO /* NOOP */,   _______,
    _______,            KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,     KC_NO /* NOOP */,   KC_NO /* NOOP */,   /* --- */
    _______,            KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,     KC_NO /* NOOP */,   KC_NO /* NOOP */,   _______,
    _______,            _______,            _______,            _______,              _______,            /* --- */           /* --- */

    // left hand side - thumb cluster
    /* --- */           _______,            _______,
    /* --- */           /* --- */           _______,
    _______,            _______,            _______,

    // right hand side - main
    _______,            KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,     KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,
    _______,            KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,     KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,
    /* --- */           KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,     KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,
    _______,            KC_NO /* NOOP */,   KC_NO /* NOOP */,   KC_NO /* NOOP */,     KC_NO /* NOOP */,   KC_NO /* NOOP */,   _______,
    /* --- */           /* --- */           _______,            _______,              _______,            _______,            _______,

    // right hand side - thumb cluster
    _______,            _______,            /* --- */
    _______,            /* --- */           /* --- */
    _______,            _______,            _______
  ),

  /* DE_NORMAL: DE QWERTZ
   *
   * ,--------------------------------------------------.           ,--------------------------------------------------.
   * |   =    |   1  |   2  |   3  |   4  |   5  | ESC  |           | NEO_1|   6  |   7  |   8  |   9  |   0  |    ß   |
   * |--------+------+------+------+------+-------------|           |------+------+------+------+------+------+--------|
   * |   \    |   Q  |   W  |   E  |   R  |   T  | ---- |           | ---- |   Z  |   U  |   I  |   O  |   P  |    Ü   |
   * |--------+------+------+------+------+------|      |           |      |------+------+------+------+------+--------|
   * |  TAB   |   A  |   S  |   D  |   F  |   G  |------|           |------|   H  |   J  |   K  |   L  |   Ö  |    Ä   |
   * |--------+------+------+------+------+------| ---- |           | ---- |------+------+------+------+------+--------|
   * | LSHIFT |   Y  |   X  |   C  |   V  |   B  |      |           |      |   N  |   M  |   ,  |   .  |   -  | RSHIFT |
   * `--------+------+------+------+------+-------------'           `-------------+------+------+------+------+--------'
   *   | LGUI |   `  | ---- | ---- | FKEYS|                                       | Left | Down |  Up  | Right| RGUI |
   *   `----------------------------------'                                       `----------------------------------'
   *                                        ,-------------.       ,-------------.
   *                                        | LCTRL| LALT |       | RALT | RCTRL|
   *                                 ,------|------|------|       |------+------+------.
   *                                 |      |      | HOME |       | PGUP |      |      |
   *                                 | BKSP | DEL  |------|       |------| ENTR | SPCE |
   *                                 |      |      | END  |       | PGDN |      |      |
   *                                 `--------------------'       `--------------------'
   */
  [DE_NORMAL] = LAYOUT_ergodox(
    // left hand side - main
    KC_EQUAL,         DE_1,         DE_2,       DE_3,       DE_4,       DE_5,       KC_ESCAPE,
    KC_BSLASH,        DE_Q,         DE_W,       DE_E,       DE_R,       DE_T,       KC_NO /* NOOP */,
    KC_TAB,           DE_A,         DE_S,       DE_D,       DE_F,       DE_G,       /* --- */
    KC_LSHIFT,        DE_Y,         DE_X,       DE_C,       DE_V,       DE_B,       KC_NO /* NOOP */,
    KC_LGUI,          KC_NO,        KC_NO,      KC_NO,      MO(FKEYS),  /* --- */   /* --- */

    // left hand side - thumb cluster
    /* --- */         KC_LCTRL,     KC_LALT,
    /* --- */         /* --- */     KC_HOME,
    KC_BSPACE,        KC_DELETE,    KC_END,

    // right hand side - main
    TO(NEO_1),        DE_6,         DE_7,       DE_8,       DE_9,       DE_0,       DE_SS,
    KC_NO,            DE_Z,         DE_U,       DE_I,       DE_O,       DE_P,       DE_UDIA,
    /* --- */         DE_H,         DE_J,       DE_K,       DE_L,       DE_ODIA,    DE_ADIA,
    KC_NO /* NOOP */, DE_N,         DE_M,       DE_COMM,    DE_DOT,     DE_MINS,    KC_RSHIFT,
    /* --- */         /* --- */     KC_LEFT,    KC_DOWN,    KC_UP,      KC_RIGHT,   KC_RGUI,

    // right hand side - thumb cluster
    KC_RALT,          KC_RCTRL,     /* --- */
    KC_PGUP,          /* --- */     /* --- */
    KC_PGDOWN,        KC_ENTER,     KC_SPACE
  ),

  /* FKEYS: Function keys
   *
   * ,--------------------------------------------------.           ,--------------------------------------------------.
   * |  Prev  |  F1  |  F2  |  F3  |  F4  |  F5  |  F11 |           |  F12 |  F6  |  F7  |  F8  |  F9  |  F10 |  VolUp |
   * |--------+------+------+------+------+-------------|           |------+------+------+------+------+------+--------|
   * |  Play  |      |      |      |      |      |      |           |      |      |      |      |      |      |  VolDn |
   * |--------+------+------+------+------+------|      |           |      |------+------+------+------+------+--------|
   * |  Next  |      |      |      |      |      |------|           |------|      |      |      |      |      |  Mute  |
   * |--------+------+------+------+------+------|      |           |      |------+------+------+------+------+--------|
   * |        |      |      |      |      |      |      |           |      |      |      |      |      |      |        |
   * `--------+------+------+------+------+-------------'           `-------------+------+------+------+------+--------'
   *   |      |      |      |      |      |                                       |      |      |      |      |      |
   *   `----------------------------------'                                       `----------------------------------'
   *                                        ,-------------.       ,-------------.
   *                                        |      |      |       |      |      |
   *                                 ,------|------|------|       |------+------+------.
   *                                 |      |      |      |       |      |      |      |
   *                                 |      |      |------|       |------|      |      |
   *                                 |      |      |      |       |      |      |      |
   *                                 `--------------------'       `--------------------'
   */
  [FKEYS] = LAYOUT_ergodox(
    // left hand side - main
    KC_MEDIA_REWIND,        KC_F1,              KC_F2,              KC_F3,                KC_F4,              KC_F5,              KC_F11,
    KC_MEDIA_PLAY_PAUSE,    _______,            _______,            _______,              _______,            _______,            _______,
    KC_MEDIA_FAST_FORWARD,  _______,            _______,            _______,              _______,            _______,            /* --- */
    _______,                _______,            _______,            _______,              _______,            _______,            _______,
    _______,                _______,            _______,            _______,              _______,            /* --- */           /* --- */

    // left hand side - thumb cluster
    /* --- */               _______,            _______,
    /* --- */               /* --- */           _______,
    _______,                _______,            _______,

    // right hand side - main
    KC_F12,                 KC_F6,              KC_F7,              KC_F8,                KC_F9,              KC_F10,             KC_AUDIO_VOL_UP,
    _______,                _______,            _______,            _______,              _______,            _______,            KC_AUDIO_VOL_DOWN,
    /* --- */               _______,            _______,            _______,              _______,            _______,            KC_AUDIO_MUTE,
    _______,                _______,            _______,            _______,              _______,            _______,            _______,
    /* --- */               /* --- */           _______,            _______,              _______,            _______,            _______,

    // right hand side - thumb cluster
    _______,                _______,            /* --- */
    _______,                /* --- */           /* --- */
    _______,                _______,            _______
  ),
};

// Send a key tap with a optional set of modifiers.
void tap_with_modifiers(uint16_t keycode, uint8_t force_modifiers) {
  uint8_t active_modifiers = get_mods();

  if ((force_modifiers & MODS_SHIFT) && !(active_modifiers & MODS_SHIFT)) register_code(KC_LSFT);
  if ((force_modifiers & MODS_CTRL) && !(active_modifiers & MODS_CTRL)) register_code(KC_LCTRL);
  if ((force_modifiers & MODS_ALT) && !(active_modifiers & MODS_ALT)) register_code(KC_LALT);
  if ((force_modifiers & MODS_GUI) && !(active_modifiers & MODS_GUI)) register_code(KC_LGUI);

  register_code(keycode);
  unregister_code(keycode);

  if ((force_modifiers & MODS_SHIFT) && !(active_modifiers & MODS_SHIFT)) unregister_code(KC_LSFT);
  if ((force_modifiers & MODS_CTRL) && !(active_modifiers & MODS_CTRL)) unregister_code(KC_LCTRL);
  if ((force_modifiers & MODS_ALT) && !(active_modifiers & MODS_ALT)) unregister_code(KC_LALT);
  if ((force_modifiers & MODS_GUI) && !(active_modifiers & MODS_GUI)) unregister_code(KC_LGUI);
}

// Special remapping for keys with different keycodes/macros when used with shift modifiers.
bool process_record_user_shifted(uint16_t keycode, keyrecord_t *record) {
  uint8_t active_modifiers = get_mods();
  uint8_t shifted = active_modifiers & MODS_SHIFT;

  // Early return on key release
  if(!record->event.pressed) {
    return true;
  }

  if(shifted) {
    clear_mods();

    // The sent keys here are all based on US layout. I.e. look up how to
    // produce the key you want using the german qwertz, then look in
    // keymap_german what you need to send to get that.
    switch(keycode) {
      case NEO2_1:
        // degree symbol
        SEND_STRING(SS_LSFT("`"));
        break;
      case NEO2_2:
        // section symbol
        SEND_STRING(SS_LSFT("3"));
        break;
      case NEO2_3:
        SEND_STRING(SS_RALT("1"));
        break;
      case NEO2_4:
        // right angled quote
        SEND_STRING(SS_RALT("z"));
        break;
      case NEO2_5:
        // left angled quote
        SEND_STRING(SS_RALT("x"));
        break;
      case NEO2_6:
        // dollar sign
        SEND_STRING(SS_LSFT("4"));
        break;
      case NEO2_7:
        // euro sign
        SEND_STRING(SS_RALT("e"));
        break;
      case NEO2_8:
        // low9 double quote
        SEND_STRING(SS_RALT("v"));
        break;
      case NEO2_9:
        // left double quote
        SEND_STRING(SS_RALT("b"));
        break;
      case NEO2_0:
        // right double quote
        SEND_STRING(SS_RALT("n"));
        break;
      case NEO2_MINUS:
        // em dash
        SEND_STRING(SS_LSFT(SS_RALT("/")));
        break;
      case NEO2_COMMA:
        // en dash
        SEND_STRING(SS_RALT("/"));
        break;
      case NEO2_DOT:
        // bullet
        SEND_STRING(SS_RALT(","));
        break;
      case NEO2_SHARP_S:
        // german sharp s
        SEND_STRING(SS_LSFT(SS_RALT("s")));
        break;
      default:
        set_mods(active_modifiers);
        return true;
    }

    set_mods(active_modifiers);
    return false;
  } else {
    switch(keycode) {
      case NEO2_1:
        SEND_STRING(SS_TAP(X_1));
        break;
      case NEO2_2:
        SEND_STRING(SS_TAP(X_2));
        break;
      case NEO2_3:
        SEND_STRING(SS_TAP(X_3));
        break;
      case NEO2_4:
        SEND_STRING(SS_TAP(X_4));
        break;
      case NEO2_5:
        SEND_STRING(SS_TAP(X_5));
        break;
      case NEO2_6:
        SEND_STRING(SS_TAP(X_6));
        break;
      case NEO2_7:
        SEND_STRING(SS_TAP(X_7));
        break;
      case NEO2_8:
        SEND_STRING(SS_TAP(X_8));
        break;
      case NEO2_9:
        SEND_STRING(SS_TAP(X_9));
        break;
      case NEO2_0:
        SEND_STRING(SS_TAP(X_0));
        break;
      case NEO2_MINUS:
        SEND_STRING(SS_TAP(X_SLASH));
        break;
      case NEO2_COMMA:
        SEND_STRING(SS_TAP(X_COMMA));
        break;
      case NEO2_DOT:
        SEND_STRING(SS_TAP(X_DOT));
        break;
      case NEO2_SHARP_S:
        // german sharp s
        SEND_STRING(SS_TAP(X_MINS));
        break;
      case NEO2_L3_CIRCUMFLEX:
        SEND_STRING(SS_TAP(X_GRAVE) SS_TAP(X_SPACE));
        break;
      case NEO2_L3_BACKTICK:
        SEND_STRING(SS_LSFT("=") SS_TAP(X_SPACE));
        break;
      case YELDIR_CTLTAB:
        SEND_STRING(SS_LCTL("\t"));
        break;
      case YELDIR_CTLSTAB:
        SEND_STRING(SS_LSFT(SS_LCTL("\t")));
        break;
      default:
        return true;
    }

    return false;
  }
}

// Runs for each key down or up event.
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
  switch(keycode) {
    case KC_LSHIFT:
      if (record->event.pressed) {
        capslock_state |= (MOD_BIT(KC_LSHIFT));
      } else {
        capslock_state &= ~(MOD_BIT(KC_LSHIFT));
      }
      break;
    case KC_RSHIFT:
      if (record->event.pressed) {
        capslock_state |= MOD_BIT(KC_RSHIFT);
      } else {
        capslock_state &= ~(MOD_BIT(KC_RSHIFT));
      }
      break;
    case YELDIR_AC:
      if (record->event.pressed) {
        register_code(KC_LALT);
        register_code(KC_LCTRL);
      } else {
        unregister_code(KC_LALT);
        unregister_code(KC_LCTRL);
      }
      break;
    case NEO2_LMOD3:
      if (record->event.pressed) {
        layer_on(NEO_3);
        neo3_state |= (1 << 1);
      } else {
        // Turn off NEO_3 layer unless it's enabled through NEO2_RMOD3 as well.
        if ((neo3_state & ~(1 << 1)) == 0) {
          layer_off(NEO_3);
        }
        neo3_state &= ~(1 << 1);
      }
      break;
    case NEO2_RMOD3:
      if (record->event.pressed) {
        neo3_timer = timer_read();
        neo3_state |= (1 << 2);
        layer_on(NEO_3);
      } else {
        // Turn off NEO_3 layer unless it's enabled through NEO2_LMOD3 as well.
        if ((neo3_state & ~(1 << 2)) == 0) {
          layer_off(NEO_3);
        }
        neo3_state &= ~(1 << 2);

        // Was the NEO2_RMOD3 key TAPPED?
        if (timer_elapsed(neo3_timer) <= 150) {
          if (neo3_state > 0) {
            // We are still in NEO_3 layer, send keycode and modifiers for @
            tap_with_modifiers(KC_2, MODS_SHIFT);
            return false;
          } else {
            // Do the normal key processing, send y
            tap_with_modifiers(KC_Y, MODS_NONE);
            return false;
          }
        }
      }
      break;
  }

  if ((capslock_state & MODS_SHIFT) == MODS_SHIFT) {
    // CAPSLOCK is currently active, disable it
    if (host_keyboard_leds() & (1<<USB_LED_CAPS_LOCK)) {
      unregister_code(KC_LOCKING_CAPS);
    } else {
      register_code(KC_LOCKING_CAPS);
    }
    return false;
  }

  return process_record_user_shifted(keycode, record);
};


// Runs just one time when the keyboard initializes.
void matrix_init_user(void) {

};


// Runs constantly in the background, in a loop.
void matrix_scan_user(void) {
    uint8_t layer = biton32(layer_state);

    ergodox_board_led_off();
    ergodox_right_led_1_off();
    ergodox_right_led_2_off();
    ergodox_right_led_3_off();
    switch (layer) {
      // TODO: Make this relevant to the ErgoDox EZ.
        case 1:
            ergodox_right_led_1_on();
            break;
        case 2:
            ergodox_right_led_2_on();
            break;
        default:
            // none
            break;
    }
};
