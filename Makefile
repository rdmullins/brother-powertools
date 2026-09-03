CC = gcc
CFLAGS = -Wall -Wextra -std=c17 -Iinclude $(shell pkg-config --cflags libcurl json-c)
LDLIBS = $(shell pkg-config --libs libcurl json-c)

SOURCES = $(wildcard src/*.c)

all:
	$(CC) $(CFLAGS) $(SOURCES) -o powertools $(LDLIBS)

test: test-card-set test-catalog test-catalog-search test-openlibrary test-catalog-cards test-citation
	@echo "All tests passed."

test-card-set:
	$(CC) $(CFLAGS) tests/test_card_set.c src/cards.c src/card_set.c src/citation.c src/notes.c -o /tmp/test_card_set $(LDLIBS)
	/tmp/test_card_set

test-openlibrary:
	$(CC) $(CFLAGS) tests/test_openlibrary.c src/openlibrary.c src/catalog.c -o /tmp/test_openlibrary $(LDLIBS)
	/tmp/test_openlibrary

test-catalog:
	$(CC) $(CFLAGS) tests/test_catalog.c src/catalog.c -o /tmp/test_catalog $(LDLIBS)
	/tmp/test_catalog

test-catalog-search:
	$(CC) $(CFLAGS) tests/test_catalog_search.c src/catalog.c -o /tmp/test_catalog_search $(LDLIBS)
	/tmp/test_catalog_search

test-catalog-cards:
	$(CC) $(CFLAGS) tests/test_catalog_cards.c src/cards.c src/card_set.c src/citation.c src/notes.c -o /tmp/test_catalog_cards $(LDLIBS)
	/tmp/test_catalog_cards

test-citation:
	$(CC) $(CFLAGS) tests/test_citation.c src/citation.c -o /tmp/test_citation $(LDLIBS)
	/tmp/test_citation