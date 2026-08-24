# PowerTools Library Data Format

## Purpose

The PowerTools Library is a plain-text research and personal-library
management system.

It supports two related but distinct collections:

1. A **catalog** of books and other items owned or otherwise available
   to the user.
2. A **research bibliography** containing sources actually used or being
   used in research, together with associated research notes.

The system is designed around the traditional physical 3×5 card systems
used by libraries and researchers.

All information is stored as plain text. External services such as Open
Library are metadata sources only and are not the database.

---

# Design Principles

1. Plain text first.
2. No database engine dependency.
3. Human-readable records.
4. Stable format independent of external APIs.
5. Data must remain usable with ordinary Unix tools.
6. Bibliographic information is stored once and rendered into different
   output formats.
7. Physical 3×5 cards are a first-class output.
8. The physical card system should remain understandable and useful
   independently of the computer.
9. External API failures must not affect existing local records.
10. Data should be straightforward to import and export.

---

# The Two Collections

## Catalog

The catalog represents the user's library.

A catalog item does not have to have been used in research.

The traditional catalog generates:

- an author card;
- a title card;
- one subject card for each subject assigned to the item.

These cards belong to a single alphabetized physical card file.

## Research Bibliography

The research bibliography contains sources that have actually been used
or are currently being used in research.

A research source may be:

- a book;
- journal article;
- government report;
- newspaper article;
- archival document;
- website;
- database printout;
- electronic file;
- or another source type.

A research source is not required to exist in the personal catalog.

A research source has:

- a Chicago-style bibliographic citation;
- location/provenance information sufficient to find the source again;
- a stable bibliography-card identifier.

Research notes are associated with research sources.

---

# Catalog Records

Catalog records are stored in:

    catalog.db

Records use `KEY=VALUE` fields.

Records are separated by a line containing exactly:

    ---

Example:

    ID=000001
    AUTHOR=Faragher, John Mack
    TITLE=Women and Men on the Overland Trail
    PUBLISHER=Yale University Press
    PLACE=New Haven
    YEAR=2001
    ISBN10=0300089240
    ISBN13=9780300089240
    PAGES=240
    OLID=
    SUBJECTS=Oregon Trail;Western Migration;Women
    ---

## Catalog Fields

### ID

Unique PowerTools catalog identifier.

### AUTHOR

Author or authors.

### TITLE

Title of the work.

### PUBLISHER

Publisher of the particular edition.

### PLACE

Place of publication.

### YEAR

Publication year.

### ISBN10

ISBN-10, when available.

### ISBN13

ISBN-13, when available.

### PAGES

Number of pages, when known.

### OLID

Open Library identifier for the edition, when available.

### SUBJECTS

Semicolon-separated subject headings.

Example:

    SUBJECTS=Oregon Trail;Western Migration;Women

Subjects are used to generate physical subject cards.

---

# Catalog Card Generation

One catalog record can produce multiple physical cards.

## Author Card

The author card is filed alphabetically with all other catalog cards.

Conceptually:

    FARAGHER, John Mack

    Women and Men on the
    Overland Trail

The author card identifies the author and points to the work.

## Title Card

Conceptually:

    WOMEN AND MEN ON THE
    OVERLAND TRAIL

    Faragher, John Mack

The title card identifies the work by title.

## Subject Card

One subject card is generated for each subject.

Conceptually:

    OREGON TRAIL

    Faragher, John Mack
    Women and Men on the
    Overland Trail

A book with three subjects therefore generates three subject cards.

The physical catalog consists of all author, title, and subject cards
filed together alphabetically.

PowerTools should generate these cards from the catalog record rather
than storing preformatted card text.

---

# Research Source Records

Research sources are stored in:

    bibliography.db

Each record represents a source that has been used or may be used in
research.

Records use `KEY=VALUE` fields and are separated by:

    ---

Example:

    ID=000001
    TYPE=journal_article
    LOCATION=J-Stor
    LOCATION_DETAIL=Printed
    AUTHOR=Boyer, Paul
    TITLE=From Activism to Apathy: The American People and Nuclear Weapons, 1963-1980
    JOURNAL=The Journal of American History
    VOLUME=70
    ISSUE=4
    DATE=March 1984
    PAGES=821-844
    CARD_ID=BC-00001
    CATALOG_ID=
    ---

Another example:

    ID=000024
    TYPE=government_report
    LOCATION=Gov. Report
    LOCATION_DETAIL=Printout
    AUTHOR=Jordan, Nehemiah
    TITLE=U.S. Civil Defense Before 1950: The Roots of Public Law 920
    REPORT_NUMBER=Study S-212
    INSTITUTION=Institute for Defense Analyses
    DEPARTMENT=Economic and Political Studies Division
    DATE=May 1966
    CARD_ID=BC-00024
    CATALOG_ID=
    ---

---

# Research Source Fields

## ID

Unique PowerTools bibliography record identifier.

## TYPE

General source type.

Examples:

    book
    journal_article
    newspaper_article
    government_report
    archival_document
    website
    dissertation
    thesis
    printout
    file

The list may grow over time.

## LOCATION

Short human-readable description of where or how the source can be
found.

Examples:

    J-Stor
    Gov. Report
    UK Library
    Binder 7
    Computer

This is intentionally free-form.

PowerTools should not require users to select from a rigid list.

## LOCATION_DETAIL

Additional information about the source location.

Examples:

    Printed
    Special Collections
    Box 14
    PDF
    Printout

This field may be blank.

## AUTHOR

Author or authors.

## TITLE

Title of the work, article, report, etc.

Additional fields may be used depending on source type.

## CARD_ID

Stable identifier for the physical bibliography card.

Bibliography card identifiers use the form:

    BC-00001
    BC-00002
    BC-00003

