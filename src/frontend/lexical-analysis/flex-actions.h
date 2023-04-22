#ifndef FLEX_ACTIONS_HEADER
#define FLEX_ACTIONS_HEADER

#include "../../backend/support/shared.h"
#include "../syntactic-analysis/bison-parser.h"

/**
 * Se definen las acciones a ejecutar sobre cada patrón hallado mediante el
 * analizador léxico Flex. Este analizador solo puede identificar
 * construcciones regulares, ya que utiliza un autómata finito determinístico
 * (a.k.a. DFA), como mecanismo de escaneo y reconocimiento.
 *
 * Debería existir una acción por cada patrón disponible en Flex (es decir,
 * por cada regla).
 *
 * Solo aquellas acciones que tengan interés en conocer el largo del lexema
 * emplean el parámetro adicional "length". Esto es útil cuando se desea
 * asignar memoria dinámica, y por lo tanto, se debe saber exactamente el
 * largo de aquello que se va a almacenar.
 */

token ImagePatternAction(const char * lexeme);
token FilterPatternAction(const char * lexeme);
token SetPatternAction(const char * lexeme);

token ForPatternAction(const char * lexeme);
token InPatternAction(const char * lexeme);

token DotPatternAction(const char * lexeme);
token EqualPatternAction(const char * lexeme);
token OpenParenthesisPatternAction(const char * lexeme);
token CloseParenthesisPatternAction(const char * lexeme);
token QuotePatternAction(const char * lexeme);
token CommaPatternAction(const char * lexeme);
token SemiColonPatternAction(const char * lexeme);
token OpenCurlyBracePatternAction(const char * lexeme);
token CloseCurlyBracePatternAction(const char * lexeme);

token ApplyPatternAction(const char * lexeme);
token OverlapPatternAction(const char * lexeme);
token ResizePatternAction(const char * lexeme);
token TrimPatternAction(const char * lexeme);
token UnionPatternAction(const char * lexeme);
token SavePatternAction(const char * lexeme);
token ParameterPatternAction(const char * lexeme);

token WidthPatternAction(const char * lexeme);
token HeightPatternAction(const char * lexeme);
token PositionPatternAction(const char * lexeme);
token AxisPatternAction(const char * lexeme);
token FormatPatternAction(const char * lexeme);
token ExposurePatternAction(const char * lexeme);
token LuminosityPatternAction(const char * lexeme);
token ShadowsPatternAction(const char * lexeme);
token ContrastPatternAction(const char * lexeme);
token BrightnessPatternAction(const char * lexeme);
token SaturationPatternAction(const char * lexeme);
token OpacityPatternAction(const char * lexeme);

token VariableNamePatternAction(const char * lexeme);
token StringPatternAction(const char * lexeme);
token FloatPatternAction(const char * lexeme);

token CommentPatternAction(const char * lexeme);

token UnknownPatternAction(const char * lexeme);

void IgnoredPatternAction(const char * lexeme);

typedef enum tokens {
    // Código de error de Bison, que permite abortar el escaneo de lexemas cuando
	// se presente un patrón desconocido. El número "257" coincide con el valor
	// que Bison le otorga por defecto, pero además permite que el resto de
	// tokens continúen desde el valor "258" lo que permite proteger los IDs
	// internos que Bison reserva para crear "tokens literales":
	YYUNDEF = 257,

    IMAGE,
    FILTER,
    SET,

    FOR,
    IN,

    DOT,
    EQUAL,
    OPEN_PARENTHESIS,
    CLOSE_PARENTHESIS,
    QUOTE,
    COMMA,
    SEMI_COLON,
    OPEN_CURLY_BRACE,
    CLOSE_CURLY_BRACE,

    APPLY,
    OVERLAP,
    RESIZE,
    TRIM,
    UNION,
    SAVE,
    PARAMETER,

    WIDTH,
    HEIGHT,
    POSITION,
    AXIS,
    FORMAT,
    EXPOSURE,
    LUMINOSITY,
    SHADOWS,
    CONTRAST,
    BRIGHTNESS,
    SATURATION,
    OPACITY,

    VAR_NAME,
    STRING,
    FLOAT,

    COMMENT
}

#endif