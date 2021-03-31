# PIPE

Use C++ to implement pipeline and solve hazard.<br>
Input the instructions as binary.
> 5 stage : IF / ID / EXE / MEM / WB 

## Initialization
* Register <br>

| register | initial value |
| -------- | ------------- |
| $0 | 0 |
| $1 | 9 |
| $2 | 5 |
| $3 | 7 |
| $4 | 1 |
| $5 | 2 |
| $6 | 3 |
| $7 | 4 |
| $8 | 5 |
| $9 | 6 |

* Memory

| Memory | initial value |
| ------ | ------------- |
| 0x00 | 5 |
| 0x04 | 9 |
| 0x08 | 4 |
| 0x0C | 8 |

## Test data

* General : no hazard
* loadhazard
* branchhazard
* datahazard
