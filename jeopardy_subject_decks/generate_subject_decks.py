#!/usr/bin/env python3
"""Generate broad-subject BSDGames quiz decks from JEOPARDY_CSV.csv.

Usage:
  python3 generate_subject_decks.py JEOPARDY_CSV.csv OUTPUT_DIR

The generated index assumes the decks will live in:
  /home/roger/brother-powertools/data/quiz/subjects

Edit classify_category() rules to refine the taxonomy.
"""
import csv, re, sys
from pathlib import Path
from collections import defaultdict

# Ordered, editable category-name rules. First match wins.
PUZZLE = ['BEFORE & AFTER', 'POTPOURRI', 'COMMON BONDS', 'RHYME TIME', 'STUPID ANSWERS', 'HODGEPODGE', 'ODDS & ENDS', 'LEFTOVERS', 'DOUBLE TALK', 'WORD PUZZLES', 'CROSSWORD', 'QUASI-RELATED', 'WHICH CAME FIRST', 'ADD A LETTER', 'LETTER PERFECT', 'NUMBER, PLEASE', 'FAMILIAR PHRASES', 'FACTS & FIGURES', '3-LETTER', '4-LETTER', '5-LETTER', '6-LETTER', '10-LETTER', '12-LETTER', '13-LETTER', '20-LETTER', 'ANAGRAM', 'HOMOPHONE', 'ABBREV']

