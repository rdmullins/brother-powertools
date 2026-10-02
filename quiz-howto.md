# PowerTools Quiz: Creating Custom Question Decks

PowerTools uses the BSDGames `quiz` format, which makes it surprisingly easy to build your own study decks.

A deck is simply a text file containing **questions and acceptable answers**, plus an `index` file that tells `quiz` what the fields mean.

## 1. Create a deck directory

PowerTools custom decks live here:

```text
~/brother-powertools/data/quiz/
```

For example:

```text
~/brother-powertools/data/quiz/jeopardy/
```

You can also keep all your decks directly in `data/quiz/`.

---

## 2. Create the question file

Each line is one record. Fields are separated by a colon (`:`).

For example:

```text
Commodore 64:6510|MOS 6510
Apple II:6502|MOS 6502
IBM PC 5150:8088|Intel 8088
Macintosh 128K:68000|Motorola 68000
```

The first field is the **question** and the second field is the **answer**.

So:

```text
Commodore 64:6510|MOS 6510
```

means:

> **Question:** Commodore 64
> **Answer:** 6510

### Multiple acceptable answers

Separate alternatives with `|`:

```text
Commodore 64:6510|MOS 6510
```

Either `6510` or `MOS 6510` will be accepted.

This is especially useful for Jeopardy prep, where you may want to accept abbreviations, alternate spellings, or common forms.

---

## 3. Create an index entry

The `index` file tells `quiz` where the deck lives and gives names to its fields.

For example:

```text
/home/roger/brother-powertools/data/quiz/retro:computer:cpu
```

The parts are:

```text
/path/to/deck:field1:field2
```

So this tells Quiz:

```text
deck = retro
field 1 = computer
field 2 = cpu
```

Once the index exists, you can quiz in either direction.

---

## 4. Quiz in either direction

Given:

```text
computer:cpu
```

you can run:

```bash
quiz -i ~/brother-powertools/data/quiz/index computer cpu
```

This asks:

> Commodore 64?

and expects:

> 6510

Reverse the fields:

```bash
quiz -i ~/brother-powertools/data/quiz/index cpu computer
```

and it asks:

> 6510?

with:

> Commodore 64

This **bidirectional capability is extremely useful for studying**.

---

## 5. More than two fields

A deck can contain more than two fields.

For example:

```text
Commodore 64:6510:1982:Commodore
Apple II:6502:1977:Apple
Macintosh 128K:68000:1984:Apple
```

The index could be:

```text
/home/roger/brother-powertools/data/quiz/retro:computer:cpu:year:company
```

Now you can create several different study combinations:

```bash
quiz -i ~/brother-powertools/data/quiz/index computer cpu
quiz -i ~/brother-powertools/data/quiz/index cpu computer
quiz -i ~/brother-powertools/data/quiz/index computer year
quiz -i ~/brother-powertools/data/quiz/index year computer
quiz -i ~/brother-powertools/data/quiz/index company computer
```

**One dataset can therefore become many quizzes.**

---

## 6. Useful answer patterns

Quiz supports alternatives with `|`.

```text
William Shakespeare:Shakespeare|William Shakespeare
```

Optional portions can be represented with `{}`.

For example:

```text
U.S.S. Enterprise:{the }enterprise|Enterprise
```

This allows more than one form of an answer.

Keep the patterns reasonably simple. The goal is to make studying easier, not to build a programming language.

---

## 7. Testing a new deck

Before adding a deck to the PowerTools menu, test it directly from the terminal.

For example:

```bash
quiz -i ~/brother-powertools/data/quiz/index category1 category2
```

Then test the reverse:

```bash
quiz -i ~/brother-powertools/data/quiz/index category2 category1
```

If both work, the deck is ready to integrate into PowerTools.

---

## 8. A good format for Jeopardy prep

For Jeopardy, think in terms of **facts that can be recalled in multiple directions**.

For example:

```text
Mount Everest:Nepal
Nile River:Egypt
Canberra:Australia
```

could become:

```text
place:country
```

and allow:

```bash
quiz -i ~/brother-powertools/data/quiz/index place country
```

as well as:

```bash
quiz -i ~/brother-powertools/data/quiz/index country place
```

For richer subjects, use additional fields:

```text
Gettysburg:1863:Pennsylvania:Civil War
Appomattox:1865:Virginia:Civil War
Yorktown:1781:Virginia:Revolutionary War
```

Possible categories:

```text
battle:year:state:war
```

That gives you several different ways to drill the same material.

---

## 9. Recommended workflow

```text
1. Pick a subject
       ↓
2. Decide what facts you want to know
       ↓
3. Create a data file
       ↓
4. Give each field a meaningful name
       ↓
5. Add the deck to index
       ↓
6. Test both directions
       ↓
7. Add useful combinations to PowerTools
       ↓
8. Study on the Brother
```

### The golden rule

**Build datasets, not individual quizzes.**

If you collect good factual records with several useful fields, PowerTools can turn the same material into many different study drills.

That's where this gets really powerful for Jeopardy prep.
