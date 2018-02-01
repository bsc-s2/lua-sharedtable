
## GC algorithm

### Terminology

-   Reference: A reference B means: B is a child item of A and B is reachable
    from A.

-   ADD: add a reference(from A to B).

-   DEL: delete a reference(from A to B).

### Data structure

-   `gc_round`:
    a int number that indicates a complete gc.
    A complete gc includes mark step and sweep step.

-   `item_status`:
    -   `unknown`:   defined to be: `gc_round+0` or any int smaller than `gc_round`. Not yet scanned.
    -   `reachable`: defined to be: `gc_round+1`. Reachable from one of the `roots`.
    -   `garbage`:   defined to be: `gc_round+2`. Confirmed to be unreachable from any `roots`.

-   `roots`:
    is a collection of items those are **root**s and should never be freed.
    A root item is where the mark-process starts.

-   `mark_queue`:
    is queue of items to mark as `reachable`.

-   `sweep_queue`:
    is queue of items being DEL-ed one or more times in this
    gc-round.

    Items in this queue might be in one of three status:

    -   Not marked(`unknown`) and being DEL-ed once or more.
    -   Is a garbage and marked(`reachable`) and being DEL-ed once or more.
    -   Is not a garbage and marked(`reachable`) and being DEL-ed once or more.

    The first status of items are freed in this gc-round.
    The other two status are left to next gc-round to determine.

-   `prev_sweep_queue`:
    is queue of items those can not be decided whether it is a garbage in
    previous round of gc.

    Since DEL puts an item into `sweep_queue`,
    after a full gc-round, all items in `prev_sweep_queue` are in one of the two
    status:

    -   `unknown`: it must be a garbage.
    -   `reachable`: it must NOT be a garbage.

-   `garbage_queue`:
    is queue of `garbage` item.

### GC workflow

#### Item reference operation

ADD/DEL reference of items and stepped-gc happens alternately.
A full gc-round might be split into several steps.
**A gc-step is protected by a gc lock**.
But between two gc-step, item referencing might be changed:

-   When ADD reference, the item should be put into
    `mark_queue`.

-   When DEL reference, the item should be removed from
    `sweep_queue` or `prev_sweep_queue`, and be put into `sweep_queue`.

#### GC

-   A complete GC round runs all following steps.
-   A stepped GC runs from step 3.


0.  Mark all items as `unknown`.
    This is done by incrementing `gc_round` by `4`.
    By the definition of `unknown`: `item_status <= gc_round`, all items are now `unknown`.

1.  Put items in `roots` into `mark_queue`.

2.  Mark:

    Repeat until queue is empty:

    -   Pop one item from `mark_queue`.
    -   Mark it as `reachable`.
    -   Put all its `unknown` child items to `mark_queue`.

    > After this mark-step, all items those are reachable from any `root`s, are
    > marked as `reachable`.
    > But not all `reachable` are actually reachable.
    > There might be item being DEL-ed after being marked as
    > `reachable`.

    > And all items being DEL-ed in this gc-round are in `sweep_queue`.
    > `prev_sweep_queue` does not contain any item being DEL-ed in this
    > gc-round.

3.  Sweep-prev:

    Repeat until queue is empty:

    -   Pop one item from `prev_sweep_queue`.
    -   If it is `reachable`, skip.
    -   If it is `unknown`:
        -   Mark it as `garbage`.
        -   Put it into `garbage_queue`.
        -   Put all its `unknown` child items to `prev_sweep_queue`.

    > Because a DEL operation removes an item from `prev_sweep_queue`(and puts
    > it into `sweep_queue`),
    > all items in `prev_sweep_queue` are NOT DEL-ed in this gc-round.
    >
    > Thus these item status are determined:
    > - `reachable` item is actually reachable,
    > - and `unknown` item is actually garbage.

4.  Sweep:

    Repeat until queue is empty:

    -   Pop one item from `sweep_queue`.
    -   If it is `reachable`, put it into `prev_sweep_queue`.
    -   If it is `unknown`:
        -   Mark it as `garbage`.
        -   Put it into `garbage_queue`.
        -   Put all its `unknown` child items to `sweep_queue`.

    > `reachable` item in `sweep_queue` means it has been ADD-ed and DEL-ed in
    > this gc-round and we could not determine its actual status.
    > Leave such item to next gc-round by putting it into `prev_sweep_queue`.

    > After this step, `sweep_queue` is empty and `prev_sweep_queue` contains
    > undetermined items.

5.  Free items in `garbage_queue` one by one:

    Scan queue:

    -   Pop one item from `garbage_queue`.
    -   Release references to its children.
    -   Release item.

6.  GC completed.


## Example of the three item status in sweep_queue: (absolute garbage, reachable garbage, reachable non-garbage)

```
### Initial:

R
|`----.----.
v     v    v
C     D    E

mark_queue:  [R]
sweep_queue: []

### User DEL ref (R->C).

R
 `----.----.
      v    v
      D    E

mark_queue:  [R]
sweep_queue: [C]

### GC scans to D, D is reachable(`(r)` means reachable), only E is in mark_queue:

R
 `----.----.
      v    v
      D(r) E

mark_queue:  [E]
sweep_queue: [C]

### User ADD ref (D->E), then DEL ref (D->E).

R
 `----.----.
      v    v
      D(r) E

mark_queue:  [E]
sweep_queue: [C, E]

### GC scans all

R
 `----.----.
      v    v
      D(r) E(r)

mark_queue:  []
sweep_queue: [C, E(r)]

### User DEL ref (R->D)

R
 `---------.
           v
           E(r)

mark_queue:  []
sweep_queue: [C, E(r), D(r)]

### After GC

-   C is unknown and is in sweep_queue, it is a determined garbage in this round.

-   D is reachable and is in sweep_queue, it can not be determined to be garbage.
    But it actually is. It will be freed(by moving it to prev_sweep_queue and it
    will not be reachable) in next round of GC.

-   E is reachable and is not a garbage. It will be reachable and removed from
    prev_sweep_queue in next round of GC.

### GC round 2, clear all reachable by incrementing gc_round

R
 `---------.
           v
           E

mark_queue:       [R]
prev_sweep_queue: [E, D]
sweep_queue:      []

### after scan and mark all reachable item:

R
 `---------.
           v
           E(r)

mark_queue:       []
prev_sweep_queue: [D]
sweep_queue:      []

### D is determined to be garbage and is freed.
```
