# SymbolDiff
A C++ library for symbolic differentiation

# Usage

Simply run the program and enter the equasion you wish to differentiate. SymbolDiff will output that expression's derivative in simplified form:
```
> f (x) = 3x^2+2x+1
< f'(x) = 6x+2

> f (x) = 2ax^0.5
< f'(x) = ax^-0.5

> f (x) = 1/x
< f'(x) = -1/x^2
```

# How it works

There are 5 main stages, 

### 1. Lexing

First the input is split into a series of strings representing each token:

```
> f (x) = 3x^2+2x+1

3x^2 + 0.5x + 1 -> [ "3", "x", "^", "2", "+", "0.5", "x", "+", "1" ]
```

### 2. Parsing

Next the tokens are parsed to build an expression tree

```
        +
       / \
      +   1
     / \
    /   \
   /     \
  *       *
 / \     / \
3   ^  0.5  x
   / \
  x   2
```

### 3. Differentiation

The rules of differentiation are applied (e.g product rule, chain rule, etc) in otder to generate an expression which is the derivative. The intermediate result is usually very complex and needs to be simplified significantly

### 4. Simplification

The resulting expression is simplified if possible

```
    +
   / \
  *   2
 / \
6   x
```

### 5. Printing the expression

At this stage paranthesis are added if required, and the operands might be flipped or even removed (e.g `a*31 -> 31*a -> 31a`)

```
< f'(x) = 6x+2
```