RULES = [('HISTORY', ['\\bHISTORY\\b', '\\bWAR\\b', '\\bREVOLUTION\\b', '\\bCIVIL WAR\\b', '\\bPRESIDENTS?\\b', '\\bFIRST LAD', '\\bVICE PRESIDENT', '\\bCOLONIAL', '\\bANCIENT', '\\bMIDDLE AGES\\b', '\\bCENTURY\\b', '\\bHISTORIC', '\\bHISTORICAL', '\\bWORLD LEADERS\\b', '\\bPOLITIC', '\\bGOVERNMENT\\b', '\\bROYALTY\\b', '\\bMONARCH', '\\bEXPLORERS?\\b', '\\bRULERS\\b', '\\bOLD WEST\\b', '\\bBORN & DIED\\b', '\\bDECADE\\b', '\\bMILITARY\\b']), ('GEOGRAPHY', ['\\bGEOGRAPH', '\\bCITIES\\b', '\\bCITY\\b', '\\bCAPITALS?\\b', '\\bCOUNTRIES\\b', '\\bSTATES\\b', '\\bISLANDS?\\b', '\\bRIVERS?\\b', '\\bLAKES?\\b', '\\bMOUNTAINS?\\b', '\\bBODIES OF WATER\\b', '\\bOCEANS?\\b', '\\bSEAS?\\b', '\\bMAPS?\\b', '\\bNATIONAL PARKS?\\b', '\\bLANDMARKS?\\b', '\\bCONTINENTS?\\b', '\\bEUROPE\\b', '\\bASIA\\b', '\\bAFRICA\\b', '\\bAMERICA\\b', '\\bTRAVEL\\b', '\\bTOURISM\\b', '\\bWHERE AM I', '\\bHIGHWAYS\\b', '\\bBRIDGES\\b', '\\bCOLLEGE TOWNS\\b', '\\bJAPAN\\b']), ('SCIENCE & NATURE', ['\\bSCIENCE\\b', '\\bBIOLOGY\\b', '\\bZOOLOGY\\b', '\\bBOTANY\\b', '\\bCHEMISTRY\\b', '\\bPHYSICS\\b', '\\bASTRONOM', '\\bGEOLOGY\\b', '\\bMEDIC', '\\bANATOM', '\\bHEALTH\\b', '\\bBODY\\b', '\\bANIMALS?\\b', '\\bMAMMALS?\\b', '\\bBIRDS\\b', '\\bINSECTS?\\b', '\\bNATURE\\b', '\\bPLANTS?\\b', '\\bTREES\\b', '\\bFLOWERS\\b', '\\bWEATHER\\b', '\\bELEMENTS\\b', '\\bPLANETS?\\b', '\\bSPACE\\b', '\\bDINOSAURS?\\b', '\\bFOSSILS?\\b', '\\bOLOGY\\b', '\\bSCIENTISTS?\\b', '\\bENVIRONMENT\\b', '\\bFISH\\b', '\\bGARDEN', '\\bAGRICULTURE\\b', '\\bMETALS\\b', '\\bROCKS\\b', '\\bMINERALS\\b', '\\bVOLCANO', '\\bCONSTELLATIONS?\\b', '\\bSOLAR SYSTEM\\b', '\\bNUTRITION\\b', '\\bPSYCHOLOGY\\b', '\\bWEAPONS\\b']), ('LITERATURE', ['\\bLITERATURE\\b', '\\bLITERARY\\b', '\\bBOOKS?\\b', '\\bAUTHORS?\\b', '\\bNOVELS?\\b', '\\bFICTION\\b', '\\bNONFICTION\\b', '\\bPOET', '\\bSHAKESPEARE\\b', '\\bPLAYS?\\b', '\\bPLAYWRIGHTS?\\b', '\\bSHORT STORIES\\b', '\\bCHILDREN.?S LIT', '\\bBOOKSTORE\\b', '\\bBESTSELLERS\\b', '\\bWRITERS?\\b', '\\bPUBLISH', '\\bLIT\\b', '\\bCLIFFS NOTES\\b', '\\bNURSERY RHYMES\\b', '\\bKIDDY LIT\\b']), ('LANGUAGE & WORDS', ['\\bWORD', '\\bWORDS\\b', '\\bLANGUAGE', '\\bVOCAB', '\\bDICTIONARY\\b', '\\bSPELL', '\\bGRAMMAR\\b', '\\bPHRASES?\\b', '\\bQUOTES?\\b', '\\bQUOTATIONS\\b', '\\bSAYINGS\\b', '\\bPROVERBS\\b', '\\bIDIOMS?\\b', '\\bLETTER', '\\bANAGRAM', '\\bHOMOPHONE', '\\bSYNONYM', '\\bANTONYM', '\\bABBREV', '\\bETYMOLOG', '\\bLATIN\\b', '\\bGREEK\\b', '\\bFRENCH\\b', '\\bENGLISH\\b', '\\bADJECTIVES?\\b', '\\bNICKNAMES?\\b', '\\bFIRST NAMES?\\b', '\\bMIDDLE NAMES?\\b', '\\bPEN NAMES?\\b']), ('MUSIC & PERFORMING ARTS', ['\\bMUSIC\\b', '\\bOPERA\\b', '\\bBALLET\\b', '\\bDANCE\\b', '\\bTHEAT[ERRE]\\b', '\\bDRAMA\\b', '\\bMUSICAL', '\\bCOMPOS', '\\bSINGERS?\\b', '\\bINSTRUMENTS?\\b', '\\bSONGS?\\b', '\\bBANDS?\\b', '\\bCONDUCTORS?\\b', '\\bBROADWAY\\b', '\\bALBUMS?\\b', '\\bJAZZ\\b', '\\bANTHEMS?\\b']), ('FILM & TELEVISION', ['\\bMOVIES?\\b', '\\bFILM\\b', '\\bTELEVISION\\b', '\\bTV\\b', '\\bACTORS?\\b', '\\bACTRESSES\\b', '\\bOSCARS?\\b', '\\bDIRECTORS?\\b', '\\bSITCOM', '\\bCINEMA\\b', '\\bSCREEN\\b', '\\bEMMYS\\b', '\\bWHO PLAYED']), ('ART & DESIGN', ['\\bART\\b', '\\bARTISTS?\\b', '\\bPAINT', '\\bSCULPT', '\\bARCHITECT', '\\bMUSEUM', '\\bFASHION\\b', '\\bDESIGN\\b', '\\bPHOTOGRAPH', '\\bFURNITURE\\b', '\\bGEMS?\\b', '\\bJEWELRY\\b']), ('SPORTS', ['\\bSPORT', '\\bOLYMPIC', '\\bBASEBALL\\b', '\\bFOOTBALL\\b', '\\bBASKETBALL\\b', '\\bHOCKEY\\b', '\\bTENNIS\\b', '\\bGOLF\\b', '\\bBOXING\\b', '\\bRACING\\b', '\\bATHLET']), ('FOOD & DRINK', ['\\bFOOD\\b', '\\bDRINK\\b', '\\bCOOK', '\\bCUISINE\\b', '\\bFRUIT', '\\bVEGETABLE', '\\bHERBS?\\b', '\\bSPICES?\\b', '\\bPOTENT POTABLES\\b', '\\bBEVERAGE', '\\bDESSERT', '\\bCHEESE\\b', '\\bWINE\\b']), ('RELIGION & MYTHOLOGY', ['\\bRELIG', '\\bBIBLE\\b', '\\bTESTAMENT\\b', '\\bMYTH', '\\bSAINTS?\\b', '\\bGODS?\\b', '\\bGODDESSES\\b', '\\bCHURCH', '\\bFAITH\\b', '\\bSCRIPTURE', '\\bGENESIS\\b', '\\bBIBLICAL\\b']), ('BUSINESS & ECONOMICS', ['\\bBUSINESS\\b', '\\bINDUSTRY\\b', '\\bECONOM', '\\bFINANCE\\b', '\\bBANK', '\\bMONEY\\b', '\\bSTOCK', '\\bMARKET', '\\bBRAND NAMES\\b', '\\bCORPORAT', '\\bCOMPAN']), ('TECHNOLOGY & INVENTIONS', ['\\bTECHNOLOGY\\b', '\\bINVENT', '\\bCOMPUT', '\\bINTERNET\\b', '\\bELECTRON', '\\bENGINEER', '\\bGADGET', '\\bMACHINES?\\b', '\\bINNOVATION', '\\bWEBSITES?\\b', '\\bCOMMUNICATION\\b']), ('TRANSPORTATION', ['\\bTRANSPORT', '\\bAVIATION\\b', '\\bSHIPS?\\b', '\\bTRAINS?\\b', '\\bRAILROAD', '\\bAUTOMOB', '\\bCARS?\\b', '\\bBOATS?\\b', '\\bAIRCRAFT\\b', '\\bSAILING\\b']), ('PEOPLE & SOCIETY', ['\\bPEOPLE\\b', '\\bFAMOUS\\b', '\\bNOTABLE\\b', '\\bNAMES\\b', '\\bWOMEN\\b', '\\bMEN\\b', '\\bBIOGRAPH', '\\bCELEBRIT', '\\bORGANIZATIONS?\\b', '\\bOCCUPATIONS?\\b', '\\bCOLLEGES?\\b', '\\bUNIVERSITIES\\b', '\\bEDUCATION\\b', '\\bMAGAZINES?\\b', '\\bNEWSPAPERS?\\b', '\\bAWARDS?\\b', '\\bHONORS?\\b', '\\bHOLIDAYS?\\b', '\\bOBSERVANCES\\b', '\\bANNUAL EVENTS\\b', '\\bLIBRARIES\\b', '\\bFLAGS?\\b', '\\bCOLORS\\b', '\\bSYMBOLS\\b', '\\bRANKS\\b', '\\bTITLES\\b', '\\bETIQUETTE\\b', '\\bLAW\\b', '\\bSUPREME COURT\\b', '\\bCONSTITUTION\\b', '\\bUNITED NATIONS\\b', '\\bNATIVE AMERICANS\\b', '\\bAFRICAN AMERICANS\\b', '\\bBLACK AMERICANS\\b']), ('MATH & MEASUREMENT', ['\\bMATH', '\\bGEOMETRY\\b', '\\bWEIGHTS?\\b', '\\bMEASURES?\\b', '\\bNUMBERS?\\b', '\\bBY THE NUMBERS\\b', '\\bCOINS\\b', '\\bSTAMPS\\b', '\\bPARTS OF THE WHOLE\\b']), ('CULTURE & GENERAL', ['\\bCULTURE\\b', '\\bAMERICANA\\b', '\\bPOP CULTURE\\b', '\\bGAMES?\\b', '\\bTOYS\\b', '\\bGUINNESS\\b', '\\bTRIVIA\\b', '\\bWORLD FACTS\\b', '\\bFACTS\\b', '\\bON THE NEWS\\b', '\\bIN THE NEWS\\b', '\\bPOT LUCK\\b', '\\bU\\.S\\.A\\.\\b', '\\bAROUND THE WORLD\\b'])]

