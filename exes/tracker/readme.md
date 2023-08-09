# Tracking items

The core functionality of tracking items is an instance of a conveyor for example.
A conveyor can withold couple of inputs and outputs as well as action points. 
Conveyors can be consecutive or send an item to left or right given the mechanical structure of the system.

```
        sensor(forced input)
          ↓               
--------------------------------------------
 +-----+       +-----+       +-----+
 |     |       |     |       |     |
 |     |       |     |       |     |
 +-----+       +-----+       +-----+
--------------------------------------------
```

