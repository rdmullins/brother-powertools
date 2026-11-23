# Jeopardy broad-subject Quiz decks

Generated from `JEOPARDY_CSV.csv`.

## What's included

- `subjects/`: one BSDGames Quiz deck per broad subject.
- `subjects.index`: index entries for the generated decks.
- `generate_subject_decks.py`: reproducible generator with editable category-name rules.

Each Quiz record has two fields:

`BROAD SUBJECT / ORIGINAL CATEGORY / VALUE / CLUE:ANSWER`

The original Jeopardy category is preserved in every prompt. Colons inside clue/answer text are changed to em dashes so they do not break Quiz's two-field format.

## Install

From the root of your `brother-powertools` checkout, copy the generated directory and index:

```bash
mkdir -p data/quiz/subjects
cp -a subjects/. data/quiz/subjects/
cp subjects.index data/quiz/subjects.index
```

The index currently uses the known repo path `/home/roger/brother-powertools/data/quiz/subjects`.
If your checkout lives elsewhere, edit that prefix in `data/quiz/subjects.index`.

## Test

From anywhere:

```bash
quiz -i /home/roger/brother-powertools/data/quiz/subjects.index -n 5 history response
quiz -i /home/roger/brother-powertools/data/quiz/subjects.index -n 5 science_nature response
quiz -i /home/roger/brother-powertools/data/quiz/subjects.index -n 5 literature response
```

The first field name is the deck slug; the second field is always `response`.

## Important classification note

This is a first-pass, editable taxonomy based primarily on recognizable words in the original category label. Categories that do not match a rule go into `GENERAL KNOWLEDGE`. A few playful category names may therefore be imperfectly classified. The original category is always retained, making it easy to audit and reclassify later.

Rows with an empty clue or answer are skipped. Missing dollar values are shown as `VALUE N/A`.

## Generated deck counts

Total usable clues: **216,927**

- GENERAL KNOWLEDGE: 106,459 clues (17,358 original category labels)
- HISTORY: 13,103 clues (1,320 original category labels)
- GEOGRAPHY: 11,150 clues (987 original category labels)
- LITERATURE: 10,945 clues (1,118 original category labels)
- LANGUAGE & WORDS: 9,989 clues (1,082 original category labels)
- SCIENCE & NATURE: 9,984 clues (764 original category labels)
- FILM & TELEVISION: 9,329 clues (1,193 original category labels)
- PEOPLE & SOCIETY: 9,108 clues (905 original category labels)
- WORDPLAY & MIXED: 8,702 clues (504 original category labels)
- MUSIC & PERFORMING ARTS: 7,610 clues (776 original category labels)
- ART & DESIGN: 3,725 clues (297 original category labels)
- SPORTS: 3,108 clues (367 original category labels)
- FOOD & DRINK: 3,058 clues (279 original category labels)
- RELIGION & MYTHOLOGY: 2,988 clues (278 original category labels)
- CULTURE & GENERAL: 2,244 clues (186 original category labels)
- BUSINESS & ECONOMICS: 1,885 clues (221 original category labels)
- MATH & MEASUREMENT: 1,374 clues (126 original category labels)
- TECHNOLOGY & INVENTIONS: 1,136 clues (113 original category labels)
- TRANSPORTATION: 1,030 clues (101 original category labels)
