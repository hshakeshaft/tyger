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

- [ ] basics:
    - [ ] `var`
    - [ ] identifiers
    - [ ] integer literals
    - [ ] string literals
    - [ ] `println`
    - [ ] operators (`+ - * / = == !=` etc)

## Parsing

- [ ] parse all elements in `lexing`


## Evaluating

- [ ] evaluate the specified nodes from lexing and parsing
