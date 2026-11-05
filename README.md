# Práctica 02: Análisis Sintáctico con DeSiGNAR (LL(1))

## Cómo compilar y ejecutar

Desde la carpeta `compiladores-task`:

```bash
g++ -std=c++17 -I DeSiGNAR/include practica_02.cpp DeSiGNAR/src/hash.cpp -o practica_02
./practica_02
```

---

## Cambios realizados en `practica_02.cpp` (Actividad 4)

1. **Regla léxica en `make_lexer()`**:
   ```cpp
   lexer.add_token("MINUS", "-");
   ```
2. **Producción en `make_grammar()`**:
   ```cpp
   g.add_production("E'", Grammar::Sequence({"MINUS", "T", "E'"}));
   ```

---

## Diferencia entre Error Léxico y Error Sintáctico

El propio mensaje de `error.what()` que imprime el programa indica en qué fase ocurrió el fallo:

- **Error Léxico (`Lexer::tokenize: ...`)**:
  Ocurre en `tokenize_without_spaces()` **antes** de llegar al parser. Sucede cuando la cadena de entrada contiene un carácter que no coincide con ninguna regla registrada en `make_lexer()`.
  - **Ejemplo (`8 & 2`)**: El carácter `&` no es un número, ni `+`, `-`, `*`, `(`, `)`, ni espacio. El `Lexer` lo rechaza con:
    ```text
    Error: Lexer::tokenize: no rule matches at position 2 (character '&')
    ```