def classify_category(category):
    c = (category or "").strip()
    if any(re.search(p, c, re.I) for p in PUZZLE):
        return "WORDPLAY & MIXED"
    for subject, patterns in RULES:
        if any(re.search(p, c, re.I) for p in patterns):
            return subject
    return "GENERAL KNOWLEDGE"

def clean(value):
    s = str(value or "").replace("\r", " ").replace("\n", " ").replace("\t", " ")
    s = s.replace(":", "—").replace("|", "/").replace("{", "(").replace("}", ")")
    s = s.replace("[", "(").replace("]", ")").replace("\\", "")
    return re.sub(r"\s+", " ", s).strip()

def slug(subject):
    return re.sub(r"[^a-z0-9]+", "_", subject.lower()).strip("_")

def main():
    if len(sys.argv) != 3:
        raise SystemExit("Usage: generate_subject_decks.py INPUT.csv OUTPUT_DIR")
    source, out = Path(sys.argv[1]), Path(sys.argv[2])
    decks = out / "subjects"
    decks.mkdir(parents=True, exist_ok=True)
    handles, counts = {}, defaultdict(int)
    with source.open(encoding="utf-8-sig", newline="") as src:
        reader = csv.DictReader(src)
        for row in reader:
            row = {str(k).strip(): (v or "").strip() for k, v in row.items()}
            clue, answer = row.get("Question", ""), row.get("Answer", "")
            if not clue or not answer:
                continue
            subject = classify_category(row.get("Category", ""))
            key = slug(subject)
            if key not in handles:
                handles[key] = (decks / (key + ".txt")).open("w", encoding="utf-8", newline="\n")
            category = clean(row.get("Category", "")) or "UNCATEGORIZED"
            value = clean(row.get("Value", "")) or "VALUE N/A"
            prompt = clean(f"{subject} / {category} / {value} / {clue}")
            handles[key].write(prompt + ":" + clean(answer) + "\n")
            counts[subject] += 1
    for handle in handles.values():
        handle.close()
    repo_dir = "/home/roger/brother-powertools/data/quiz/subjects"
    with (out / "subjects.index").open("w", encoding="utf-8") as idx:
        for key in sorted(handles):
            idx.write(f"{repo_dir}/{key}.txt:{key}:response\n")
    for subject, count in sorted(counts.items(), key=lambda item: -item[1]):
        print(f"{subject}: {count:,}")

if __name__ == "__main__":
    main()
