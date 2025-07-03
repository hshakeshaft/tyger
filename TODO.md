# Infra

- [ ] disable direct push to main
- [ ] stop test job running on push to main
- [ ] Make tests run on Windows too
- [ ] setup "build" step for distribution

--- 

# V1 Lang:

Implement small subset of end language first, think this can be variable binding,
expression evaluation, and builtin `println` evaluation.

## Lexing

- [x] basics:
    - [x] `var`
    - [x] identifiers
    - [x] integer literals
    - [x] string literals
    - [x] `println`
    - [x] operators (`+ - * / = == !=` etc)

## Parsing

- [x] parse all elements in `lexing`


## Evaluating

- [ ] evaluate the specified nodes from lexing and parsing
