
## GC algorithm

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
    is queue of items whose references are deleted one or more times.

-   `prev_sweep_queue`:
    is queue of items those can not be decided whether it is a garbage in
    previous round of gc.

-   `garbage_queue`:
    is queue of `garbage` item.

### GC workflow

A complete GC round runs all following steps.

A stepped GC runs from step 3.

0.  Mark all items as `unknown`.
    This is done by incrementing `gc_round` by `4`.
    By the definition of `unknown`: `item_status <= gc_round`, all items are now `unknown`.

1.  Put items in `roots` into `mark_queue`.

2.  Mark:

    Repeat until queue is empty:

    -   Pop one item from `mark_queue`.
    -   Mark it as `reachable`.
    -   Put all its `unknown` child items to `mark_queue`.

3.  Sweep-prev:

    Repeat until queue is empty:

    -   Pop one item from `prev_sweep_queue`.
    -   If it is `reachable`, skip.
    -   If it is `unknown`:
        -   Mark it as `garbage`.
        -   Put it into `garbage_queue`.
        -   Put all its `unknown` child items to `prev_sweep_queue`.

4.  Sweep:

    Scan queue:

    -   Pop one item from `sweep_queue`.
    -   If it is `reachable`, put it into `prev_sweep_queue`.
    -   If it is `unknown`:
        -   Put it into `garbage_queue`.

5.  Free items in `garbage_queue` one by one:

    Scan queue:

    -   Pop one item from `garbage_queue`.
    -   Mark it as `garbage`.
    -   Put all its `unknown` child items to `garbage_queue`.
    -   Release references to its children.

6.  Replace `prev_sweep_queue` with `sweep_queue`.
    Empty `sweep_queue`.

    > A `reachable` item in this gc round can not be confirmed to be a garbage.
    > Those items must be kept. Otherwise, if it is actually a garbage, its
    > memory will never be freed.
    > Thus we move these items to `prev_sweep_queue`.
    > Then in next round of GC, these `reachable` items will be reset to
    > `unknown`.
    > If these items are not reachable in next GC round, we confirms that
    > these items are garbage.

7.  GC completed.

### Item reference operation

Adding/deleting reference of items happens alternately.

-   When adding reference, the item being referenced should be added to
    `mark_queue`.

-   When deleting reference, the item being de-referenced should be removed from
    `sweep_queue` or `prev_sweep_queue`, and should be added to `sweep_queue`.

## Example of the three class of items in sweep_queue: (absolute garbage, reachable garbage, reachable non-garbage)

```
### Initial:

R
|`----.----.
v     v    v
C     D    E

mark_queue:  [R]
sweep_queue: []

### User delete ref (R->C).

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

### User add ref (D->E), then delete ref (D->E).

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

### User delete ref (R->D)

R
 `---------.
           v
           E(r)

mark_queue:  []
sweep_queue: [C, E(r), D(r)]

### After GC

-   C is unknown and is in sweep_queue, it is a confirmed garbage in this round.

-   D is reachable and is in sweep_queue, it can not be confirmed to be garbage.
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

### D is confirmed to be garbage and is freed.
```