The identifier is printed in the upper-right corner of the physical
bibliography card.

The identifier belongs to the research source record and should remain
stable even if the citation is later edited.

## CATALOG_ID

Optional relationship to a catalog record.

A research source does not have to exist in the catalog.

For example, a journal article found through JSTOR may have no
corresponding catalog record.

If the source is also represented in the catalog, `CATALOG_ID` connects
the two records.

---

# Bibliographic Citation

The citation printed on a bibliography card is generated from the
research source fields.

The database should not require the complete citation to be stored as
a single text field.

This allows PowerTools to:

- generate Chicago-style citations;
- correct individual metadata fields;
- support additional citation styles in the future;
- export structured information.

The initial bibliography renderer should produce Chicago-style
citations.

---

# Bibliography Cards

Each research source produces one bibliography card.

The card is a physical locator record, not merely a citation.

The card contains three principal elements:

1. Card identifier;
2. location/provenance information;
3. complete Chicago-style citation.

The identifier appears in the upper-right corner.

Location/provenance information appears at the upper left.

The citation follows below.

The citation uses a hanging indentation when appropriate.

Example layout:

                 BC-00001

    J-Stor
    Printed

         Boyer, Paul. "From Activism to Apathy:
         The American People and Nuclear
         Weapons, 1963-1980." The Journal
         of American History 70, no. 4
         (March 1984): 821-844.

Another example:

                 BC-00024

    Gov. Report
    Printout

         Jordan, Nehemiah. U.S. Civil Defense Before
         1950: The Roots of Public Law 920. Study
         S-212. Institute for Defense Analyses:
         Economic and Political Studies Division.
         May 1966.

The exact line width and wrapping will be determined by testing on the
Brother PowerNote.

---

# Research Notes

Research notes are stored in:

    notes.db

Each note belongs to a research bibliography source.

Example:

    ID=000001
    SOURCE_ID=000001
    SHORT_TITLE=Women on the trail
    PAGE=47
    NOTE=...
    CARD_ID=RN-00001
    ---

## Note Fields

### ID

Unique PowerTools note identifier.

### SOURCE_ID

The `ID` of the associated bibliography record.

### SHORT_TITLE

A brief heading describing where or how the note was used.

This is printed at the top of the physical note card.

The short title is intentionally distinct from the source title.

### PAGE

Page number or other source reference.

Examples:

    47
    47-49
    Introduction
    n. 14

### NOTE

The research note itself.

The note may contain either a quotation, paraphrase, observation,
summary, or other research material.

### CARD_ID

Stable physical note-card identifier.

Note card identifiers use the form:

    RN-00001
    RN-00002
    RN-00003

---

# Research Note Cards

Each research note normally occupies one physical card.

Conceptually:

    WOMEN ON THE TRAIL

    Faragher, Women and Men on the
    Overland Trail

    p. 47

    [research note]

The short title is the principal filing/reminder heading.

If a note is too long for one card, PowerTools may divide it into
multiple numbered cards.

---

# 3×5 Card Output

3×5 cards are a first-class output format.

PowerTools generates card content as ordinary text.

The resulting text may be:

- displayed on screen;
- saved as a `.txt` file;
- transferred to the Brother PowerNote;
- printed by another system;
- archived independently of PowerTools.

The database does not store preformatted cards.

---

# Card Rendering

The card renderer is responsible for:

1. enforcing the configured card width;
2. wrapping text;
3. preserving words where possible;
4. splitting exceptionally long words only when necessary;
5. maintaining headings;
6. maintaining hanging indentation for bibliography citations;
7. keeping related information together;
8. numbering continuation cards when required.

Card dimensions and line width should be configurable.

The renderer should not assume that the Linux terminal's width is the
physical card width.

---

# Complete Card Sets

PowerTools should eventually support generating complete physical
backup sets.

## Catalog Card Set

For a selected catalog item:

    Author card
    Title card
    Subject card 1
    Subject card 2
    ...

For the entire catalog:

    Library -> Catalog Cards -> Entire Catalog

The result is a complete set of traditional catalog cards.

## Research Card Set

For a selected research source:

    Bibliography card
    Research note card 1
    Research note card 2
    ...

For the entire research bibliography:

    Library -> Research Cards -> Entire Bibliography

The result is a complete physical backup of the research card file.

---

# PowerNote Transfer

Card output should integrate with the existing PowerTools transfer
system.

PowerTools should be able to:

1. generate the requested card set;
2. calculate its size;
3. divide it into transfer-sized parts;
4. transfer the parts to the Brother;
5. allow the user to save each part;
6. continue until the complete card set has been transferred.

The user should not have to manually prepare individual card files.

---

# External Metadata

The first planned external metadata source is Open Library.

External APIs are lookup services, not the database.

An ISBN lookup may:

1. accept an ISBN;
2. query Open Library;
3. retrieve metadata;
4. display the proposed catalog record;
5. allow the user to review or modify it;
6. save the local record.

The local PowerTools record remains authoritative.

Existing records must continue to work if the external API becomes
unavailable.

---

# Import and Export

Because all records are plain text, PowerTools should eventually
support:

- text import;
- text export;
- CSV export;
- JSON export;
- bibliography export;
- card export.

The native PowerTools format must remain the simplest and most
recoverable representation.

---

# Future Possibilities

The format should leave room for:

- title/author Open Library searches;
- additional metadata sources;
- multiple bibliography styles;
- additional source types;
- call numbers;
- library locations;
- acquisition information;
- note searching;
- subject searching;
- selected card-set export;
- complete-library card export;
- CSV and JSON conversion;
- importing existing bibliographies;
- synchronization with other systems.

These features should not complicate the initial implementation unless
necessary.