- **Error Sintáctico (`LLParser: ...`)**:
  Ocurre en `parser.parse(tokens)`. Aquí el `Lexer` sí reconoció todos los símbolos porque son tokens válidos del lenguaje, pero **el orden o estructura en que aparecen no cumple las producciones de la gramática**.
  - **Ejemplo 1 (`9 - * 2`)**: Todos los tokens existen (`9`, `-`, `*`, `2`), pero después de `MINUS` (`-`) la gramática espera un término `T` (que inicia con `NUM` o `LPAREN`) y en su lugar encuentra `TIMES` (`*`):
    ```text
    Error: LLParser: unexpected token 'TIMES' at position 4 while parsing 'T'`
    ```
  - **Ejemplo 2 (`(8 + 3`)**: Todos los tokens son válidos, pero se termina la entrada (`$`) cuando la producción `F -> LPAREN E RPAREN` esperaba el paréntesis de cierre `RPAREN`:
    ```text
    Error: LLParser: unexpected token '$' at position 5 (expected 'RPAREN')
    ```

---

## Registro de las pruebas sugeridas (Sección 4.2)

| Entrada | Resultado | Tipo de fallo | Por qué ocurre |
| :--- | :--- | :--- | :--- |
| `42` | **Aceptada** | Ninguno | Número válido (`E -> T -> F -> NUM`). |
| `(8 + 3) * 2` | **Aceptada** | Ninguno | Paréntesis y operadores bien formados. |
| `9 - 4 * 2` | **Aceptada** | Ninguno | Resta y multiplicación válidas (`*` tiene mayor precedencia al agruparse en `T`). |
| `9 - * 2` | **Rechazada** | **Error sintáctico** | Dos operadores seguidos (`-` y `*`); `LLParser` esperaba `NUM` o `(`. |
| `(8 + 3` | **Rechazada** | **Error sintáctico** | Falta el paréntesis de cierre `)`; `LLParser` llega al fin de cadena `$` esperando `RPAREN`. |
| `8 & 2` | **Rechazada** | **Error léxico** | El carácter `&` no pertenece al lenguaje; `Lexer::tokenize` no tiene regla para reconocerlo. |

---

## Respuestas a las preguntas teóricas de la guía

### 1. Sección 2 — Antes de programar
- **No terminales**: `E`, `E'`, `T`, `T'`, `F`.
- **Terminales de la gramática**: `PLUS`, `MINUS`, `TIMES`, `LPAREN`, `RPAREN`, `NUM` *(el token `WS` reconoce espacios en el lexer, pero se filtra antes de llamar al parser)*.
- **FIRST de `F` y distinción de alternativas**:
  Las producciones de `F` son `F -> LPAREN E RPAREN | NUM`, por lo que:
  $$\text{FIRST}(F) = \{\text{LPAREN}, \text{NUM}\}$$
  Sus alternativas se distinguen mirando el primer token (*lookahead*) porque $\text{FIRST}(\text{LPAREN } E \text{ RPAREN}) = \{\text{LPAREN}\}$ y $\text{FIRST}(\text{NUM}) = \{\text{NUM}\}$ son conjuntos disjuntos y ninguna deriva en $\varepsilon$.

---

## Cambios en FIRST, FOLLOW y continuación de `E'` (Sección 4.1 - Tarea 5)

### 1. ¿Qué terminales pueden iniciar una continuación de `E'`?
Al agregar la resta, las producciones de `E'` quedan así:
$$E' \to \text{PLUS } T \text{ } E' \mid \text{MINUS } T \text{ } E' \mid \varepsilon$$

- **Terminales que inician una continuación (no vacía) de `E'`**: **`PLUS`** (`+`) y **`MINUS`** (`-`).
  - Como el primer símbolo de una alternativa es `PLUS` y el de la otra es `MINUS`, son distintos entre sí y el parser LL(1) sabe inmediatamente cuál elegir mirando el siguiente token (*lookahead*).
- *(Nota: si el siguiente token es `RPAREN` `)` o el fin de cadena `$`, que están en $\text{FOLLOW}(E')$, entonces `E'` usa la producción vacía $\varepsilon$ y termina la continuación).*

### 2. ¿Qué conjuntos FIRST cambian?
**Solo cambia $\text{FIRST}(E')$**, porque fue el único no terminal al que se le agregó una nueva producción que empieza con `MINUS`:

| No terminal | FIRST antes | FIRST después | ¿Cambia? |
| :--- | :--- | :--- | :--- |
| **`E'`** | `{ PLUS, ε }` | `{ PLUS, MINUS, ε }` | **Sí** |
| **`E`** | `{ LPAREN, NUM }` | `{ LPAREN, NUM }` | No |
| **`T`** | `{ LPAREN, NUM }` | `{ LPAREN, NUM }` | No |
| **`T'`** | `{ TIMES, ε }` | `{ TIMES, ε }` | No |
| **`F`** | `{ LPAREN, NUM }` | `{ LPAREN, NUM }` | No |

### 3. ¿Qué conjuntos FOLLOW cambian?
**Cambian $\text{FOLLOW}(T)$, $\text{FOLLOW}(T')$ y $\text{FOLLOW}(F)$**, porque en la gramática `T` va seguido de `E'` (`E -> T E'`), así que todo lo que está en $\text{FIRST}(E')$ (excepto $\varepsilon$) pasa a formar parte de $\text{FOLLOW}(T)$, y de ahí se propaga a `T'` y a `F`:

| No terminal | FOLLOW antes | FOLLOW después | ¿Cambia? |
| :--- | :--- | :--- | :--- |
| **`E`** | `{ RPAREN, $ }` | `{ RPAREN, $ }` | No |
| **`E'`** | `{ RPAREN, $ }` | `{ RPAREN, $ }` | No |
| **`T`** | `{ PLUS, RPAREN, $ }` | `{ PLUS, MINUS, RPAREN, $ }` | **Sí** |
| **`T'`** | `{ PLUS, RPAREN, $ }` | `{ PLUS, MINUS, RPAREN, $ }` | **Sí** |
| **`F`** | `{ TIMES, PLUS, RPAREN, $ }` | `{ TIMES, PLUS, MINUS, RPAREN, $ }` | **Sí** |


# tarea-compiladores
