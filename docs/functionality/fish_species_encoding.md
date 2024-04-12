# Fish species, how they are stored

In order to utilize the same numerical values for fish species 
indepent of the customer using our systems we have utilized
a predefined list of fish species obtained from fao see https://www.fao.org/fishery/en/collection/asfis/en

In addition to having a defined list of fish species we have a system
of encoding the three letter markings into integers that makes it easy
to transmit to plc controllers and any other system that requires knowledge
of the product being processed.

Due to our requirement of plc's having an easy time understanding the
encoding, we limited the size of this integer to 16-bits. This makes it
easier to transmit over modbus and matches the word size for most plc controllers.

### Examples
To convert a three letter marking to an integer we use the letters placement
in the alphabet. For reference here is the alphabet with each letters location marked.
```
0 A
1 B
2 C
3 D
4 E
5 F
6 G
7 H
8 I
9 J
10 K
11 L
12 M
13 N
14 O
15 P
16 Q
17 R
18 S
19 T
20 U
21 V
22 W
23 X
24 Y
25 Z
```
To convert a three letter marking to an integer we use the following formula
```
(26^2 * first_letter) + (26 * second_letter) + third_letter
```
Here is an example for the fish species `COD`
```
(26^2 * 2) + (26 * 14) + 3 = 1719

```
Here is an example for the fish species `HAD`
```
(26^2 * 7) + (26 * 0) + 3 = 473
```

## Speciality markings
Anybody can see from this example and the fact that we limited the size of our integer to ```2^16```
that there is still some space. We have created `metadata` that can also be sent as a species.

The highest three letter code we have to create is `ZZZ` with a value of `17575` so the lowest speciality
code we can use is `17576`. The following metadata has been defined and used in our systems.

```
|------|------------|
| Code | Name       |
|------|------------|
| !UNS | UNSURE     |
| !EMP | EMPTY      |
| !DBL | DOUBLE     |
| !GAR | GARBAGE    |
| !GIG | GIGOLO     |
| !ICP | INCOMPLETE |
| !SNG | SINGLE     |
| !DMG | DAMAGED    |
| !BAG | bag        |
|------|------------|
```
We then define `offset = 17576` then our equation for calculating speciality codes is
```
offset + (26^2 * first_letter) + (26 * second_letter) + third_letter
```

### Examples
Here is an example for the speciality code `!UNS`
```
17576 + (26^2 * 20) + (26 * 13) + 18 = 31452
```
`!GIG`
```
17576 + (26^2 * 6) + (26 * 8) + 6 = 21846
```
`!EMP`
```
17576 + (26^2 * 2) + (26 * 12) + 15 = 20607
```